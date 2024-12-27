//
// Created by Dan on 12/25/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <random>
#include <deque>

#include "point_nd.h"
#include "sum.h"

namespace {

    using namespace aoc;

    /*
     * Input:
     *
     * Sprinkles: capacity 5, durability -1, flavor 0, texture 0, calories 5
     * PeanutButter: capacity -1, durability 3, flavor 0, texture 0, calories 1
     * Frosting: capacity 0, durability -1, flavor 4, texture 0, calories 6
     * Sugar: capacity -1, durability 0, flavor 0, texture 2, calories 8
     */

    constexpr int SPRINKLES_IDX = 0;
    constexpr int PB_IDX = 1;
    constexpr int FROSTING_IDX = 2;
    constexpr int SUGAR_IDX = 3;
    constexpr int SPRINKLES_CALS = 5;
    constexpr int PB_CALS = 1;
    constexpr int FROSTING_CALS = 6;
    constexpr int SUGAR_CALS = 8;
    constexpr point<4> CALORIES = {SPRINKLES_CALS, PB_CALS, FROSTING_CALS, SUGAR_CALS};
    constexpr int REQUIRED_CALS = 500;

    std::pair<int, int> calculate_score(const point<4>& ingredients) {
        int capacity_score = std::max(5*ingredients[SPRINKLES_IDX] - ingredients[PB_IDX] - ingredients[SUGAR_IDX], 0);
        int durability_score = std::max(-ingredients[SPRINKLES_IDX] + 3*ingredients[PB_IDX] - ingredients[FROSTING_IDX], 0);
        int flavor_score = 4*ingredients[FROSTING_IDX];
        int texture_score = 2*ingredients[SUGAR_IDX];
        int total_score = capacity_score * durability_score * flavor_score * texture_score;
        int total_calories = static_cast<int>(CALORIES * ingredients);
        return {total_score, total_calories};
    }

    bool is_valid(const point<4>& ingredients) {
        return std::all_of(ingredients.begin(), ingredients.end(), [](int v){ return v >= 0 && v <= 100; }) &&
               sum(ingredients) == 100;
    }

    int maximize(const point<4>& start) {
        constexpr auto DIRS = get_count_preserving_directions<4>();
        auto [best, start_cals] = calculate_score(start);
        point<4> last = start, winning{};
        bool any_better = true;
        while (any_better) {
            any_better = false;
            for (const auto &d: DIRS) {
                const auto next = last + d;
                if (is_valid(next)) {
                    auto [score, cals] = calculate_score(next);
                    if (score > best) {
                        any_better = true;
                        best = score;
                        winning = next;
                    }
                }
            }
            if (any_better) {
                last = winning;
            }
            else if (best == 0) {
                //We need to be careful because we can get stuck in local minima.
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distrib(0, DIRS.size() - 1);
                do {
                    last = last + DIRS[distrib(gen)];
                } while(!is_valid(last));
                any_better = true;
            }
        }
        return best;
    }

    point<4> find_required_cal_recipe(const point<4>& start) {
        constexpr auto DIRS = get_count_preserving_directions<4>();
        auto [best, best_cals] = calculate_score(start);
        point<4> last = start, winning = start;
        int cal_diff = REQUIRED_CALS - best_cals;
        while (cal_diff != 0) {
            for (const auto &d: DIRS) {
                const auto next = last + d;
                if (is_valid(next)) {
                    auto [score, cals] = calculate_score(next);
                    int diff = REQUIRED_CALS - cals;
                    if (std::abs(diff) < std::abs(cal_diff)) {
                        cal_diff = diff;
                        winning = next;
                    }
                }
            }
            if (cal_diff == 0) {
                return winning;
            }
        }
        return winning;
    }

    std::vector<point<4>> get_calorie_preserving_dirs() {
        const int min_diff = -10, max_diff = 10;
        std::vector<point<4>> retval;
        for (auto w = min_diff; w <= max_diff; ++w) {
            for (auto x = min_diff; x <= max_diff; ++x) {
                for (auto y = min_diff; y <= max_diff; ++y) {
                    for (auto z = min_diff; z <= max_diff; ++z) {
                        point<4> diff = {w, x, y, z};
                        if (sum(diff) == 0 && diff * CALORIES == 0) {
                            retval.push_back(diff);
                        }
                    }
                }
            }
        }
        return retval;
    }

    int maximize_w_calories(const point<4>& start) {
        const auto cal_dirs = get_calorie_preserving_dirs();
        point<4> last = find_required_cal_recipe(start), winning{};
        auto [best, start_cals] = calculate_score(last);
        bool any_better = true;
        while (any_better) {
            any_better = false;
            for (const auto &d: cal_dirs) {
                const auto next = last + d;
                if (is_valid(next)) {
                    auto [score, cals] = calculate_score(next);
                    if (score > best) {
                        any_better = true;
                        best = score;
                        winning = next;
                    }
                }
            }
            if (any_better) {
                last = winning;
            }
            else if (best == 0) {
                //We need to be careful because we can get stuck in local minima.
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distrib(0, static_cast<int>(cal_dirs.size() - 1));
                do {
                    last = last + cal_dirs[distrib(gen)];
                } while(!is_valid(last));
                any_better = true;
            }
        }
        return best;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        return std::to_string(maximize({25,25,25,25}));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        return std::to_string(maximize_w_calories({25,25,25,25}));
    }

    aoc::registration r {2015, 15, part_1, part_2};

//    TEST_SUITE("2015_day15") {
//        TEST_CASE("2015_day15:example") {
//
//        }
//    }

}