//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <algorithm>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/2
     */

    std::vector<int> parse_line(std::string_view s) {
        const auto parts = split_no_empty(s, '\t');
        return parts | std::views::transform([](std::string_view v){ return parse<int>(v); }) | std::ranges::to<std::vector>();
    }

    std::vector<std::vector<int>> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_2_input.txt");
        return lines | std::views::transform(&parse_line) | std::ranges::to<std::vector>();
    }

    int checksum(const std::vector<int>& v) {
        const auto [min, max] = std::minmax_element(v.begin(), v.end());
        return *max - *min;
    }

    int evenly_divides(const std::vector<int>& v) {
        for (int i = 0; i < v.size(); ++i) {
            for (int j = i + 1; j < v.size(); ++j) {
                if (v[i] % v[j] == 0) {
                    return v[i] / v[j];
                }
                else if (v[j] % v[i] == 0) {
                    return v[j] / v[i];
                }
            }
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto checksums = input | std::views::transform(&checksum);
        const auto sum = std::accumulate(checksums.begin(), checksums.end(), 0);
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto checksums = input | std::views::transform(&evenly_divides);
        const auto sum = std::accumulate(checksums.begin(), checksums.end(), 0);
        return std::to_string(sum);
    }

    aoc::registration r{2017, 2, part_1, part_2};

//    TEST_SUITE("2017_day02") {
//        TEST_CASE("2017_day02:example") {
//
//        }
//    }

} /* namespace <anon> */