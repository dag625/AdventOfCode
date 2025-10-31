//
// Created by Dan on 10/31/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <list>

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

    class state {
    public:
        using list = std::list<int>;
        using iter = list::const_iterator;

    private:
        std::vector<int64_t> m_scores;
        list marbles = { 0 };
        iter current = marbles.begin();

        [[nodiscard]] iter next(const iter start) const {
            iter retval = std::next(start);
            if (retval == marbles.end()) {
                return marbles.begin();
            }
            else {
                return retval;
            }
        }

        [[nodiscard]] iter prev(const iter start, int steps) const {
            iter retval = start;
            for (int s = 0; s < steps; ++s) {
                if (retval == marbles.begin()) {
                    retval = marbles.end();
                }
                retval = std::prev(retval);
            }
            return retval;
        }

    public:
        explicit state(const game& g) { m_scores.resize(g.num_players); }

        const std::vector<int64_t>& scores() const { return m_scores; }

        [[nodiscard]] iter next() const {
            return next(next(current));
        }

        [[nodiscard]] iter prev() const {
            return prev(current, 7);
        }

        void add_marble(int player, int m) {
            if (m % 23 == 0) [[unlikely]] {
                m_scores[player] += m;
                const auto rem = prev();
                m_scores[player] += *rem;
                current = next(rem);
                marbles.erase(rem);
            }
            else if (marbles.size() < 2) [[unlikely]] {
                marbles.push_back(m);
                ++current;
            }
            else [[likely]] {
                const auto pos = next();
                marbles.insert(pos, m);
                current = prev(pos, 1);
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
        const auto winner = std::max_element(s.scores().begin(), s.scores().end());
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
        const auto winner = std::max_element(s.scores().begin(), s.scores().end());
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
            const auto winner = std::max_element(s.scores().begin(), s.scores().end());
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
            const auto winner = std::max_element(s.scores().begin(), s.scores().end());
            CHECK_EQ(*winner, 8317);
        }
        TEST_CASE("2018_day09:other_example2") {
            const game input {13, 7999};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores().begin(), s.scores().end());
            CHECK_EQ(*winner, 146373);
        }
        TEST_CASE("2018_day09:other_example3") {
            const game input {17, 1104};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores().begin(), s.scores().end());
            CHECK_EQ(*winner, 2764);
        }
        TEST_CASE("2018_day09:other_example4") {
            const game input {21, 6111};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores().begin(), s.scores().end());
            CHECK_EQ(*winner, 54718);
        }
        TEST_CASE("2018_day09:other_example5") {
            const game input {30, 5807};
            state s {input};
            for (int m = 1, player = 0; m <= input.num_marbles; ++m, player = (player + 1) % input.num_players) {
                s.add_marble(player, m);
            }
            const auto winner = std::max_element(s.scores().begin(), s.scores().end());
            CHECK_EQ(*winner, 37305);
        }
    }

} /* namespace <anon> */