//
// Created by Dan on 12/21/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <array>
#include <vector>

#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr std::array<int, 2> PLAYER_START = {5, 10};
    constexpr int BOARD_LENGTH = 10;

    void do_player(int& position, int& roll_num, int& score) {
        if (roll_num < 98) {
            position += (roll_num * 3 + 6) % BOARD_LENGTH;
            roll_num += 3;
        }
        else {
            for (int i = 0; i < 3; ++i) {
                position += (roll_num % 100 + 1) % BOARD_LENGTH;
                roll_num = (roll_num + 1) % 100;
            }
        }
        if (position > BOARD_LENGTH) { position -= BOARD_LENGTH; }
        score += position;
    }

    constexpr std::array<std::pair<int, int>, 7> MOVE_TO_COUNT = {
            std::pair{3, 1}, {4, 3}, {5, 6}, {6, 7}, {7, 6}, {8, 3}, {9, 1}
    };

    struct player {
        uint8_t pos;
        uint8_t score;

        bool operator==(const player& rhs) const noexcept {
            return pos == rhs.pos && score == rhs.score;
        }
        bool operator<(const player& rhs) const noexcept {
            return std::tie(pos, score) < std::tie(rhs.pos, rhs.score);
        }
    };

    struct cache_value {
        player one;
        player two;
        int64_t p1_wins = 0;
        int64_t p2_wins = 0;

        bool operator==(const cache_value& rhs) const noexcept {
            return one == rhs.one && two == rhs.two;
        }
        bool operator<(const cache_value& rhs) const noexcept {
            return std::tie(one, two) < std::tie(rhs.one, rhs.two);
        }
    };

    std::pair<int64_t, int64_t> play_with_dirac_dice(const uint8_t p1_pos, const uint8_t p2_pos, std::vector<cache_value>& cache, const uint8_t p1_score = 0, const uint8_t p2_score = 0) {
        if (p2_score >= 21) {
            return {0, 1};
        }
        cache_value current{{p1_pos, p1_score}, {p2_pos, p2_score}};
        const auto lookup = std::lower_bound(cache.begin(), cache.end(), current);
        if (lookup != cache.end() && *lookup == current) {
            return {lookup->p1_wins, lookup->p2_wins};
        }
        for (const auto& mv : MOVE_TO_COUNT) {
            const uint8_t new_pos = (p1_pos + mv.first) % 10u;
            const uint8_t new_score = p1_score + new_pos + 1u;
            const auto [wins2, wins1] = play_with_dirac_dice(p2_pos, new_pos, cache, p2_score, new_score);
            current.p1_wins += mv.second * wins1;
            current.p2_wins += mv.second * wins2;
        }
        const auto found = std::lower_bound(cache.begin(), cache.end(), current);
        cache.insert(found, current);
        return {current.p1_wins, current.p2_wins};
    }

    /*
    --- Day 21: Dirac Dice ---
    There's not much to do as you slowly descend to the bottom of the ocean. The submarine computer challenges you to a nice game of Dirac Dice.

    This game consists of a single die, two pawns, and a game board with a circular track containing ten spaces marked 1 through 10 clockwise. Each player's starting space is chosen randomly (your puzzle input). Player 1 goes first.

    Players take turns moving. On each player's turn, the player rolls the die three times and adds up the results. Then, the player moves their pawn that many times forward around the track (that is, moving clockwise on spaces in order of increasing value, wrapping back around to 1 after 10). So, if a player is on space 7 and they roll 2, 2, and 1, they would move forward 5 times, to spaces 8, 9, 10, 1, and finally stopping on 2.

    After each player moves, they increase their score by the value of the space their pawn stopped on. Players' scores start at 0. So, if the first player starts on space 7 and rolls a total of 5, they would stop on space 2 and add 2 to their score (for a total score of 2). The game immediately ends as a win for any player whose score reaches at least 1000.

    Since the first game is a practice game, the submarine opens a compartment labeled deterministic dice and a 100-sided die falls out. This die always rolls 1 first, then 2, then 3, and so on up to 100, after which it starts over at 1 again. Play using this die.

    For example, given these starting positions:

    Player 1 starting position: 4
    Player 2 starting position: 8
    This is how the game would go:

    Player 1 rolls 1+2+3 and moves to space 10 for a total score of 10.
    Player 2 rolls 4+5+6 and moves to space 3 for a total score of 3.
    Player 1 rolls 7+8+9 and moves to space 4 for a total score of 14.
    Player 2 rolls 10+11+12 and moves to space 6 for a total score of 9.
    Player 1 rolls 13+14+15 and moves to space 6 for a total score of 20.
    Player 2 rolls 16+17+18 and moves to space 7 for a total score of 16.
    Player 1 rolls 19+20+21 and moves to space 6 for a total score of 26.
    Player 2 rolls 22+23+24 and moves to space 6 for a total score of 22.
    ...after many turns...

    Player 2 rolls 82+83+84 and moves to space 6 for a total score of 742.
    Player 1 rolls 85+86+87 and moves to space 4 for a total score of 990.
    Player 2 rolls 88+89+90 and moves to space 3 for a total score of 745.
    Player 1 rolls 91+92+93 and moves to space 10 for a final score, 1000.
    Since player 1 has at least 1000 points, player 1 wins and the game ends. At this point, the losing player had 745 points and the die had been rolled a total of 993 times; 745 * 993 = 739785.

    Play a practice game using the deterministic 100-sided die. The moment either player wins, what do you get if you multiply the score of the losing player by the number of times the die was rolled during the game?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        std::array<int, 2> scores{}, positions = PLAYER_START;
        int roll_num = 0, total_rolls = 0;
        while (scores[0] < 1000 && scores[1] < 1000) {
            do_player(positions[0], roll_num, scores[0]);
            do_player(positions[1], roll_num, scores[1]);
            total_rolls += 6;
        }
        if (scores[0] >= 1000) {
            return std::to_string(scores[1] * (total_rolls - 3));
        }
        else {
            return std::to_string(scores[0] * total_rolls);
        }
    }

    /*
    --- Part Two ---
    Now that you're warmed up, it's time to play the real game.

    A second compartment opens, this time labeled Dirac dice. Out of it falls a single three-sided die.

    As you experiment with the die, you feel a little strange. An informational brochure in the compartment explains that this is a quantum die: when you roll it, the universe splits into multiple copies, one copy for each possible outcome of the die. In this case, rolling the die always splits the universe into three copies: one where the outcome of the roll was 1, one where it was 2, and one where it was 3.

    The game is played the same as before, although to prevent things from getting too far out of hand, the game now ends when either player's score reaches at least 21.

    Using the same starting positions as in the example above, player 1 wins in 444356092776315 universes, while player 2 merely wins in 341960390180808 universes.

    Using your given starting positions, determine every possible outcome. Find the player that wins in more universes; in how many universes does that player win?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        std::vector<cache_value> cache;
        cache.reserve(20000);
        const auto [p1_wins, p2_wins] = play_with_dirac_dice(PLAYER_START[0] - 1, PLAYER_START[1] - 1, cache);
        return std::to_string(std::max(p1_wins, p2_wins));
    }

    aoc::registration r {2021, 21, part_1, part_2};

    TEST_SUITE("2021_day21") {
        TEST_CASE("2021_day21:example") {
            std::vector<cache_value> cache;
            const auto [w1, w2] = play_with_dirac_dice(3, 7, cache);
            REQUIRE_EQ(std::max(w1, w2), 444356092776315ull);
        }
    }

}