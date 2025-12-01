//
// Created by Dan on 11/11/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

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

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        circular_list<int> recipes;
        recipes.push_back(3);
        recipes.push_back(7);
        iterator elf1 = recipes.begin();
        iterator elf2 = elf1 + 1;
        for (int i = 0; i < input; ++i) {
            cook(recipes, elf1, elf2);
            fmt::print("Step {:5}:  {}\n", i+1, print(recipes));
        }
        std::string result;
        auto res_it = recipes.begin() + input;//Maybe faster to go from back?
        for (int i = 0; i < 10; ++i, ++res_it) {
            result += *res_it + '0';
        }
        return result;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);


        return std::to_string(-1);
    }

    aoc::registration r{2018, 14, part_1, part_2};

    //    TEST_SUITE("2018_day14") {
    //        TEST_CASE("2018_day14:example") {
    //            const std::vector<std::string> lines {
    //
    //            };
    //            const auto input = get_input(lines);
    //
    //        }
    //    }

} /* namespace <anon> */
