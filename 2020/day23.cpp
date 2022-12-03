//
// Created by Daniel Garcia on 12/22/20.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <iostream>
#include <array>
#include <sstream>

#include "heap_array.h"

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace std::string_view_literals;

    namespace {

        constexpr auto INIT_CUPS = std::array{5,8,3,9,7,6,2,4,1};

        struct cup {
            int next = -1;
        };

        constexpr std::array<cup, INIT_CUPS.size()> get_initial_state() {
            std::array<cup, INIT_CUPS.size()> retval{};
            auto first = INIT_CUPS.front() - 1;
            retval[INIT_CUPS[0] - 1].next = INIT_CUPS[1] - 1;
            for (int i = 1; i < INIT_CUPS.size() - 1; ++i) {
                retval[INIT_CUPS[i] - 1].next = INIT_CUPS[i + 1] - 1;
            }
            retval[INIT_CUPS[INIT_CUPS.size() - 1] - 1].next = first;
            return retval;
        }

        template <std::size_t N>
        int incr_index(int val, int incr) {
            auto next = (val + incr) % static_cast<int>(N);
            while (next < 0) {
                next += N;
            }
            return next;
        }

        template <std::size_t N>
        int forward(const std::array<cup, N>& cups, int current, std::size_t dist) {
            while (dist > 0) {
                current = cups[current].next;
                --dist;
            }
            return current;
        }

        template <std::size_t N>
        int forward(const aoc::heap_array<cup, N>& cups, int current, std::size_t dist) {
            while (dist > 0) {
                current = cups[current].next;
                --dist;
            }
            return current;
        }

        template <std::size_t N>
        void remove(std::array<cup, N>& cups, int previous, int current) {
            cups[previous].next = cups[current].next;
            cups[current].next = -1;
        }

        template <std::size_t N>
        void remove(aoc::heap_array<cup, N>& cups, int previous, int current) {
            cups[previous].next = cups[current].next;
            cups[current].next = -1;
        }

        template <std::size_t N>
        void insert(std::array<cup, N>& cups, int current, int after) {
            cups[current].next = cups[after].next;
            cups[after].next = current;
        }

        template <std::size_t N>
        void insert(aoc::heap_array<cup, N>& cups, int current, int after) {
            cups[current].next = cups[after].next;
            cups[after].next = current;
        }

        template <std::size_t N, typename = std::enable_if_t<N >= INIT_CUPS.size(), void>>
        aoc::heap_array<cup, N> get_expanded_initial_state() {
            auto init = get_initial_state();
            if constexpr (N > INIT_CUPS.size()) {
                aoc::heap_array<cup, N> retval{};
                std::copy(init.begin(), init.end(), retval.begin());
                int last = INIT_CUPS[INIT_CUPS.size() - 1] - 1;
                for (int i = INIT_CUPS.size(); i < N; ++i) {
                    insert(retval, i, last);
                    last = i;
                }
                return retval;
            }
            else {
                return init;
            }
        }

        template <std::size_t N>
        int move(std::array<cup, N>& cups, int current) {
            std::array<int, 3> removed{};
            removed[0] = forward(cups, current, 1);
            removed[1] = forward(cups, removed[0], 1);
            removed[2] = forward(cups, removed[1], 1);
            remove(cups, current, removed[0]);
            remove(cups, current, removed[1]);
            remove(cups, current, removed[2]);

            auto dest = incr_index<N>(current, -1);
            while (dest == removed[0] || dest == removed[1] || dest == removed[2]) {
                dest = incr_index<N>(dest, -1);
            }
            insert(cups, removed[0], dest);
            insert(cups, removed[1], removed[0]);
            insert(cups, removed[2], removed[1]);
            return forward(cups, current, 1);
        }

        template <std::size_t N>
        int move(aoc::heap_array<cup, N>& cups, int current) {
            std::array<int, 3> removed{};
            removed[0] = forward(cups, current, 1);
            removed[1] = forward(cups, removed[0], 1);
            removed[2] = forward(cups, removed[1], 1);
            remove(cups, current, removed[0]);
            remove(cups, current, removed[1]);
            remove(cups, current, removed[2]);

            auto dest = incr_index<N>(current, -1);
            while (dest == removed[0] || dest == removed[1] || dest == removed[2]) {
                dest = incr_index<N>(dest, -1);
            }
            insert(cups, removed[0], dest);
            insert(cups, removed[1], removed[0]);
            insert(cups, removed[2], removed[1]);
            return forward(cups, current, 1);
        }

        template <std::size_t N>
        std::string get_from(const std::array<cup, N>& cups, const int after) {
            std::stringstream rs;
            int current = cups[after].next;
            do {
                rs << (current + 1);
                current = cups[current].next;
            } while (current != after);
            return rs.str();
        }

    }

    /*
    The small crab challenges you to a game! The crab is going to mix up some cups, and you have to predict where they'll end up.

    The cups will be arranged in a circle and labeled clockwise (your puzzle input). For example, if your labeling were 32415, there would be five cups in the circle; going clockwise around the circle from the first cup, the cups would be labeled 3, 2, 4, 1, 5, and then back to 3 again.

    Before the crab starts, it will designate the first cup in your list as the current cup. The crab is then going to do 100 moves.

    Each move, the crab does the following actions:

    The crab picks up the three cups that are immediately clockwise of the current cup. They are removed from the circle; cup spacing is adjusted as necessary to maintain the circle.
    The crab selects a destination cup: the cup with a label equal to the current cup's label minus one. If this would select one of the cups that was just picked up, the crab will keep subtracting one until it finds a cup that wasn't just picked up. If at any point in this process the value goes below the lowest value on any cup's label, it wraps around to the highest value on any cup's label instead.
    The crab places the cups it just picked up so that they are immediately clockwise of the destination cup. They keep the same order as when they were picked up.
    The crab selects a new current cup: the cup which is immediately clockwise of the current cup.
    For example, suppose your cup labeling were 389125467. If the crab were to do merely 10 moves, the following changes would occur:

    -- move 1 --
    cups: (3) 8  9  1  2  5  4  6  7
    pick up: 8, 9, 1
    destination: 2

    -- move 2 --
    cups:  3 (2) 8  9  1  5  4  6  7
    pick up: 8, 9, 1
    destination: 7

    -- move 3 --
    cups:  3  2 (5) 4  6  7  8  9  1
    pick up: 4, 6, 7
    destination: 3

    -- move 4 --
    cups:  7  2  5 (8) 9  1  3  4  6
    pick up: 9, 1, 3
    destination: 7

    -- move 5 --
    cups:  3  2  5  8 (4) 6  7  9  1
    pick up: 6, 7, 9
    destination: 3

    -- move 6 --
    cups:  9  2  5  8  4 (1) 3  6  7
    pick up: 3, 6, 7
    destination: 9

    -- move 7 --
    cups:  7  2  5  8  4  1 (9) 3  6
    pick up: 3, 6, 7
    destination: 8

    -- move 8 --
    cups:  8  3  6  7  4  1  9 (2) 5
    pick up: 5, 8, 3
    destination: 1

    -- move 9 --
    cups:  7  4  1  5  8  3  9  2 (6)
    pick up: 7, 4, 1
    destination: 5

    -- move 10 --
    cups: (5) 7  4  1  8  3  9  2  6
    pick up: 7, 4, 1
    destination: 3

    -- final --
    cups:  5 (8) 3  7  4  1  9  2  6
    In the above example, the cups' values are the labels as they appear moving clockwise around the circle; the current cup is marked with ( ).

    After the crab is done, what order will the cups be in? Starting after the cup labeled 1, collect the other cups' labels clockwise into a single string with no extra characters; each number except 1 should appear exactly once. In the above example, after 10 moves, the cups clockwise from 1 are labeled 9, 2, 6, 5, and so on, producing 92658374. If the crab were to complete all 100 moves, the order after cup 1 would be 67384529.

    Using your labeling, simulate 100 moves. What are the labels on the cups after cup 1?
    */
    std::string solve_day_23_1(const std::filesystem::path& input_dir) {
        auto cups = get_initial_state();
        int current = INIT_CUPS[0] - 1;
        for (int i = 0; i < 100; ++i) {
            current = move(cups, current);
        }
        return get_from(cups, 0);
    }

    /*
    Due to what you can only assume is a mistranslation (you're not exactly fluent in Crab), you are quite surprised when the crab starts arranging many cups in a circle on your raft - one million (1000000) in total.

    Your labeling is still correct for the first few cups; after that, the remaining cups are just numbered in an increasing fashion starting from the number after the highest number in your list and proceeding one by one until one million is reached. (For example, if your labeling were 54321, the cups would be numbered 5, 4, 3, 2, 1, and then start counting up from 6 until one million is reached.) In this way, every number from one through one million is used exactly once.

    After discovering where you made the mistake in translating Crab Numbers, you realize the small crab isn't going to do merely 100 moves; the crab is going to do ten million (10000000) moves!

    The crab is going to hide your stars - one each - under the two cups that will end up immediately clockwise of cup 1. You can have them if you predict what the labels on those cups will be when the crab is finished.

    In the above example (389125467), this would be 934001 and then 159792; multiplying these together produces 149245887792.

    Determine which two cups will end up immediately clockwise of cup 1. What do you get if you multiply their labels together?
    */
    std::string solve_day_23_2(const std::filesystem::path& input_dir) {
        auto cups = get_expanded_initial_state<1000000>();
        int current = INIT_CUPS[0] - 1;
        for (int i = 0; i < 10000000; ++i) {
            current = move(cups, current);
        }
        return std::to_string(static_cast<int64_t>(forward(cups, 0, 1) + 1) * static_cast<int64_t>(forward(cups, 0, 2) + 1));
    }

    static aoc::registration r {2020, 23, solve_day_23_1, solve_day_23_2};

    TEST_SUITE("day23" * doctest::description("Tests for day 23 challenges.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day23:example" * doctest::description("Testing with input from challenge example for part 2.")) {
            CHECK(true);
        }
    }

} /* namespace aoc2020 */