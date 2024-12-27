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

    std::regex regex_input {R"(^([A-Za-z]+) would (gain|lose) (\d+) happiness units by sitting next to ([A-Za-z]+).$)"};

    struct half_relationship {
        std::string src;
        std::string dst;
        int happiness;
    };

    struct relationship {
        int src;
        int dst;
        int happiness;
        relationship() = default;
        relationship(int s, int d, int h) : src{s}, dst{d}, happiness{h} {}
    };

    half_relationship parse(std::string_view s) {
        std::match_results<std::string_view::iterator> match;
        if (std::regex_match(s.begin(), s.end(), match, regex_input) && match.size() == 5) {
            return half_relationship{match[1].str(), match[4].str(),
                                     aoc::parse<int>(match[3].str()) * (match[2].str() == "lose"sv ? -1 : 1 ) };
        }
        else {
            throw std::runtime_error{"Failed to parse relationship entry."};
        }
    }

    std::pair<std::vector<relationship>, int> half_to_full(const std::vector<half_relationship>& named) {
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
        std::vector<relationship> retval;
        retval.reserve(named.size());
        for (const auto& n : named) {
            const auto a = ids[n.src], b = ids[n.dst];
            auto found = std::find_if(retval.begin(), retval.end(), [a, b](const relationship& r){
                return (r.src == a && r.dst == b) || (r.src == b && r.dst == a);
            });
            if (found == retval.end()) {
                retval.emplace_back(a, b, n.happiness);
            }
            else {
                found->happiness += n.happiness;
            }
        }
        return {std::move(retval), next_id};
    }

    std::pair<std::vector<relationship>, int> add_self(const std::pair<std::vector<relationship>, int>& without_self) {
        int my_id = without_self.second + 1;
        std::vector<relationship> with_self;
        with_self.reserve(without_self.first.size() + my_id);
        with_self.insert(with_self.end(), without_self.first.begin(), without_self.first.end());
        for (int i = 1; i < my_id; ++i) {
            with_self.emplace_back(my_id, i, 0);
        }
        return {std::move(with_self), my_id};
    }

    std::pair<std::vector<relationship>, int> get_input(const std::vector<std::string>& lines) {
        std::vector<half_relationship> named;
        named.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(named), parse);
        return half_to_full(named);
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

    int get_total_happiness(const std::vector<relationship>& routes, const std::vector<int>& path) {
        int last = 0;
        int total_happiness = 0;
        for (int current : path) {
            if (last != 0) {
                auto found = std::find_if(routes.begin(), routes.end(), [current, last](const relationship& r){
                    return r.src == last && r.dst == current ||
                            r.src == current && r.dst == last;
                });
                total_happiness += found->happiness;
            }
            last = current;
        }
        auto found = std::find_if(routes.begin(), routes.end(), [a = path.front(), b = path.back()](const relationship& r){
            return r.src == a && r.dst == b ||
                   r.src == b && r.dst == a;
        });
        return total_happiness + found->happiness;
    }

    int get_total_happiness_with_self(const std::vector<relationship>& routes, const std::vector<int>& path) {
        int last = 0;
        int total_happiness = 0;
        int least = std::numeric_limits<int>::max();
        for (int current : path) {
            if (last != 0) {
                auto found = std::find_if(routes.begin(), routes.end(), [current, last](const relationship& r){
                    return r.src == last && r.dst == current ||
                           r.src == current && r.dst == last;
                });
                total_happiness += found->happiness;
                if (found->happiness < least) {
                    least = found->happiness;
                }
            }
            last = current;
        }
        auto found = std::find_if(routes.begin(), routes.end(), [a = path.front(), b = path.back()](const relationship& r){
            return r.src == a && r.dst == b ||
                   r.src == b && r.dst == a;
        });
        if (found->happiness < least) {
            least = found->happiness;
        }
        return total_happiness + found->happiness - least;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [rels, last_id] = get_input(lines);
        const auto perms = generate_permutations(last_id, last_id);
        std::vector<int> lengths;
        lengths.reserve(perms.size());
        std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                       [&r = rels](const std::vector<int>& p){ return get_total_happiness(r, p); });
        return std::to_string(*std::max_element(lengths.begin(), lengths.end()));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [rels, last_id] = get_input(lines);
        const auto perms = generate_permutations(last_id, last_id);
        std::vector<int> lengths;
        lengths.reserve(perms.size());
        std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                       [&r = rels](const std::vector<int>& p){ return get_total_happiness_with_self(r, p); });
        return std::to_string(*std::max_element(lengths.begin(), lengths.end()));
    }

    /*
     * There are two ways to approach part 2.  This is the less clever way where we just add zero-value relationships
     * for ourselves.  This multiplies the number of permutations by the number of non-self people, taking up another
     * step on the factorial scaling of this algorithm with respect to the number of people at the table.
     *
     * The solution implemented by part_2() is a bit more clever recognizing that the answer here can be derived from
     * part_1() by taking the lowest value link of the solution there and replacing this with a 0 link where we seat
     * ourselves (this is what get_total_happiness_with_self() does compared to get_total_happiness()).
     *
     * I'm keeping this alternate, less clever solution for posterity.
     */
    std::string part_2_alt(const std::vector<std::string>& lines) {
        const auto [rels, last_id] = add_self(get_input(lines));
        const auto perms = generate_permutations(last_id, last_id);
        std::vector<int> lengths;
        lengths.reserve(perms.size());
        std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                       [&r = rels](const std::vector<int>& p){ return get_total_happiness(r, p); });
        return std::to_string(*std::max_element(lengths.begin(), lengths.end()));
    }

    aoc::registration r {2015, 13, part_1, part_2};

    TEST_SUITE("2015_day13") {
        TEST_CASE("2015_day13:example") {
            const auto [rels, last_id] = half_to_full({
                  {"Alice", "Bob", 54},
                  {"Alice", "Carol", -79},
                  {"Alice", "David", -2},

                  {"Bob", "Alice", 83},
                  {"Bob", "Carol", -7},
                  {"Bob", "David", -63},

                  {"Carol", "Alice", -62},
                  {"Carol", "Bob", 60},
                  {"Carol", "David", 55},

                  {"David", "Alice", 46},
                  {"David", "Bob", -7},
                  {"David", "Carol", 41}
            });
            const auto perms = generate_permutations(last_id, last_id);
            std::vector<int> lengths;
            lengths.reserve(perms.size());
            std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                           [&r = rels](const std::vector<int>& p){ return get_total_happiness(r, p); });
            REQUIRE_EQ(*std::max_element(lengths.begin(), lengths.end()), 330);
        }
    }

}

