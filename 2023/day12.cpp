//
// Created by Dan on 12/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>
#include <compare>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct row {
        std::string springs;
        std::vector<int> groups;
    };

    int parse_int(std::string_view s) { return parse<int>(s); }

    row parse_row(std::string_view s) {
        const auto parts = split(s, ' ');
        const auto grp_str = split(parts[1], ',');
        return {std::string{parts[0]}, grp_str | std::views::transform(&parse_int) | to<std::vector<int>>()};
    }

    std::vector<row> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_12_input.txt");
        return lines | std::views::transform(&parse_row) | to<std::vector<row>>();
    }

    row unfold(const row& r) {
        row retval = r;
        retval.groups.reserve(r.groups.size() * 5);
        retval.springs.reserve(r.springs.size() * 5 + 4);
        for (int i = 1; i < 5; ++i) {
            retval.groups.insert(retval.groups.end(), r.groups.begin(), r.groups.end());
            retval.springs.push_back('?');
            retval.springs.insert(retval.springs.end(), r.springs.begin(), r.springs.end());
        }
        return retval;
    }

    struct row_state {
        std::string springs;
        std::span<const int> groups;
        int64_t ways = 0;

        row_state(const row& r) : springs{r.springs}, groups{r.groups} {}
        bool operator==(const row_state& rhs) const {
            return springs == rhs.springs && std::equal(groups.begin(), groups.end(), rhs.groups.begin(), rhs.groups.end());
        }
        auto operator<=>(const row_state& rhs) const {
            if (const auto cmp = springs <=> rhs.springs; !std::is_eq(cmp)) {
                return cmp;
            }
            else {
                return std::lexicographical_compare_three_way(groups.begin(), groups.end(), rhs.groups.begin(), rhs.groups.end());
            }
        }
    };

    int64_t set_cache_and_return(row_state& r, std::vector<row_state>& cache, const int64_t val) {
        const auto ins_loc = std::lower_bound(cache.begin(), cache.end(), r);
        r.ways = val;
        cache.insert(ins_loc, r);
        return val;
    }

    int64_t calculate_arrangements(row_state r, std::vector<row_state>& cache) {
        if (r.springs.empty() && r.groups.empty()) {
            return 1;
        }
        else if (r.springs.empty()) {
            return 0;
        }

        const auto cfound = std::lower_bound(cache.begin(), cache.end(), r);
        if (cfound != cache.end() && *cfound == r) {
            return cfound->ways;
        }

        if (r.springs.front() == '.') {
            r.springs = r.springs.substr(1);
            const auto val = calculate_arrangements(r, cache);
            return set_cache_and_return(r, cache, val);
        }

        const auto rem_grp_size = std::accumulate(r.groups.begin(), r.groups.end(), 0);
        if (r.springs.size() < rem_grp_size) {
            return set_cache_and_return(r, cache, 0);
        }
        else if (r.springs.front() == '?') {
            row_state new_r = r;
            row_state short_r = r;
            new_r.springs.front() = '#';
            short_r.springs = short_r.springs.substr(1);
            const auto val1 = calculate_arrangements(short_r, cache);
            const auto val2 = calculate_arrangements(new_r, cache);
            return set_cache_and_return(r, cache, val1 + val2);
        }
        else if (r.springs.front() == '#') {
            if (r.groups.empty()) {
                return set_cache_and_return(r, cache, 0);
            }

            auto end = r.springs.find('.');
            if (end == std::string_view::npos) {
                end = r.springs.size();
            }
            if (end < r.groups.front()) {
                return set_cache_and_return(r, cache, 0);
            }

            row_state next_r = r;
            next_r.springs = next_r.springs.substr(r.groups.front());
            if (next_r.springs.empty()) {
                next_r.groups = next_r.groups.subspan(1);
                const auto val = calculate_arrangements(next_r, cache);
                return set_cache_and_return(r, cache, val);
            }
            else if (next_r.springs.front() == '#') {
                return set_cache_and_return(r, cache, 0);
            }
            else {
                //Next is either . or ?
                next_r.springs = next_r.springs.substr(1);
                next_r.groups = next_r.groups.subspan(1);
                const auto val = calculate_arrangements(next_r, cache);
                return set_cache_and_return(r, cache, val);
            }
        }
        return 0;//Never get here, but silence a warning.
    }

    int64_t calculate_arrangements(const row& r) {
        std::vector<row_state> cache;
        return calculate_arrangements(r, cache);
    }

    /*
    --- Day 12: Hot Springs ---
    You finally reach the hot springs! You can see steam rising from secluded areas attached to the primary, ornate building.

    As you turn to enter, the researcher stops you. "Wait - I thought you were looking for the hot springs, weren't you?" You indicate that this definitely looks like hot springs to you.

    "Oh, sorry, common mistake! This is actually the onsen! The hot springs are next door."

    You look in the direction the researcher is pointing and suddenly notice the massive metal helixes towering overhead. "This way!"

    It only takes you a few more steps to reach the main gate of the massive fenced-off area containing the springs. You go through the gate and into a small administrative building.

    "Hello! What brings you to the hot springs today? Sorry they're not very hot right now; we're having a lava shortage at the moment." You ask about the missing machine parts for Desert Island.

    "Oh, all of Gear Island is currently offline! Nothing is being manufactured at the moment, not until we get more lava to heat our forges. And our springs. The springs aren't very springy unless they're hot!"

    "Say, could you go up and see why the lava stopped flowing? The springs are too cold for normal operation, but we should be able to find one springy enough to launch you up there!"

    There's just one problem - many of the springs have fallen into disrepair, so they're not actually sure which springs would even be safe to use! Worse yet, their condition records of which springs are damaged (your puzzle input) are also damaged! You'll need to help them repair the damaged records.

    In the giant field just outside, the springs are arranged into rows. For each row, the condition records show every spring and whether it is operational (.) or damaged (#). This is the part of the condition records that is itself damaged; for some springs, it is simply unknown (?) whether the spring is operational or damaged.

    However, the engineer that produced the condition records also duplicated some of this information in a different format! After the list of springs for a given row, the size of each contiguous group of damaged springs is listed in the order those groups appear in the row. This list always accounts for every damaged spring, and each number is the entire size of its contiguous group (that is, groups are always separated by at least one operational spring: #### would always be 4, never 2,2).

    So, condition records with no unknown spring conditions might look like this:

    #.#.### 1,1,3
    .#...#....###. 1,1,3
    .#.###.#.###### 1,3,1,6
    ####.#...#... 4,1,1
    #....######..#####. 1,6,5
    .###.##....# 3,2,1
    However, the condition records are partially damaged; some of the springs' conditions are actually unknown (?). For example:

    ???.### 1,1,3
    .??..??...?##. 1,1,3
    ?#?#?#?#?#?#?#? 1,3,1,6
    ????.#...#... 4,1,1
    ????.######..#####. 1,6,5
    ?###???????? 3,2,1
    Equipped with this information, it is your job to figure out how many different arrangements of operational and broken springs fit the given criteria in each row.

    In the first line (???.### 1,1,3), there is exactly one way separate groups of one, one, and three broken springs (in that order) can appear in that row: the first three unknown springs must be broken, then operational, then broken (#.#), making the whole row #.#.###.

    The second line is more interesting: .??..??...?##. 1,1,3 could be a total of four different arrangements. The last ? must always be broken (to satisfy the final contiguous group of three broken springs), and each ?? must hide exactly one of the two broken springs. (Neither ?? could be both broken springs or they would form a single contiguous group of two; if that were true, the numbers afterward would have been 2,3 instead.) Since each ?? can either be #. or .#, there are four possible arrangements of springs.

    The last line is actually consistent with ten different arrangements! Because the first number is 3, the first and second ? must both be . (if either were #, the first number would have to be 4 or higher). However, the remaining run of unknown spring conditions have many different ways they could hold groups of two and one broken springs:

    ?###???????? 3,2,1
    .###.##.#...
    .###.##..#..
    .###.##...#.
    .###.##....#
    .###..##.#..
    .###..##..#.
    .###..##...#
    .###...##.#.
    .###...##..#
    .###....##.#
    In this example, the number of possible arrangements for each row is:

    ???.### 1,1,3 - 1 arrangement
    .??..??...?##. 1,1,3 - 4 arrangements
    ?#?#?#?#?#?#?#? 1,3,1,6 - 1 arrangement
    ????.#...#... 4,1,1 - 1 arrangement
    ????.######..#####. 1,6,5 - 4 arrangements
    ?###???????? 3,2,1 - 10 arrangements
    Adding all of the possible arrangement counts together produces a total of 21 arrangements.

    For each row, count all of the different arrangements of operational and broken springs that meet the given criteria. What is the sum of those counts?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto sum = std::accumulate(input.begin(), input.end(), static_cast<int64_t>(0),
                                         [](int64_t tot, const row& r){ return tot +
                                                 calculate_arrangements(r); });
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    As you look out at the field of springs, you feel like there are way more springs than the condition records list. When you examine the records, you discover that they were actually folded up this whole time!

    To unfold the records, on each row, replace the list of spring conditions with five copies of itself (separated by ?) and replace the list of contiguous groups of damaged springs with five copies of itself (separated by ,).

    So, this row:

    .# 1
    Would become:

    .#?.#?.#?.#?.# 1,1,1,1,1
    The first line of the above example would become:

    ???.###????.###????.###????.###????.### 1,1,3,1,1,3,1,1,3,1,1,3,1,1,3
    In the above example, after unfolding, the number of possible arrangements for some rows is now much larger:

    ???.### 1,1,3 - 1 arrangement
    .??..??...?##. 1,1,3 - 16384 arrangements
    ?#?#?#?#?#?#?#? 1,3,1,6 - 1 arrangement
    ????.#...#... 4,1,1 - 16 arrangements
    ????.######..#####. 1,6,5 - 2500 arrangements
    ?###???????? 3,2,1 - 506250 arrangements
    After unfolding, adding all of the possible arrangement counts together produces 525152.

    Unfold your condition records; what is the new sum of possible arrangement counts?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto sum = std::accumulate(input.begin(), input.end(), static_cast<int64_t>(0),
                                         [](int64_t tot, const row& r){ return tot +
                                                    calculate_arrangements(unfold(r)); });
        return std::to_string(sum);
    }

    aoc::registration r{2023, 12, part_1, part_2};

    TEST_SUITE("2023_day12") {
        TEST_CASE("2023_day12:example") {
            using namespace std::string_literals;
            std::vector<row> input {
                    {"???.###"s, {1, 1, 3}},
                    {".??..??...?##."s, {1, 1, 3}},
                    {"?#?#?#?#?#?#?#?"s, {1, 3, 1, 6}},
                    {"????.#...#..."s, {4, 1, 1}},
                    {"????.######..#####."s, {1, 6, 5}},
                    {"?###????????"s, {3, 2, 1}}
            };
            CHECK_EQ(calculate_arrangements(input[0]), 1);
            CHECK_EQ(calculate_arrangements(input[1]), 4);
            CHECK_EQ(calculate_arrangements(input[2]), 1);
            CHECK_EQ(calculate_arrangements(input[3]), 1);
            CHECK_EQ(calculate_arrangements(input[4]), 4);
            CHECK_EQ(calculate_arrangements(input[5]), 10);
            //Part 2
            CHECK_EQ(calculate_arrangements(unfold(input[0])), 1);
            CHECK_EQ(calculate_arrangements(unfold(input[1])), 16384);
            CHECK_EQ(calculate_arrangements(unfold(input[2])), 1);
            CHECK_EQ(calculate_arrangements(unfold(input[3])), 16);
            CHECK_EQ(calculate_arrangements(unfold(input[4])), 2500);
            CHECK_EQ(calculate_arrangements(unfold(input[5])), 506250);
        }
    }

}