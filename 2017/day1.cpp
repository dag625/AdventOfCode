//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/1
     */

    std::string get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_1_input.txt");
        return lines.front();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        int sum = 0;
        for (const auto p : input | std::views::slide(2)) {
            if (p.front() == p.back()) {
                sum += static_cast<int>(p.front() - '0');
            }
        }
        if (input.back() == input.front()) {
            sum += static_cast<int>(input.back() - '0');
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const int half = static_cast<int>(input.size() / 2);
        int sum = 0;
        for (int i = 0; i < input.size(); ++i) {
            const int j = (i + half) % static_cast<int>(input.size());
            if (input[i] == input[j]) {
                sum += static_cast<int>(input[i] - '0');
            }
        }
        return std::to_string(sum);
    }

    aoc::registration r{2017, 1, part_1, part_2};

//    TEST_SUITE("2017_day01") {
//        TEST_CASE("2017_day01:example") {
//
//        }
//    }

} /* namespace <anon> */