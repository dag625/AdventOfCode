//
// Created by Dan on 12/3/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <regex>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/3
     */

    std::vector<std::string> get_input(const std::vector<std::string>& lines) {
        return lines;
    }

    int64_t totalInLine(const std::string& s) {
        std::regex re {R"(mul\((\d{1,3}),(\d{1,3})\))"};
        int64_t total = 0;
        auto begin = std::sregex_iterator{s.begin(), s.end(), re};
        auto end = std::sregex_iterator{};
        for (auto it = begin; it != end; ++it) {
            auto& match = *it;
            auto v1 = parse<int>(match[1].str()), v2 = parse<int>(match[2].str());
            total += (v1 * v2);
        }
        return total;
    }

    std::pair<int64_t, bool> conditionalTotalInLine(const std::string& s, bool enabled) {
        std::regex re {R"((mul\((\d{1,3}),(\d{1,3})\))|(do\(\))|(don't\(\)))"};
        int64_t total = 0;
        auto begin = std::sregex_iterator{s.begin(), s.end(), re};
        auto end = std::sregex_iterator{};
        for (auto it = begin; it != end; ++it) {
            auto& match = *it;
            if (match[0].str().starts_with("don't")) {
                enabled = false;
            }
            else if (match[0].str().starts_with("do")) {
                enabled = true;
            }
            else if (enabled) {
                auto v1 = parse<int>(match[2].str()), v2 = parse<int>(match[3].str());
                total += (v1 * v2);
            }
        }
        return {total, enabled};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t total = 0;
        for (const auto& l : input) {
            total += totalInLine(l);
        }
        return std::to_string(total);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t total = 0;
        bool enabled = true;
        for (const auto& l : input) {
            const auto [res, resEn] = conditionalTotalInLine(l, enabled);
            total += res;
            enabled = resEn;
        }
        return std::to_string(total);
    }

    aoc::registration r{2024, 3, part_1, part_2};

    TEST_SUITE("2024_day03") {
        TEST_CASE("2024_day03:example") {
            const std::string example1 = "xmul(2,4)%&mul[3,7]!@^do_not_mul(5,5)+mul(32,64]then(mul(11,8)mul(8,5))";
            const auto res1 = totalInLine(example1);
            CHECK_EQ(res1, 161);
            const std::string example2 = "xmul(2,4)&mul[3,7]!^don't()_mul(5,5)+mul(32,64](mul(11,8)undo()?mul(8,5))";
            const auto [res2, enabled] = conditionalTotalInLine(example2, true);
            CHECK_EQ(res2, 48);
        }
    }

} /* namespace <anon> */