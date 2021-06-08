//
// Created by Daniel Garcia on 6/8/21.
//

#include "registration.h"
#include "utilities.h"
#include "parse.h"

#include <doctest/doctest.h>

#include <string_view>
#include <regex>
#include <unordered_map>
#include <array>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    std::regex regex_input {R"(^([A-za-z]+) to ([A-za-z]+) = (\d+)$)"};

    struct named_route {
        std::string src;
        std::string dst;
        int length;
    };

    struct route {
        int src;
        int dst;
        int length;
    };

    named_route parse(std::string_view s) {
        std::match_results<std::string_view::iterator> match;
        if (std::regex_match(s.begin(), s.end(), match, regex_input) && match.size() == 4) {
            return named_route{match[1].str(), match[2].str(), aoc::parse<int>(match[3].str())};
        }
        else {
            throw std::runtime_error{"Failed to parse route entry."};
        }
    }

    route conv_to_id_route(const named_route& nr, const std::unordered_map<std::string, int>& map) {
        return route{ map.at(nr.src), map.at(nr.dst), nr.length };
    }

    std::pair<std::vector<route>, int> get_input(const std::filesystem::path& input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2015" / "day_9_input.txt");
        std::vector<named_route> named;
        named.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(named), parse);
        int next_id = 0;
        std::unordered_map<std::string, int> ids;
        for (const auto& nr : named) {
            if (ids.count(nr.src) == 0) {
                ids[nr.src] = ++next_id;
            }
            if (ids.count(nr.dst) == 0) {
                ids[nr.dst] = ++next_id;
            }
        }
        std::vector<route> retval;
        retval.reserve(named.size());
        std::transform(named.begin(), named.end(), std::back_inserter(retval),
                       [&ids](const named_route& nr){ return conv_to_id_route(nr, ids); });
        return {std::move(retval), next_id};
    }

    std::vector<std::vector<int>> generate_permutations(const int max, const int length) {
        if (length == 1) {
            std::vector<std::vector<int>> retval;
            for (int i = 1; i <= max; ++i) {
                retval.push_back({i});
            }
            return retval;
        }
        else {
            auto prev = generate_permutations(max, length - 1);
            std::vector<std::vector<int>> retval;
            for (const auto& p : prev) {
                for (int i = 1; i <= max; ++i) {
                    if (std::find(p.begin(), p.end(), i) != p.end()) {
                        continue;
                    }
                    retval.emplace_back();
                    retval.back().reserve(length);
                    std::copy(p.begin(), p.end(), std::back_inserter(retval.back()));
                    retval.back().push_back(i);
                }
            }
            return retval;
        }
    }

    int get_length(const std::vector<route>& routes, const std::vector<int>& path) {
        int last = 0;
        int length = 0;
        for (int current : path) {
            if (last != 0) {
                auto found = std::find_if(routes.begin(), routes.end(), [current, last](const route& r){
                    return r.src == last && r.dst == current ||
                            r.src == current && r.dst == last;
                });
                if (found == routes.end()) {
                    return 0;
                }
                length += found->length;
            }
            last = current;
        }
        return length;
    }

    /*
    Every year, Santa manages to deliver all of his presents in a single night.

    This year, however, he has some new locations to visit; his elves have provided him the distances between every pair of locations. He can start and end at any two (different) locations he wants, but he must visit each location exactly once. What is the shortest distance he can travel to achieve this?

    For example, given the following distances:

    London to Dublin = 464
    London to Belfast = 518
    Dublin to Belfast = 141
    The possible routes are therefore:

    Dublin -> London -> Belfast = 982
    London -> Dublin -> Belfast = 605
    London -> Belfast -> Dublin = 659
    Dublin -> Belfast -> London = 659
    Belfast -> Dublin -> London = 605
    Belfast -> London -> Dublin = 982
    The shortest of these is London -> Dublin -> Belfast = 605, and so the answer is 605 in this example.

    What is the distance of the shortest route?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto [routes, last_id] = get_input(input_dir);
        const auto perms = generate_permutations(last_id, last_id);
        std::vector<int> lengths;
        lengths.reserve(perms.size());
        std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                       [&r = routes](const std::vector<int>& p){ return get_length(r, p); });
        lengths.erase(std::remove(lengths.begin(), lengths.end(), 0), lengths.end());
        return std::to_string(*std::min_element(lengths.begin(), lengths.end()));
    }

    /*
    The next year, just to show off, Santa decides to take the route with the longest distance instead.

    He can still start and end at any two (different) locations he wants, and he still must visit each location exactly once.

    For example, given the distances above, the longest route would be 982 via (for example) Dublin -> London -> Belfast.

    What is the distance of the longest route?
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto [routes, last_id] = get_input(input_dir);
        const auto perms = generate_permutations(last_id, last_id);
        std::vector<int> lengths;
        lengths.reserve(perms.size());
        std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                       [&r = routes](const std::vector<int>& p){ return get_length(r, p); });
        lengths.erase(std::remove(lengths.begin(), lengths.end(), 0), lengths.end());
        return std::to_string(*std::max_element(lengths.begin(), lengths.end()));
    }

    aoc::registration r {2015, 9, part_1, part_2};

    TEST_SUITE("2015_day9") {
        TEST_CASE("2015_day9:example") {

        }
    }

}

