//
// Created by Dan on 12/8/2024.
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
     See https://adventofcode.com/2017/day/4
     */

    std::vector<std::string> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_4_input.txt");
        return lines;
    }

    bool is_valid(std::string_view s) {
        auto words = split(s, ' ');
        const auto init_size = words.size();
        std::sort(words.begin(), words.end());
        words.erase(std::unique(words.begin(), words.end()), words.end());
        return init_size == words.size();
    }

    bool is_valid_anagrams(std::string_view s) {
        auto words = split(s, ' ') | std::views::transform([](std::string_view v){
            std::string s {v};
            std::sort(s.begin(), s.end());
            return s;
        }) | std::ranges::to<std::vector>();
        const auto init_size = words.size();
        std::sort(words.begin(), words.end());
        words.erase(std::unique(words.begin(), words.end()), words.end());
        return init_size == words.size();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num_valid = std::count_if(input.begin(), input.end(), &is_valid);
        return std::to_string(num_valid);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num_valid = std::count_if(input.begin(), input.end(), &is_valid_anagrams);
        return std::to_string(num_valid);
    }

    aoc::registration r{2017, 4, part_1, part_2};

//    TEST_SUITE("2017_day04") {
//        TEST_CASE("2017_day04:example") {
//
//        }
//    }

} /* namespace <anon> */