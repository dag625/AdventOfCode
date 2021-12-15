//
// Created by Dan on 12/14/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <utility>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    using chair = std::pair<char, char>;//= character-pair ;)

    struct mapping {
        chair from;
        std::pair<chair, chair> to;
    };

    struct chair_count {
        chair value;
        int64_t count = 0;

        explicit chair_count(chair v, int64_t c = 0) : value{std::move(v)}, count{c} {}
    };

    using char_count = std::pair<char, int64_t>;

    int64_t get_char_count(const std::vector<char_count>& counts, const char c) {
        const auto found = std::find_if(counts.begin(), counts.end(), [c](const char_count& ct){ return c == ct.first; });
        if (found != counts.end()) {
            return found->second;
        }
        else {
            return 0;
        }
    }

    void increment_count(std::vector<chair_count>& counts, const chair& c, int64_t amount = 1) {
        auto found = std::find_if(counts.begin(), counts.end(), [&c](const chair_count& ct){ return c == ct.value; });
        if (found != counts.end()) {
            found->count += amount;
        }
        else {
            counts.emplace_back(c, amount);
        }
    }

    mapping parse_mapping(std::string_view s) {
        const auto parts = split(s, " -> ");
        return {{parts[0][0], parts[0][1]}, {{parts[0][0], parts[1][0]}, {parts[1][0], parts[0][1]}}};
    }

    std::pair<std::string, std::vector<mapping>> get_input(const fs::path& input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_14_input.txt");
        const std::string& start = lines.front();

        auto mappings = lines |
                     std::views::drop(2) |
                     std::views::transform([](std::string_view s){ return parse_mapping(s); }) |
                     to<std::vector<mapping>>();
        return {start, std::move(mappings)};
    }

    std::vector<chair_count> init_counts(const std::string& start) {
        std::vector<chair_count> counts;
        for (auto a = start.begin(), b = a + 1; b != start.end(); ++a, ++b) {
            const auto c = chair{*a, *b};
            increment_count(counts, c);
        }
        return counts;
    }

    std::vector<chair_count> iterate(const std::vector<chair_count>& counts, const std::vector<mapping>& mappings) {
        std::vector<chair_count> new_counts;
        for (const auto& ct : counts) {
            const auto found_mapping = std::find_if(mappings.begin(), mappings.end(), [&ct](const mapping& m){ return ct.value == m.from; });
            if (found_mapping != mappings.end()) {
                increment_count(new_counts, found_mapping->to.first, ct.count);
                increment_count(new_counts, found_mapping->to.second, ct.count);
            }
            else {
                increment_count(new_counts, ct.value, ct.count);
            }
        }
        return new_counts;
    }

    std::vector<char_count> get_char_counts(const std::vector<chair_count>& counts) {
        std::vector<char_count> retval;
        retval.reserve(26);
        retval.emplace_back(counts.front().value.first, 1);
        for (const auto& ct : counts) {
            auto found = std::find_if(retval.begin(), retval.end(), [&ct](const char_count& c){ return c.first == ct.value.second; });
            if (found != retval.end()) {
                found->second += ct.count;
            }
            else {
                retval.emplace_back(ct.value.second, ct.count);
            }
        }
        return retval;
    }

    void do_iterations(const int num, std::vector<chair_count>& counts, const std::vector<mapping>& mappings) {
        for (int i = 0; i < num; ++i) {
            counts = iterate(counts, mappings);
        }
    }

    /*
    --- Day 14: Extended Polymerization ---
    The incredible pressures at this depth are starting to put a strain on your submarine. The submarine has polymerization equipment that would produce suitable materials to reinforce the submarine, and the nearby volcanically-active caves should even have the necessary input elements in sufficient quantities.

    The submarine manual contains instructions for finding the optimal polymer formula; specifically, it offers a polymer template and a list of pair insertion rules (your puzzle input). You just need to work out what polymer would result after repeating the pair insertion process a few times.

    For example:

    NNCB

    CH -> B
    HH -> N
    CB -> H
    NH -> C
    HB -> C
    HC -> B
    HN -> C
    NN -> C
    BH -> H
    NC -> B
    NB -> B
    BN -> B
    BB -> N
    BC -> B
    CC -> N
    CN -> C
    The first line is the polymer template - this is the starting point of the process.

    The following section defines the pair insertion rules. A rule like AB -> C means that when elements A and B are immediately adjacent, element C should be inserted between them. These insertions all happen simultaneously.

    So, starting with the polymer template NNCB, the first step simultaneously considers all three pairs:

    The first pair (NN) matches the rule NN -> C, so element C is inserted between the first N and the second N.
    The second pair (NC) matches the rule NC -> B, so element B is inserted between the N and the C.
    The third pair (CB) matches the rule CB -> H, so element H is inserted between the C and the B.
    Note that these pairs overlap: the second element of one pair is the first element of the next pair. Also, because all pairs are considered simultaneously, inserted elements are not considered to be part of a pair until the next step.

    After the first step of this process, the polymer becomes NCNBCHB.

    Here are the results of a few steps using the above rules:

    Template:     NNCB
    After step 1: NCNBCHB
    After step 2: NBCCNBBBCBHCB
    After step 3: NBBBCNCCNBBNBNBBCHBHHBCHB
    After step 4: NBBNBNBBCCNBCNCCNBBNBBNBBBNBBNBBCBHCBHHNHCBBCBHCB
    This polymer grows quickly. After step 5, it has length 97; After step 10, it has length 3073. After step 10, B occurs 1749 times, C occurs 298 times, H occurs 161 times, and N occurs 865 times; taking the quantity of the most common element (B, 1749) and subtracting the quantity of the least common element (H, 161) produces 1749 - 161 = 1588.

    Apply 10 steps of pair insertion to the polymer template and find the most and least common elements in the result. What do you get if you take the quantity of the most common element and subtract the quantity of the least common element?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto [start, mappings] = get_input(input_dir);
        auto counts = init_counts(start);
        do_iterations(10, counts, mappings);
        const auto char_counts = get_char_counts(counts);
        const auto [least_common, most_common] = std::minmax_element(char_counts.begin(), char_counts.end(),
                                                                     [](const auto& a, const auto& b){ return a.second < b.second; });
        return std::to_string(most_common->second - least_common->second);
    }

    /*
    --- Part Two ---
    The resulting polymer isn't nearly strong enough to reinforce the submarine. You'll need to run more steps of the pair insertion process; a total of 40 steps should do it.

    In the above example, the most common element is B (occurring 2192039569602 times) and the least common element is H (occurring 3849876073 times); subtracting these produces 2188189693529.

    Apply 40 steps of pair insertion to the polymer template and find the most and least common elements in the result. What do you get if you take the quantity of the most common element and subtract the quantity of the least common element?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto [start, mappings] = get_input(input_dir);
        auto counts = init_counts(start);
        do_iterations(40, counts, mappings);
        const auto char_counts = get_char_counts(counts);
        const auto [least_common, most_common] = std::minmax_element(char_counts.begin(), char_counts.end(),
                                                                     [](const auto& a, const auto& b){ return a.second < b.second; });
        return std::to_string(most_common->second - least_common->second);
    }

    aoc::registration r {2021, 14, part_1, part_2};

    TEST_SUITE("2021_day14") {
        TEST_CASE("2021_day14:example") {
            std::string start{"NNCB"};
            std::vector<std::string> map_strs = {
                    "CH -> B",
                    "HH -> N",
                    "CB -> H",
                    "NH -> C",
                    "HB -> C",
                    "HC -> B",
                    "HN -> C",
                    "NN -> C",
                    "BH -> H",
                    "NC -> B",
                    "NB -> B",
                    "BN -> B",
                    "BB -> N",
                    "BC -> B",
                    "CC -> N",
                    "CN -> C"
            };
            const auto mappings = map_strs |
                            std::views::transform([](std::string_view s){ return parse_mapping(s); }) |
                            to<std::vector<mapping>>();
            auto counts = init_counts(start);
            do_iterations(10, counts, mappings);
            const auto char_counts = get_char_counts(counts);
            const auto [least_common, most_common] = std::minmax_element(char_counts.begin(), char_counts.end(),
                                                                         [](const auto& a, const auto& b){ return a.second < b.second; });
            REQUIRE_EQ(1588, most_common->second - least_common->second);
            REQUIRE_EQ(1749, get_char_count(char_counts, 'B'));
            REQUIRE_EQ(298, get_char_count(char_counts, 'C'));
            REQUIRE_EQ(161, get_char_count(char_counts, 'H'));
            REQUIRE_EQ(865, get_char_count(char_counts, 'N'));
        }
    }

}