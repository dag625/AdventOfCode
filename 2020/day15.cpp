//
// Created by Daniel Garcia on 12/15/20.
//

#include "day15.h"
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
                    auto last = numbers[next];
                    numbers[next] = turn;
                    next = turn - last;
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

    /*
    You catch the airport shuttle and try to book a new flight to your vacation island. Due to the storm, all direct flights have been cancelled, but a route is available to get around the storm. You take it.

    While you wait for your flight, you decide to check in with the Elves back at the North Pole. They're playing a memory game and are ever so excited to explain the rules!

    In this game, the players take turns saying numbers. They begin by taking turns reading from a list of starting numbers (your puzzle input). Then, each turn consists of considering the most recently spoken number:

    If that was the first time the number has been spoken, the current player says 0.
    Otherwise, the number had been spoken before; the current player announces how many turns apart the number is from when it was previously spoken.
    So, after the starting numbers, each turn results in that player speaking aloud either 0 (if the last number is new) or an age (if the last number is a repeat).

    For example, suppose the starting numbers are 0,3,6:

    Turn 1: The 1st number spoken is a starting number, 0.
    Turn 2: The 2nd number spoken is a starting number, 3.
    Turn 3: The 3rd number spoken is a starting number, 6.
    Turn 4: Now, consider the last number spoken, 6. Since that was the first time the number had been spoken, the 4th number spoken is 0.
    Turn 5: Next, again consider the last number spoken, 0. Since it had been spoken before, the next number to speak is the difference between the turn number when it was last spoken (the previous turn, 4) and the turn number of the time it was most recently spoken before then (turn 1). Thus, the 5th number spoken is 4 - 1, 3.
    Turn 6: The last number spoken, 3 had also been spoken before, most recently on turns 5 and 2. So, the 6th number spoken is 5 - 2, 3.
    Turn 7: Since 3 was just spoken twice in a row, and the last two turns are 1 turn apart, the 7th number spoken is 1.
    Turn 8: Since 1 is new, the 8th number spoken is 0.
    Turn 9: 0 was last spoken on turns 8 and 4, so the 9th number spoken is the difference between them, 4.
    Turn 10: 4 is new, so the 10th number spoken is 0.
    (The game ends when the Elves get sick of playing or dinner is ready, whichever comes first.)

    Their question for you is: what will be the 2020th number spoken? In the example above, the 2020th number spoken will be 436.

    Here are a few more examples:

    Given the starting numbers 1,3,2, the 2020th number spoken is 1.
    Given the starting numbers 2,1,3, the 2020th number spoken is 10.
    Given the starting numbers 1,2,3, the 2020th number spoken is 27.
    Given the starting numbers 2,3,1, the 2020th number spoken is 78.
    Given the starting numbers 3,2,1, the 2020th number spoken is 438.
    Given the starting numbers 3,1,2, the 2020th number spoken is 1836.
    Given your starting numbers, what will be the 2020th number spoken?
    */
    void solve_day_15_1(const std::filesystem::path&) {
        std::cout << '\t' << play_until_vec(INPUT, 2020) << '\n';
    }

    /*
    Impressed, the Elves issue you a challenge: determine the 30000000th number spoken. For example, given the same starting numbers as above:

    Given 0,3,6, the 30000000th number spoken is 175594.
    Given 1,3,2, the 30000000th number spoken is 2578.
    Given 2,1,3, the 30000000th number spoken is 3544142.
    Given 1,2,3, the 30000000th number spoken is 261214.
    Given 2,3,1, the 30000000th number spoken is 6895259.
    Given 3,2,1, the 30000000th number spoken is 18.
    Given 3,1,2, the 30000000th number spoken is 362.
    Given your starting numbers, what will be the 30000000th number spoken?
    */
    void solve_day_15_2(const std::filesystem::path&) {
        std::cout << '\t' << play_until_vec(INPUT, 30000000) << '\n';
    }

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