//
// Created by Dan on 12/2/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/2
     */

    std::vector<int> parse_line(std::string_view l) {
        const auto parts = split_no_empty(l, ' ');
        return parts |
                std::views::transform([](std::string_view s){ return parse<int>(s); }) |
                std::ranges::to<std::vector>();
    }

    std::vector<std::vector<int>> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_2_input.txt");
        return lines |
                std::views::transform(&parse_line) |
                std::ranges::to<std::vector>();
    }

    bool is_safe(const std::vector<int>& vals) {
        if (!std::is_sorted(vals.begin(), vals.end()) &&
            !std::is_sorted(vals.begin(), vals.end(), std::greater<>{}))
        {
            return false;
        }

        int last = -1;
        for (const int v : vals) {
            if (last >= 0) {
                const int diff = std::abs(v - last);
                if (diff < 1 || diff > 3) {
                    return false;
                }
            }
            last = v;
        }

        return true;
    }

    void drop_value(const std::vector<int>& vals, const int idx, std::vector<int>& new_vals) {
        new_vals.clear();
        for (int i = 0; i < vals.size(); ++i) {
            if (i != idx) {
                new_vals.push_back(vals[i]);
            }
        }
    }

    bool is_safe_removable(const std::vector<int>& vals) {
        std::vector<int> new_vals;
        new_vals.reserve(vals.size());
        for (int i = 0; i < vals.size(); ++i) {
            drop_value(vals, i, new_vals);
            if (is_safe(new_vals)) {
                return true;
            }
        }
        return false;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num_safe = std::count_if(input.begin(), input.end(), &is_safe);
        return std::to_string(num_safe);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num_safe = std::count_if(input.begin(), input.end(), &is_safe_removable);
        return std::to_string(num_safe);
    }

    aoc::registration r{2024, 2, part_1, part_2};

//    TEST_SUITE("2024_day02") {
//        TEST_CASE("2024_day02:example") {
//
//        }
//    }

} /* namespace <anon> */