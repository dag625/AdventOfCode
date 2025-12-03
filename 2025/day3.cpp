//
// Created by Dan on 12/3/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/3
     */

    std::vector<std::string> get_input(const std::vector<std::string>& lines) {
        return lines;
    }

    int64_t largest_joltage_n(std::string_view pack, const int num) {
        int64_t retval = 0;
        auto current = pack.begin();
        for (int n = 0; n < num; ++n) {
            current = std::max_element(current, pack.end() - (num - n - 1));
            retval *= 10;
            retval += static_cast<int>(*current - '0');
            ++current;
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto sum = std::accumulate(input.begin(), input.end(), 0ll,
            [](int64_t tot, std::string_view s){ return tot + largest_joltage_n(s, 2); });
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto sum = std::accumulate(input.begin(), input.end(), 0ll,
            [](int64_t tot, std::string_view s){ return tot + largest_joltage_n(s, 12); });
        return std::to_string(sum);
    }

    aoc::registration r {2025, 3, part_1, part_2};

    TEST_SUITE("2025_day03") {
        TEST_CASE("2025_day03:example") {
            const std::vector<std::string> lines {
                "987654321111111",
                "811111111111119",
                "234234234234278",
                "818181911112111"
            };
            const auto input = get_input(lines);
            CHECK_EQ(largest_joltage_n(input[0], 2), 98);
            CHECK_EQ(largest_joltage_n(input[1], 2), 89);
            CHECK_EQ(largest_joltage_n(input[2], 2), 78);
            CHECK_EQ(largest_joltage_n(input[3], 2), 92);
            CHECK_EQ(largest_joltage_n(input[0], 12), 987654321111ll);
            CHECK_EQ(largest_joltage_n(input[1], 12), 811111111119ll);
            CHECK_EQ(largest_joltage_n(input[2], 12), 434234234278ll);
            CHECK_EQ(largest_joltage_n(input[3], 12), 888911112111ll);

            const auto sum = std::accumulate(input.begin(), input.end(), 0ll,
                [](int64_t tot, std::string_view s){ return tot + largest_joltage_n(s, 2); });
            CHECK_EQ(sum, 357);
        }
    }

} /* namespace <anon> */