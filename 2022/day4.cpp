//
// Created by Dan on 12/4/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    using assignment = std::pair<int, int>;
    using pairing = std::pair<assignment, assignment>;

    pairing parse_pairing(std::string_view s) {
        const auto ss = split(s, ',');
        const auto se1 = split(ss[0], '-');
        const auto se2 = split(ss[1], '-');
        return {{parse<int>(se1[0]), parse<int>(se1[1])}, {parse<int>(se2[0]), parse<int>(se2[1])}};
    }

    std::vector<pairing> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_4_input.txt");
        return lines | std::views::transform(parse_pairing) | to<std::vector<pairing>>();
    }

    bool contains(const assignment& outer, const assignment& inner) {
        return inner.first >= outer.first && inner.second <= outer.second;
    }

    bool either_contains(const pairing& p) {
        return contains(p.first, p.second) || contains(p.second, p.first);
    }

    bool inside(const assignment& a, int pos) {
        return pos >= a.first && pos <= a.second;
    }

    bool overlap(const pairing& p) {
        return either_contains(p) || inside(p.first, p.second.first) || inside(p.first, p.second.second);
    }

    /*
    --- Day 4: Camp Cleanup ---
    Space needs to be cleared before the last supplies can be unloaded from the ships, and so several Elves have been assigned the job of cleaning up sections of the camp. Every section has a unique ID number, and each Elf is assigned a range of section IDs.

    However, as some of the Elves compare their section assignments with each other, they've noticed that many of the assignments overlap. To try to quickly find overlaps and reduce duplicated effort, the Elves pair up and make a big list of the section assignments for each pair (your puzzle input).

    For example, consider the following list of section assignment pairs:

    2-4,6-8
    2-3,4-5
    5-7,7-9
    2-8,3-7
    6-6,4-6
    2-6,4-8
    For the first few pairs, this list means:

    Within the first pair of Elves, the first Elf was assigned sections 2-4 (sections 2, 3, and 4), while the second Elf was assigned sections 6-8 (sections 6, 7, 8).
    The Elves in the second pair were each assigned two sections.
    The Elves in the third pair were each assigned three sections: one got sections 5, 6, and 7, while the other also got 7, plus 8 and 9.
    This example list uses single-digit section IDs to make it easier to draw; your actual list might contain larger numbers. Visually, these pairs of section assignments look like this:

    .234.....  2-4
    .....678.  6-8

    .23......  2-3
    ...45....  4-5

    ....567..  5-7
    ......789  7-9

    .2345678.  2-8
    ..34567..  3-7

    .....6...  6-6
    ...456...  4-6

    .23456...  2-6
    ...45678.  4-8
    Some of the pairs have noticed that one of their assignments fully contains the other. For example, 2-8 fully contains 3-7, and 6-6 is fully contained by 4-6. In pairs where one assignment fully contains the other, one Elf in the pair would be exclusively cleaning sections their partner will already be cleaning, so these seem like the most in need of reconsideration. In this example, there are 2 such pairs.

    In how many assignment pairs does one range fully contain the other?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        return std::to_string(std::count_if(input.begin(), input.end(), either_contains));
    }

    /*
    --- Part Two ---
    It seems like there is still quite a bit of duplicate work planned. Instead, the Elves would like to know the number of pairs that overlap at all.

    In the above example, the first two pairs (2-4,6-8 and 2-3,4-5) don't overlap, while the remaining four pairs (5-7,7-9, 2-8,3-7, 6-6,4-6, and 2-6,4-8) do overlap:

    5-7,7-9 overlaps in a single section, 7.
    2-8,3-7 overlaps all of the sections 3 through 7.
    6-6,4-6 overlaps in a single section, 6.
    2-6,4-8 overlaps in sections 4, 5, and 6.
    So, in this example, the number of overlapping assignment pairs is 4.

    In how many assignment pairs do the ranges overlap?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        return std::to_string(std::count_if(input.begin(), input.end(), overlap));//699 too low
    }

    aoc::registration r{2022, 4, part_1, part_2};

    TEST_SUITE("2022_day04") {
        TEST_CASE("2022_day04:example") {
            std::vector<pairing> input {
                    {{2, 4}, {6, 8}},
                    {{2, 3}, {4, 5}},
                    {{5, 7}, {7, 9}},
                    {{2, 8}, {3, 7}},
                    {{6, 6}, {4, 6}},
                    {{2, 6}, {4, 8}}
            };
            CHECK(!overlap(input[0]));
            CHECK(!overlap(input[1]));
            CHECK(overlap(input[2]));
            CHECK(overlap(input[3]));
            CHECK(overlap(input[4]));
            CHECK(overlap(input[5]));
            CHECK_EQ(std::count_if(input.begin(), input.end(), overlap), 4);
        }
    }

}