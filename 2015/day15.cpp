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

namespace fs = std::filesystem;

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

    /*
    --- Day 15: Science for Hungry People ---
    Today, you set out on the task of perfecting your milk-dunking cookie recipe. All you have to do is find the right balance of ingredients.

    Your recipe leaves room for exactly 100 teaspoons of ingredients. You make a list of the remaining ingredients you could use to finish the recipe (your puzzle input) and their properties per teaspoon:

    capacity (how well it helps the cookie absorb milk)
    durability (how well it keeps the cookie intact when full of milk)
    flavor (how tasty it makes the cookie)
    texture (how it improves the feel of the cookie)
    calories (how many calories it adds to the cookie)
    You can only measure ingredients in whole-teaspoon amounts accurately, and you have to be accurate so you can reproduce your results in the future. The total score of a cookie can be found by adding up each of the properties (negative totals become 0) and then multiplying together everything except calories.

    For instance, suppose you have these two ingredients:

    Butterscotch: capacity -1, durability -2, flavor 6, texture 3, calories 8
    Cinnamon: capacity 2, durability 3, flavor -2, texture -1, calories 3
    Then, choosing to use 44 teaspoons of butterscotch and 56 teaspoons of cinnamon (because the amounts of each ingredient must add up to 100) would result in a cookie with the following properties:

    A capacity of 44*-1 + 56*2 = 68
    A durability of 44*-2 + 56*3 = 80
    A flavor of 44*6 + 56*-2 = 152
    A texture of 44*3 + 56*-1 = 76
    Multiplying these together (68 * 80 * 152 * 76, ignoring calories for now) results in a total score of 62842880, which happens to be the best score possible given these ingredients. If any properties had produced a negative total, it would have instead become zero, causing the whole score to multiply to zero.

    Given the ingredients in your kitchen and their properties, what is the total score of the highest-scoring cookie you can make?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        return std::to_string(maximize({25,25,25,25}));
    }

    /*
    --- Part Two ---
    Your cookie recipe becomes wildly popular! Someone asks if you can make another recipe that has exactly 500 calories per cookie (so they can use it as a meal replacement). Keep the rest of your award-winning process the same (100 teaspoons, same ingredients, same scoring system).

    For example, given the ingredients above, if you had instead selected 40 teaspoons of butterscotch and 60 teaspoons of cinnamon (which still adds to 100), the total calorie count would be 40*8 + 60*3 = 500. The total score would go down, though: only 57600000, the best you can do in such trying circumstances.

    Given the ingredients in your kitchen and their properties, what is the total score of the highest-scoring cookie you can make with a calorie total of 500?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        return std::to_string(maximize_w_calories({25,25,25,25}));
    }

    aoc::registration r {2015, 15, part_1, part_2};

//    TEST_SUITE("2015_day15") {
//        TEST_CASE("2015_day15:example") {
//
//        }
//    }

}