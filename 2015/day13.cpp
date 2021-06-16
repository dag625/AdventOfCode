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

    std::pair<std::vector<relationship>, int> get_input(const std::filesystem::path& input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2015" / "day_13_input.txt");
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

    /*
    In years past, the holiday feast with your family hasn't gone so well. Not everyone gets along! This year, you resolve, will be different. You're going to find the optimal seating arrangement and avoid all those awkward conversations.

    You start by writing up a list of everyone invited and the amount their happiness would increase or decrease if they were to find themselves sitting next to each other person. You have a circular table that will be just big enough to fit everyone comfortably, and so each person will have exactly two neighbors.

    For example, suppose you have only four attendees planned, and you calculate their potential happiness as follows:

    Alice would gain 54 happiness units by sitting next to Bob.
    Alice would lose 79 happiness units by sitting next to Carol.
    Alice would lose 2 happiness units by sitting next to David.
    Bob would gain 83 happiness units by sitting next to Alice.
    Bob would lose 7 happiness units by sitting next to Carol.
    Bob would lose 63 happiness units by sitting next to David.
    Carol would lose 62 happiness units by sitting next to Alice.
    Carol would gain 60 happiness units by sitting next to Bob.
    Carol would gain 55 happiness units by sitting next to David.
    David would gain 46 happiness units by sitting next to Alice.
    David would lose 7 happiness units by sitting next to Bob.
    David would gain 41 happiness units by sitting next to Carol.
    Then, if you seat Alice next to David, Alice would lose 2 happiness units (because David talks so much), but David would gain 46 happiness units (because Alice is such a good listener), for a total change of 44.

    If you continue around the table, you could then seat Bob next to Alice (Bob gains 83, Alice gains 54). Finally, seat Carol, who sits next to Bob (Carol gains 60, Bob loses 7) and David (Carol gains 55, David gains 41). The arrangement looks like this:

         +41 +46
    +55   David    -2
    Carol       Alice
    +60    Bob    +54
         -7  +83
    After trying every other seating arrangement in this hypothetical scenario, you find that this one is the most optimal, with a total change in happiness of 330.

    What is the total change in happiness for the optimal seating arrangement of the actual guest list?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto [rels, last_id] = get_input(input_dir);
        const auto perms = generate_permutations(last_id, last_id);
        std::vector<int> lengths;
        lengths.reserve(perms.size());
        std::transform(perms.begin(), perms.end(), std::back_inserter(lengths),
                       [&r = rels](const std::vector<int>& p){ return get_total_happiness(r, p); });
        return std::to_string(*std::max_element(lengths.begin(), lengths.end()));
    }

    /*
    In all the commotion, you realize that you forgot to seat yourself. At this point, you're pretty apathetic toward the whole thing, and your happiness wouldn't really go up or down regardless of who you sit next to. You assume everyone else would be just as ambivalent about sitting next to you, too.

    So, add yourself to the list, and give all happiness relationships that involve you a score of 0.

    What is the total change in happiness for the optimal seating arrangement that actually includes yourself?
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto [rels, last_id] = get_input(input_dir);
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
    std::string part_2_alt(const std::filesystem::path& input_dir) {
        const auto [rels, last_id] = add_self(get_input(input_dir));
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

