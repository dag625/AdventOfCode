//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/5
     */

    std::vector<int> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform([](const std::string& s){ return parse<int>(s); }) | std::ranges::to<std::vector>();
    }

    int run_until_exit(std::vector<int> jumps) {
        int steps = 0, ins = 0;
        while (ins >= 0 && ins < jumps.size()) {
            const auto offset = jumps[ins];
            ++jumps[ins];
            ins += offset;
            ++steps;
        }
        return steps;
    }

    int run_until_exit_strangely(std::vector<int> jumps) {
        int steps = 0, ins = 0;
        while (ins >= 0 && ins < jumps.size()) {
            const auto offset = jumps[ins];
            if (offset > 2) {
                --jumps[ins];
            }
            else {
                ++jumps[ins];
            }
            ins += offset;
            ++steps;
        }
        return steps;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto steps = run_until_exit(input);
        return std::to_string(steps);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto steps = run_until_exit_strangely(input);
        return std::to_string(steps);
    }

    aoc::registration r{2017, 5, part_1, part_2};

//    TEST_SUITE("2017_day05") {
//        TEST_CASE("2017_day05:example") {
//
//        }
//    }

} /* namespace <anon> */