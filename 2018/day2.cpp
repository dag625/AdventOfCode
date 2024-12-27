//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2018/day/2
     */

    std::vector<std::string> get_input(const std::vector<std::string>& lines) {
        return lines;
    }

    std::pair<bool, bool> has_duplicates(const std::string& s) {
        std::array<int, 26> counts{};
        for (const char c : s) {
            ++counts[c - 'a'];
        }
        return {std::any_of(counts.begin(), counts.end(), [](int c){ return c == 2; }),
                std::any_of(counts.begin(), counts.end(), [](int c){ return c == 3; })};
    }

    std::optional<std::string> get_diffs(const std::string& a, const std::string& b) {
        int diffs = 0, diff_idx = -1;
        //Assume same size
        for (int i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) {
                ++diffs;
                diff_idx = i;
            }
        }
        if (diffs == 1) {
            std::string retval = a;
            retval.erase(retval.begin() + diff_idx);
            return retval;
        }
        else {
            return std::nullopt;
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int num2s = 0, num3s = 0;
        for (const auto& s : input) {
            const auto [has2, has3] = has_duplicates(s);
            num2s += static_cast<int>(has2);
            num3s += static_cast<int>(has3);
        }
        return std::to_string(num2s * num3s);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::string same;
        for (int i = 0; i < input.size(); ++i) {
            for (int j = i + 1; j < input.size(); ++j) {
                auto diff = get_diffs(input[i], input[j]);
                if (diff) {
                    same = std::move(*diff);
                    i = static_cast<int>(input.size());
                    break;
                }
            }
        }
        return same;
    }

    aoc::registration r{2018, 2, part_1, part_2};

//    TEST_SUITE("2018_day02") {
//        TEST_CASE("2018_day02:example") {
//
//        }
//    }

} /* namespace <anon> */