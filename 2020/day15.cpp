//
// Created by Daniel Garcia on 12/15/20.
//

#include "registration.h"
#include "span.h"

#include <doctest/doctest.h>

#include <iostream>
#include <array>
#include <unordered_map>
#include <vector>

namespace aoc2020 {

    namespace {

        constexpr auto INPUT = std::array{0,12,6,13,20,1,17};

        /*
         * Newer implementation.  Faster but less space efficient.
         */
        uint64_t play_until_vec(aoc::span<const int> input, uint64_t turns) {
            //Assume turns > INPUT.size()
            std::vector<int64_t> numbers (turns, -1);
            uint64_t turn = 0;
            for (auto v : input) {
                numbers[v] = turn;
                ++turn;
            }
            int64_t next = 0;//Assume no repeated numbers in the input.
            const auto last_turn = turns - 1;
            for (; turn < last_turn; ++turn) {
                auto prev = numbers[next];
                if (prev >= 0) {
                    numbers[next] = turn;
                    next = turn - prev;
                }
                else {
                    numbers[next] = turn;
                    next = 0;
                }
            }
            return next;
        }

        /*
         * Original implementation.  Slower but more space efficient.
         */
        uint64_t play_until_map(aoc::span<const int> input, uint64_t turns) {
            //Assume turns > INPUT.size()
            std::unordered_map<uint64_t, uint64_t> numbers;
            uint64_t turn = 0;
            for (auto v : input) {
                numbers[v] = turn;
                ++turn;
            }
            int64_t next = 0;//Assume no repeated numbers in the input.
            const auto last_turn = turns - 1;
            for (; turn < last_turn; ++turn) {
                auto found = numbers.find(next);
                if (found != numbers.end()) {
                    auto last = found->second;
                    found->second = turn;
                    next = turn - last;
                }
                else {
                    numbers.insert({next, turn});
                    next = 0;
                }
            }
            return next;
        }

    }

    /************************* Part 1 *************************/
    std::string solve_day_15_1(const std::vector<std::string>& lines) {
        return std::to_string(play_until_vec(INPUT, 2020));
    }

    /************************* Part 2 *************************/
    std::string solve_day_15_2(const std::vector<std::string>& lines) {
        return std::to_string(play_until_vec(INPUT, 30000000));
    }

    static aoc::registration r {2020, 15, solve_day_15_1, solve_day_15_2};

    TEST_SUITE("day15_orig" * doctest::description("Tests for day 15 challenges.")) {
        TEST_CASE("day15_orig:main_example" * doctest::description("Day 15 example with input (0,3,6).")) {
            constexpr auto TEST_INPUT = std::array{0,3,6};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 436);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 175594);
        }
        TEST_CASE("day15_orig:2nd_example" * doctest::description("Day 15 example with input (1,3,2).")) {
            constexpr auto TEST_INPUT = std::array{1,3,2};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 1);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 2578);
        }
        TEST_CASE("day15_orig:3rd_example" * doctest::description("Day 15 example with input (2,1,3).")) {
            constexpr auto TEST_INPUT = std::array{2,1,3};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 10);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 3544142);
        }
        TEST_CASE("day15_orig:4th_example" * doctest::description("Day 15 example with input (1,2,3).")) {
            constexpr auto TEST_INPUT = std::array{1,2,3};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 27);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 261214);
        }
        TEST_CASE("day15_orig:5th_example" * doctest::description("Day 15 example with input (2,3,1).")) {
            constexpr auto TEST_INPUT = std::array{2,3,1};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 78);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 6895259);
        }
        TEST_CASE("day15_orig:6th_example" * doctest::description("Day 15 example with input (3,2,1).")) {
            constexpr auto TEST_INPUT = std::array{3,2,1};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 438);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 18);
        }
        TEST_CASE("day15_orig:7th_example" * doctest::description("Day 15 example with input (3,1,2).")) {
            constexpr auto TEST_INPUT = std::array{3,1,2};
            REQUIRE_EQ(play_until_map(TEST_INPUT, 2020), 1836);
            REQUIRE_EQ(play_until_map(TEST_INPUT, 30000000), 362);
        }
    }

    TEST_SUITE("day15_v2" * doctest::description("Tests for day 15 challenges.")) {
        TEST_CASE("day15_v2:main_example" * doctest::description("Day 15 example with input (0,3,6).")) {
            constexpr auto TEST_INPUT = std::array{0,3,6};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 436);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 175594);
        }
        TEST_CASE("day15_v2:2nd_example" * doctest::description("Day 15 example with input (1,3,2).")) {
            constexpr auto TEST_INPUT = std::array{1,3,2};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 1);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 2578);
        }
        TEST_CASE("day15_v2:3rd_example" * doctest::description("Day 15 example with input (2,1,3).")) {
            constexpr auto TEST_INPUT = std::array{2,1,3};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 10);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 3544142);
        }
        TEST_CASE("day15_v2:4th_example" * doctest::description("Day 15 example with input (1,2,3).")) {
            constexpr auto TEST_INPUT = std::array{1,2,3};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 27);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 261214);
        }
        TEST_CASE("day15_v2:5th_example" * doctest::description("Day 15 example with input (2,3,1).")) {
            constexpr auto TEST_INPUT = std::array{2,3,1};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 78);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 6895259);
        }
        TEST_CASE("day15_v2:6th_example" * doctest::description("Day 15 example with input (3,2,1).")) {
            constexpr auto TEST_INPUT = std::array{3,2,1};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 438);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 18);
        }
        TEST_CASE("day15_v2:7th_example" * doctest::description("Day 15 example with input (3,1,2).")) {
            constexpr auto TEST_INPUT = std::array{3,1,2};
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 2020), 1836);
                    REQUIRE_EQ(play_until_vec(TEST_INPUT, 30000000), 362);
        }
    }

} /* namespace aoc2020 */