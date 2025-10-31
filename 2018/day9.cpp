//
// Created by Dan on 10/31/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/9
     */

    struct game {
        int num_players = 0;
        int num_marbles = 0;
    };

    game get_input(const std::vector<std::string>& lines) {
        const auto parts = split_no_empty(lines.front(), ' ');
        return {parse32(parts[0]), parse32(parts[6])};
    }

    struct state {
        std::vector<int64_t> scores;
        std::vector<int> marbles = { 0 };
        int current = 0;

        explicit state(const game& g) { scores.resize(g.num_players); marbles.reserve(g.num_marbles + 1); }

        [[nodiscard]] int next() const {
            const int tmp = current + 2;//We are inserting at the postion before this.
            const int sz = static_cast<int>(marbles.size());
            const int a = tmp - static_cast<int>(tmp > sz) * sz;
            return a;
        }

        [[nodiscard]] int prev() const {
            const int tmp = current - 7;
            const int sz = static_cast<int>(marbles.size());
            const int a = tmp + static_cast<int>(tmp < 0) * sz;
            return a;
        }

        void add_marble(int player, int m) {
            if (m % 23 == 0) [[unlikely]] {
                scores[player] += m;
                const auto n = prev();
                const auto rem = marbles.begin() + n;
                scores[player] += *rem;
                marbles.erase(rem);
                current = n;
            }
            else if (marbles.size() < 2) [[unlikely]] {
                marbles.push_back(m);
                ++current;
            }
            else [[likely]] {
                const auto n = next();
                const auto pos = marbles.begin() + n;
                marbles.insert(pos, m);
                current = n;
            }
        }
    };

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        state s {input};
        for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
            s.add_marble(player, m);
        }
        const auto winner = std::max_element(s.scores.begin(), s.scores.end());
        return std::to_string(*winner);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        input.num_marbles *= 100; //I'm sure there's a smart algorithm, but this finishes in just under 10 min on my machine.
        state s {input};
        for (int m = 1, player = 0; m <= input.num_marbles ; ++m, player = (player + 1) % input.num_players) {
            s.add_marble(player, m);
        }
        const auto winner = std::max_element(s.scores.begin(), s.scores.end());
        return std::to_string(*winner);
    }

    aoc::registration r {2018, 9, part_1, part_2};

    TEST_SUITE("2018_day09") {
        TEST_CASE("2018_day09:example") {
            const game input {9, 25};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores.begin(), s.scores.end());
            CHECK_EQ(*winner, 32);
        }
        /*
            10 players; last marble is worth 1618 points: high score is 8317
            13 players; last marble is worth 7999 points: high score is 146373
            17 players; last marble is worth 1104 points: high score is 2764
            21 players; last marble is worth 6111 points: high score is 54718
            30 players; last marble is worth 5807 points: high score is 37305
         */
        TEST_CASE("2018_day09:other_example1") {
            const game input {10, 1618};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores.begin(), s.scores.end());
            CHECK_EQ(*winner, 8317);
        }
        TEST_CASE("2018_day09:other_example2") {
            const game input {13, 7999};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores.begin(), s.scores.end());
            CHECK_EQ(*winner, 146373);
        }
        TEST_CASE("2018_day09:other_example3") {
            const game input {17, 1104};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores.begin(), s.scores.end());
            CHECK_EQ(*winner, 2764);
        }
        TEST_CASE("2018_day09:other_example4") {
            const game input {21, 6111};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores.begin(), s.scores.end());
            CHECK_EQ(*winner, 54718);
        }
        TEST_CASE("2018_day09:other_example5") {
            const game input {30, 5807};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores.begin(), s.scores.end());
            CHECK_EQ(*winner, 37305);
        }
    }

} /* namespace <anon> */