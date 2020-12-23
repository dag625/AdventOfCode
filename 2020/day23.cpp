//
// Created by Daniel Garcia on 12/22/20.
//

#include "day23.h"

#include <doctest/doctest.h>

#include <iostream>
#include <array>
#include <sstream>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace std::string_view_literals;

    namespace {

        constexpr auto INIT_CUPS = std::array{5,8,3,9,7,6,2,4,1};

        struct cup {
            int next = -1;
            int prev = -1;
        };

        constexpr std::array<cup, INIT_CUPS.size()> get_initial_state() {
            std::array<cup, INIT_CUPS.size()> retval{};
            auto last = INIT_CUPS.back() - 1, first = INIT_CUPS.front() - 1;
            retval[INIT_CUPS[0] - 1].prev = last;
            retval[INIT_CUPS[0] - 1].next = INIT_CUPS[1] - 1;
            for (int i = 1; i < INIT_CUPS.size() - 1; ++i) {
                retval[INIT_CUPS[i] - 1].prev = INIT_CUPS[i - 1] - 1;
                retval[INIT_CUPS[i] - 1].next = INIT_CUPS[i + 1] - 1;
            }
            retval[INIT_CUPS[INIT_CUPS.size() - 1] - 1].prev = INIT_CUPS[INIT_CUPS.size() - 2] - 1;
            retval[INIT_CUPS[INIT_CUPS.size() - 1] - 1].next = first;
            return retval;
        }

        template <std::size_t N>
        int incr_index(int val, int incr) {
            auto next = (val + incr) % N;
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
        int backward(const std::array<cup, N>& cups, int current, std::size_t  dist) {
            while (dist > 0) {
                current = cups[current].prev;
                --dist;
            }
            return current;
        }

        template <std::size_t N>
        void remove(std::array<cup, N>& cups, int current) {
            cups[cups[current].prev].next = cups[current].next;
            cups[cups[current].next].prev = cups[current].prev;
            cups[current].next = -1;
            cups[current].prev = -1;
        }

        template <std::size_t N>
        void insert(std::array<cup, N>& cups, int current, int after) {
            cups[current].next = cups[after].next;
            cups[current].prev = after;
            cups[after].next = current;
            cups[cups[current].next].prev = current;
        }

        template <std::size_t N, typename = std::enable_if_t<N >= INIT_CUPS.size(), void>>
        std::array<cup, N> get_expanded_initial_state() {
            auto init = get_initial_state();
            if constexpr (N > INIT_CUPS.size()) {
                std::array<cup, N> retval{};
                std::copy(init.begin(), init.end(), retval.begin());
                int last = INIT_CUPS[INIT_CUPS.size() - 1] - 1;
                for (int i = INIT_CUPS.size(); i < N; ++i) {
                    insert(retval, i, last);
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
            remove(cups, removed[0]);
            remove(cups, removed[1]);
            remove(cups, removed[2]);

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

    */
    void solve_day_23_1(const std::filesystem::path& input_dir) {
        auto cups = get_initial_state();
        int current = INIT_CUPS[0] - 1;
        for (int i = 0; i < 100; ++i) {
            current = move(cups, current);
        }
        std::cout << '\t' << get_from(cups, 0) << '\n';
    }

    /*

    */
    void solve_day_23_2(const std::filesystem::path& input_dir) {
        auto cups = get_expanded_initial_state<1000000>();
        int current = INIT_CUPS[0] - 1;
        for (int i = 0; i < 10000000; ++i) {
            current = move(cups, current);
        }
        std::cout << '\t' << static_cast<int64_t>(forward(cups, 0, 1) + 1) * static_cast<int64_t>(forward(cups, 0, 2) + 1) << '\n';
    }

    TEST_SUITE("day23" * doctest::description("Tests for day 23 challenges.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day23:example" * doctest::description("Testing with input from challenge example for part 2.")) {
            CHECK(true);
        }
    }

} /* namespace aoc2020 */