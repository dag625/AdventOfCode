//
// Created by Dan on 12/6/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <bit>
#include <iostream>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/19
     */

    uint32_t get_input(const std::vector<std::string>& lines) {
        return parse<uint32_t>(lines.front());
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        //Solution to the Josephus Circle problem, taken from Wikipedia article on the subject.
        const auto res = ~std::bit_floor(input*2) & ((input<<1) | 1);
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = static_cast<int>(get_input(lines));
        //Solution thanks to following reddit comments:
        //https://www.reddit.com/r/adventofcode/comments/5j4lp1/comment/dbdihvu/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
        //https://www.reddit.com/r/adventofcode/comments/5j4lp1/comment/dbdfabo/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
        int pow3 = 3;
        while (pow3 < input) { pow3 *= 3; }
        const int pow3input = pow3 / 3; //This is floor(log_3(input))
        if (input == pow3input) {
            return std::to_string(input);
        }
        else if (input <= pow3input * 2) {
            return std::to_string(input - pow3input);
        }
        else {
            return std::to_string(2 * input - 3 * pow3input);
        }
    }

    aoc::registration r{2016, 19, part_1, part_2};

//    TEST_SUITE("2016_day19") {
//        TEST_CASE("2016_day19:example") {
//
//        }
//    }

} /* namespace <anon> */