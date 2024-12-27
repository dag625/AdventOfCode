//
// Created by Dan on 12/10/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <regex>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    bool check_char(const char c, const char start, const char end, std::vector<char>& stack) {
        if (c == start) {
            stack.push_back(c);
        }
        else if (c == end) {
            if (stack.empty() || stack.back() != start) {
                return false;
            }
            else {
                stack.pop_back();
            }
        }
        return true;
    }

    std::optional<char> find_first_illegal(std::string_view s) {
        std::vector<char> stack;
        for (const char c : s) {
            bool valid = check_char(c, '(', ')', stack) &&
                    check_char(c, '{', '}', stack) &&
                    check_char(c, '[', ']', stack) &&
                    check_char(c, '<', '>', stack);
            if (!valid) {
                return c;
            }
        }
        return std::nullopt;
    }

   int64_t score_remaining(std::string_view s) {
        std::vector<char> stack;
        for (const char c : s) {
            bool valid = check_char(c, '(', ')', stack) &&
                         check_char(c, '{', '}', stack) &&
                         check_char(c, '[', ']', stack) &&
                         check_char(c, '<', '>', stack);
            if (!valid) {
                return c;
            }
        }
        int64_t score = 0;
        for (const char c : std::views::reverse(stack)) {
            score *= 5;
            switch (c) {
                case '(':  score += 1; break;
                case '[':  score += 2; break;
                case '{':  score += 3; break;
                case '<':  score += 4; break;
                default:  break;
            }
        }
        return score;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        int score = 0;
        for (const auto& l : lines) {
            if (const auto res = find_first_illegal(l); res) {
                switch (*res) {
                    case ')':  score += 3; break;
                    case ']':  score += 57; break;
                    case '}':  score += 1197; break;
                    case '>':  score += 25137; break;
                    default:  break;
                }
            }
        }
        return std::to_string(score);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = lines;
        input.erase(std::remove_if(input.begin(), input.end(),
                                   [](std::string_view s){ return find_first_illegal(s).has_value(); }), input.end());
        auto scores = input | std::ranges::views::transform(score_remaining) | to<std::vector<int64_t>>();
        std::ranges::sort(scores);
        return std::to_string(scores[scores.size() / 2]);
    }

    aoc::registration r {2021, 10, part_1, part_2};

//    TEST_SUITE("2021_day10") {
//        TEST_CASE("2021_day10:example") {

//        }
//    }

}