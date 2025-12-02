//
// Created by Dan on 12/1/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/1
     */

    int parse_spin(std::string_view line) {
        int sign = 1;
        if (line[0] == 'L') { sign = -1; }
        line.remove_prefix(1);
        return sign * parse32(line);
    }

    std::vector<int> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_spin) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int pos = 50, count = 0;
        for (const auto spin : input) {
            pos += spin;
            if (pos % 100 == 0) {
                ++count;
            }
        }
        return std::to_string(count);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int pos = 50, count = 0;
        for (const auto spin : input) {
            //This is dumb and slow but correct.  Better than my fast and "smart" and wrong ones.  :(
            if (spin > 0) {
                for (int i = 0; i < spin; ++i) {
                    ++pos;
                    if (pos == 0) {
                        ++count;
                    }
                    else if (pos == 100) {
                        pos = 0;
                        ++count;
                    }
                }
            }
            else {
                for (int i = 0; i > spin; --i) {
                    --pos;
                    if (pos == 0) {
                        ++count;
                    }
                    else if (pos == -1) {
                        pos = 99;
                    }
                }
            }
        }
        return std::to_string(count);
    }

    aoc::registration r {2025, 1, part_1, part_2};

    TEST_SUITE("2025_day01") {
        TEST_CASE("2025_day01:example") {
            const std::vector<std::string> lines {
                "L68",
                "L30",
                "R48",
                "L5",
                "R60",
                "L55",
                "L1",
                "L99",
                "R14",
                "L82"
            };
            const auto input = get_input(lines);
            int pos = 50, count = 0;
            for (const auto spin : input) {
                pos += spin;
                if (pos == 0) {
                    ++count;
                }
                else {
                    while (pos > 100) {
                        pos -= 100;
                        ++count;
                    }
                    while (pos < 0) {
                        pos += 100;
                        ++count;
                    }
                }
            }
            CHECK_EQ(count, 6);
            CHECK_EQ(pos, 32);
        }
        TEST_CASE("2025_day01:big_spin") {
            const std::vector<int> input1 = { 1, 1000 };
            int pos = 50, count = 0;
            for (const auto spin : input1) {
                pos += spin;
                if (pos == 0) {
                    ++count;
                }
                else {
                    while (pos > 100) {
                        pos -= 100;
                        ++count;
                    }
                    while (pos < 0) {
                        pos += 100;
                        ++count;
                    }
                }
            }
            CHECK_EQ(count, 10);
            CHECK_EQ(pos, 51);

            const std::vector<int> input2 = { -50, -100 };
            pos = 50;
            count = 0;
            for (const auto spin : input2) {
                pos += spin;
                if (pos == 0) {
                    ++count;
                }
                else {
                    while (pos > 100) {
                        pos -= 100;
                        ++count;
                    }
                    while (pos < 0) {
                        pos += 100;
                        ++count;
                    }
                }
            }
            CHECK_EQ(count, 2);
            CHECK_EQ(pos, 0);
        }
    }

} /* namespace <anon> */