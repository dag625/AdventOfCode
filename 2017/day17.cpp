//
// Created by Dan on 12/11/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/17
     */

    int get_input(const std::vector<std::string>& lines) {
        return parse<int>(lines.front());
    }

    int spin(std::vector<int>& data, const int step, const int current_pos, const int steps) {
        const auto new_pos = (current_pos + steps) % static_cast<int>(data.size()) + 1;
        data.insert(data.begin() + new_pos, step);
        return new_pos;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<int> data {0};
        int pos = 0;
        for (int i = 1; i <= 2017; ++i) {
            pos = spin(data, i, pos, input);
        }
        const auto found = std::find(data.begin(), data.end(), 2017);
        return std::to_string(*(found + 1));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        /*
         * We want the value after 0.  0 is always at the start of the buffer.  So we just want to keep track of the
         * index and see when it hits 1 and save the value.  We don't want/need to store the whole list of values.
         */

        int last_val = -1, pos = 0;
        for (int v = 1; v <= 50'000'000; ++v) {
            pos = (pos + input) % v + 1;
            if (pos == 1) {
                last_val = v;
            }
        }
        return std::to_string(last_val);
    }

    aoc::registration r{2017, 17, part_1, part_2};

//    TEST_SUITE("2017_day17") {
//        TEST_CASE("2017_day17:example") {
//
//        }
//    }

} /* namespace <anon> */