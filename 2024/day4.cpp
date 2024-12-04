//
// Created by Dan on 12/4/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <algorithm>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/04
     */

    constexpr std::string_view FIND_XMAS = "XMAS";
    constexpr std::string_view FIND_X_MAS = "MAS";

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_4_input.txt");
        const auto row_len = lines.front().size();
        return {lines | std::views::join | std::ranges::to<std::vector>(), row_len};
    }

    int num_in_span(stride_span<const char> span) {
        int retval = 0;
        for (int i = 0; i < span.size() - FIND_XMAS.size() + 1; ++i) {
            const auto poss = span.sub_span(i, FIND_XMAS.size());
            if (std::equal(FIND_XMAS.begin(), FIND_XMAS.end(), poss.begin(), poss.end()) ||
                std::equal(FIND_XMAS.rbegin(), FIND_XMAS.rend(), poss.begin(), poss.end()))
            {
                ++retval;
            }
        }
        return retval;
    }

    int num_in_diags(const grid<char>& g) {
        int retval = 0;
        for (int r = 0; r < g.num_rows() - FIND_XMAS.size() + 1; ++r) {
            for (int c = 0; c < g.num_cols() - FIND_XMAS.size() + 1; ++c) {
                if ((FIND_XMAS[0] == g.at(r, c) && FIND_XMAS[1] == g.at(r + 1, c + 1) && FIND_XMAS[2] == g.at(r + 2, c + 2) && FIND_XMAS[3] == g.at(r + 3, c + 3)) ||
                    (FIND_XMAS[3] == g.at(r, c) && FIND_XMAS[2] == g.at(r + 1, c + 1) && FIND_XMAS[1] == g.at(r + 2, c + 2) && FIND_XMAS[0] == g.at(r + 3, c + 3)))
                {
                    ++retval;
                }
                if ((FIND_XMAS[0] == g.at(r, c + 3) && FIND_XMAS[1] == g.at(r + 1, c + 2) && FIND_XMAS[2] == g.at(r + 2, c + 1) && FIND_XMAS[3] == g.at(r + 3, c)) ||
                    (FIND_XMAS[3] == g.at(r, c + 3) && FIND_XMAS[2] == g.at(r + 1, c + 2) && FIND_XMAS[1] == g.at(r + 2, c + 1) && FIND_XMAS[0] == g.at(r + 3, c)))
                {
                    ++retval;
                }
            }
        }
        return retval;
    }

    int num_x_mas(const grid<char>& g) {
        int retval = 0;
        for (int r = 0; r < g.num_rows() - 2; ++r) {
            for (int c = 0; c < g.num_cols() - 2; ++c) {
                if (FIND_X_MAS[1] == g.at(r + 1, c + 1)) {
                    if ((FIND_X_MAS[0] == g.at(r + 0, c + 0) && FIND_X_MAS[0] == g.at(r + 2, c + 0) && FIND_X_MAS[2] == g.at(r + 0, c + 2) && FIND_X_MAS[2] == g.at(r + 2, c + 2)) ||
                        (FIND_X_MAS[2] == g.at(r + 0, c + 0) && FIND_X_MAS[0] == g.at(r + 2, c + 0) && FIND_X_MAS[2] == g.at(r + 0, c + 2) && FIND_X_MAS[0] == g.at(r + 2, c + 2)) ||
                        (FIND_X_MAS[2] == g.at(r + 0, c + 0) && FIND_X_MAS[2] == g.at(r + 2, c + 0) && FIND_X_MAS[0] == g.at(r + 0, c + 2) && FIND_X_MAS[0] == g.at(r + 2, c + 2)) ||
                        (FIND_X_MAS[0] == g.at(r + 0, c + 0) && FIND_X_MAS[2] == g.at(r + 2, c + 0) && FIND_X_MAS[0] == g.at(r + 0, c + 2) && FIND_X_MAS[2] == g.at(r + 2, c + 2)))
                    {
                        ++retval;
                    }
                }
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        int retval = 0;
        for (int r = 0; r < input.num_rows(); ++r) {
            retval += num_in_span(input.row_span(r));
        }
        for (int c = 0; c < input.num_cols(); ++c) {
            retval += num_in_span(input.column_span(c));
        }
        retval += num_in_diags(input);
        return std::to_string(retval);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto retval = num_x_mas(input);
        return std::to_string(retval);
    }

    aoc::registration r{2024, 4, part_1, part_2};

//    TEST_SUITE("2024_day04") {
//        TEST_CASE("2024_day04:example") {
//
//        }
//    }

} /* namespace <anon> */