//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/9
     */

    std::string get_input(const std::vector<std::string>& lines) {
        return lines.front();
    }

    int p1_score(const std::string& s) {
        int current_group = 0;
        int score = 0;
        bool in_garbage = false;
        bool ignore = false;
        for (const char c : s) {
            if (ignore) {
                ignore = false;
            }
            else if (c == '!') {
                ignore = true;
            }
            else if (in_garbage && c == '>') {
                in_garbage = false;
            }
            else if (!in_garbage && c == '<') {
                in_garbage = true;
            }
            else if (!in_garbage && c == '{') {
                ++current_group;
                score += current_group;
            }
            else if (!in_garbage && c == '}') {
                --current_group;
            }
        }
        return score;
    }

    int p2_score(const std::string& s) {
        int current_group = 0;
        int score = 0;
        bool in_garbage = false;
        bool ignore = false;
        for (const char c : s) {
            if (ignore) {
                ignore = false;
            }
            else if (c == '!') {
                ignore = true;
            }
            else if (in_garbage && c == '>') {
                in_garbage = false;
            }
            else if (!in_garbage && c == '<') {
                in_garbage = true;
            }
            else if (!in_garbage && c == '{') {
                ++current_group;
            }
            else if (!in_garbage && c == '}') {
                --current_group;
            }
            else if (in_garbage) {
                ++score;
            }
        }
        return score;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto score = p1_score(input);
        return std::to_string(score);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto score = p2_score(input);
        return std::to_string(score);
    }

    aoc::registration r{2017, 9, part_1, part_2};

//    TEST_SUITE("2017_day09") {
//        TEST_CASE("2017_day09:example") {
//
//        }
//    }

} /* namespace <anon> */