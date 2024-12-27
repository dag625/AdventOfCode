//
// Created by Dan on 12/6/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <iostream>

#include "utilities.h"
#include "grid.h"

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/18
     */

    grid<char> get_input(const std::vector<std::string>& lines, const std::size_t num_rows) {
        grid<char> retval {num_rows, lines.front().size()};
        for (auto& p : retval) {
            p = '.';
        }
        for (int c = 0; c < retval.num_cols(); ++c) {
            retval[0][c] = lines.front()[c];
        }
        return retval;
    }

    char get_trap(const char l, const char c, const char r) {
        return l != r ? '^' : '.';
    }

    char get_trap(const grid<char>& g, const int r, const int c) {
        if (c == 0) {
            return g[r-1][c+1];
        }
        else if (c == g.num_cols() - 1) {
            return g[r-1][c-1];
        }
        else {
            return get_trap(g[r-1][c-1], g[r-1][c], g[r-1][c+1]);
        }
    }

    void set_rows(grid<char>& g) {
        for (int r = 1; r < g.num_rows(); ++r) {
            for (int c = 0; c < g.num_cols(); ++c) {
                g[r][c] = get_trap(g, r, c);
            }
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines, 40);
        set_rows(input);
        const auto num_safe = std::count_if(input.begin(), input.end(), [](const char c){ return c == '.'; });
        return std::to_string(num_safe);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines, 400000);
        set_rows(input);
        const auto num_safe = std::count_if(input.begin(), input.end(), [](const char c){ return c == '.'; });
        return std::to_string(num_safe);
    }

    aoc::registration r{2016, 18, part_1, part_2};

//    TEST_SUITE("2016_day18") {
//        TEST_CASE("2016_day18:example") {
//
//        }
//    }

} /* namespace <anon> */