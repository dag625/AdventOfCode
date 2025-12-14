//
// Created by Dan on 11/11/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "circular_list.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2018/day/14
     */

    using iterator = circular_list<int>::iterator;

    int get_input(const std::vector<std::string>& lines) {
        return parse32(lines.front());
    }

    void cook(circular_list<int>& recipes, iterator& elf1, iterator& elf2) {
        int r1 = *elf1, r2 = *elf2;
        int sum = r1 + r2;
        int num1 = sum / 10, num2 = sum % 10;
        if (num1 != 0) { recipes.push_back(num1); } //Only if the sum>10.
        recipes.push_back(num2);
        elf1 += r1 + 1;
        elf2 += r2 + 1;
    }

    std::string print(const circular_list<int>& recipes) {
        return recipes |
            std::views::transform([](const int i){ return std::to_string(i); }) |
            std::views::join_with(' ') |
            std::ranges::to<std::string>();
    }

    std::string try_cooking_recipes(const int num_recipes) {
        circular_list<int> recipes;
        recipes.push_back(3);
        recipes.push_back(7);
        iterator elf1 = recipes.begin();
        iterator elf2 = elf1 + 1;
        for (int i = 0; recipes.size() < num_recipes + 10; ++i) {
            cook(recipes, elf1, elf2);
            //fmt::print("Step {:5}:  {}\n", i+1, print(recipes));
        }
        std::string result;
        auto res_it = recipes.begin() + num_recipes;//Maybe faster to go from back?
        for (int i = 0; i < 10; ++i, ++res_it) {
            result += *res_it + '0';
        }
        return result;
    }

    std::string recipes_from_end(const circular_list<int>& recipes, const int until_end, const int size) {
        std::string reversed;
        for (auto current = recipes.rbegin() + until_end; current != recipes.rbegin() + until_end + size; ++current) {
            reversed.push_back(static_cast<char>(*current + '0'));
        }
        std::string result {reversed.rbegin(), reversed.rend()};
        return result;
    }

    int try_finding_recipes(const std::string_view to_find) {
        const auto find_size = static_cast<int>(to_find.size());
        circular_list<int> recipes;
        recipes.push_back(3);
        recipes.push_back(7);
        iterator elf1 = recipes.begin();
        iterator elf2 = elf1 + 1;
        for (int step = 0; true; ++step) {
            const auto old_size = recipes.size();
            cook(recipes, elf1, elf2);
            //fmt::print("Step {:5}:  {}\n", step+1, print(recipes));
            const auto added =  recipes.size() - old_size;
            for (int j = 0; j < added; ++j) {
                const auto end = recipes_from_end(recipes, j, find_size);
                if (end == to_find) {
                    return static_cast<int>(recipes.size()) - find_size - j;
                }
            }
        }
        return -1;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto result = try_cooking_recipes(input);
        return result;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto result = try_finding_recipes(std::to_string(input));
        return std::to_string(result);
    }

    aoc::registration r{2018, 14, part_1, part_2};

    TEST_SUITE("2018_day14") {
        TEST_CASE("2018_day14:example") {
            const auto p1_1 = try_cooking_recipes(9);
            const auto p1_2 = try_cooking_recipes(5);
            const auto p1_3 = try_cooking_recipes(18);
            const auto p1_4 = try_cooking_recipes(2018);
            CHECK_EQ(p1_1, "5158916779");
            CHECK_EQ(p1_2, "0124515891");
            CHECK_EQ(p1_3, "9251071085");
            CHECK_EQ(p1_4, "5941429882");

            const auto p2_1 = try_finding_recipes("51589");
            const auto p2_2 = try_finding_recipes("01245");
            const auto p2_3 = try_finding_recipes("92510");
            const auto p2_4 = try_finding_recipes("59414");
            CHECK_EQ(p2_1, 9);
            CHECK_EQ(p2_2, 5);
            CHECK_EQ(p2_3, 18);
            CHECK_EQ(p2_4, 2018);
        }
    }

} /* namespace <anon> */
