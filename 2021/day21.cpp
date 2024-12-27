//
// Created by Dan on 12/21/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <array>
#include <vector>

#include "ranges.h"

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

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
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

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
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