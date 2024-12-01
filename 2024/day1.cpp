//
// Created by Dan on 12/1/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <algorithm>

#include "utilities.h"
#include "parse.h"
#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/01
     */

    std::pair<int, int> parse_line(std::string_view l) {
        const auto parts = split_no_empty(l, ' ');
        return { parse<int>(parts[0]), parse<int>(parts[1]) };
    }

    std::vector<std::pair<int, int>> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_1_input.txt");
        return lines |
                std::views::transform(&parse_line) |
                std::ranges::to<std::vector>();
    }

    std::pair<std::vector<int>, std::vector<int>> split_list(const std::vector<std::pair<int, int>>& list) {
        std::pair<std::vector<int>, std::vector<int>> retval;
        retval.first.reserve(list.size());
        retval.second.reserve(list.size());
        for (const auto [a, b] : list) {
            retval.first.push_back(a);
            retval.second.push_back(b);
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto [list1, list2] = split_list(input);
        std::sort(list1.begin(), list1.end());
        std::sort(list2.begin(), list2.end());

//        auto score = std::ranges::fold_left(std::views::zip_transform(
//                [](auto a, auto b){ return static_cast<int64_t>(std::abs(a - b)); }, list1, list2),
//                0ll, std::plus<int64_t>{});

        //This is slightly faster.
        int64_t score = 0;
        for (const auto [a, b] : std::views::zip(list1, list2)) {
            score += static_cast<int64_t>(std::abs(a - b));
        }

        return std::to_string(score);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto [list1, list2] = split_list(input);

        int64_t score = 0;
        for (const auto v : list1) {
            const auto c = std::count(list2.begin(), list2.end(), v);
            score += static_cast<int64_t>(v) * c;
        }

        return std::to_string(score);
    }

    aoc::registration r{2024, 1, part_1, part_2};

//    TEST_SUITE("2024_day1") {
//        TEST_CASE("2024_day1:example") {
//
//        }
//    }

} /* namespace <anon> */