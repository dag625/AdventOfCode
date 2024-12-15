//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2018/day/1
     */

    std::vector<int> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2018" / "day_1_input.txt");
        return lines | std::views::transform([](std::string_view s){ return parse_allow_plus<int>(s); }) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto sum = std::accumulate(input.begin(), input.end(), 0);
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        std::vector<int> seen;
        int freq = 0;
        seen.push_back(freq);
        bool found_twice = false;
        while (!found_twice) {
            for (const int v: input) {
                freq += v;
                const auto found = std::lower_bound(seen.begin(), seen.end(), freq);
                if (found != seen.end() && *found == freq) {
                    found_twice = true;
                    break;
                }
                else {
                    seen.insert(found, freq);
                }
            }
        }
        return std::to_string(freq);
    }

    aoc::registration r{2018, 1, part_1, part_2};

//    TEST_SUITE("2018_day01") {
//        TEST_CASE("2018_day01:example") {
//
//        }
//    }

} /* namespace <anon> */