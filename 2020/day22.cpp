//
// Created by Daniel Garcia on 12/22/20.
//

#include "day22.h"

#include <doctest/doctest.h>

#include <iostream>
#include <deque>
#include <ostream>
#include <array>
#include <numeric>
#include <vector>
#include <tuple>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace std::string_view_literals;

    namespace {

        constexpr auto PLAYER_1_INIT = std::array{
                6,
                25,
                8,
                24,
                30,
                46,
                42,
                32,
                27,
                48,
                5,
                2,
                14,
                28,
                37,
                17,
                9,
                22,
                40,
                33,
                3,
                50,
                47,
                19,
                41
        };

        constexpr auto PLAYER_2_INIT = std::array{
                1,
                18,
                31,
                39,
                16,
                10,
                35,
                29,
                26,
                44,
                21,
                7,
                45,
                4,
                20,
                38,
                15,
                11,
                34,
                36,
                49,
                13,
                23,
                43,
                12
        };

        int calculate_score(const std::deque<int>& winning_deck) {
            int i = 1, score = 0;
            for (auto it = winning_deck.rbegin(); it != winning_deck.rend(); ++it, ++i) {
                score += *it * i;
            }
            return score;
        }

        int play_game_1() {
            std::deque<int> p1 {PLAYER_1_INIT.begin(), PLAYER_1_INIT.end()}, p2{PLAYER_2_INIT.begin(), PLAYER_2_INIT.end()};
            while (!p1.empty() && !p2.empty()) {
                if (p1.front() > p2.front()) {
                    p1.push_back(p1.front());
                    p1.push_back(p2.front());
                    p1.pop_front();
                    p2.pop_front();
                }
                else {
                    p2.push_back(p2.front());
                    p2.push_back(p1.front());
                    p1.pop_front();
                    p2.pop_front();
                }
            }
            return calculate_score(p1.empty() ? p2 : p1);
        }

        struct hand {
            std::vector<int> p1_cards;
            std::vector<int> p2_cards;
            hand(const std::deque<int>& p1, const std::deque<int>& p2) :
                p1_cards{p1.begin(), p1.end()}, p2_cards{p2.begin(), p2.end()} {}
        };

        bool has_been_played(std::vector<hand>& game, const std::deque<int>& p1, const std::deque<int>& p2) {
            auto found = std::find_if(game.begin(), game.end(), [&p1, &p2](const hand& g){
               return std::equal(g.p1_cards.begin(), g.p1_cards.end(), p1.begin(), p1.end()) &&
                    std::equal(g.p2_cards.begin(), g.p2_cards.end(), p2.begin(), p2.end());
            });
            if (found != game.end()) {
                return true;
            }
            else {
                game.emplace_back(p1, p2);
                return false;
            }
        }

        std::pair<bool, int> play_game_recursively(std::deque<int> p1, std::deque<int> p2) {
            std::vector<hand> game;
            while (!p1.empty() && !p2.empty()) {
                if (has_been_played(game, p1, p2)) {
                    return {true, calculate_score(p1)};
                }
                else if (p1.front() < p1.size() && p2.front() < p2.size()) {
                    auto [p1_wins, score] = play_game_recursively(
                            {p1.begin() + 1, p1.begin() + 1 + p1.front()},
                            {p2.begin() + 1, p2.begin() + 1 + p2.front()});
                    if (p1_wins) {
                        p1.push_back(p1.front());
                        p1.push_back(p2.front());
                        p1.pop_front();
                        p2.pop_front();
                    }
                    else {
                        p2.push_back(p2.front());
                        p2.push_back(p1.front());
                        p1.pop_front();
                        p2.pop_front();
                    }
                }
                else if (p1.front() > p2.front()) {
                    p1.push_back(p1.front());
                    p1.push_back(p2.front());
                    p1.pop_front();
                    p2.pop_front();
                }
                else {
                    p2.push_back(p2.front());
                    p2.push_back(p1.front());
                    p1.pop_front();
                    p2.pop_front();
                }
            }
            return {p2.empty(), calculate_score(p1.empty() ? p2 : p1)};
        }

    }

    /*

    */
    void solve_day_22_1(const std::filesystem::path& input_dir) {
        std::cout << '\t' << play_game_1() << '\n';
    }

    /*

    */
    void solve_day_22_2(const std::filesystem::path& input_dir) {
        auto [p1_wins, score] = play_game_recursively(
                {PLAYER_1_INIT.begin(), PLAYER_1_INIT.end()},
                {PLAYER_2_INIT.begin(), PLAYER_2_INIT.end()});
        std::cout << '\t' << score << '\n';
    }

    TEST_SUITE("day22" * doctest::description("Tests for day 22 challenges.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day22:example" * doctest::description("Testing with input from challenge example for part 2.")) {
            auto [p1_wins, score] = play_game_recursively({9, 2, 6, 3, 1}, {5, 8, 4, 7, 10});
                    CHECK_FALSE(p1_wins);
                    CHECK_EQ(score, 291);
        }
    }

} /* namespace aoc2020 */