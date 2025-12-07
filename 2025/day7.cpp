//
// Created by Dan on 12/7/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "grid.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/7
     */

    grid<char> get_input(const std::vector<std::string>& lines) {
        return to_grid(lines);
    }

    std::vector<int> find_start_col(const grid<char>& g) {
        for (int col = 0; col < g.num_cols(); ++col) {
            if (g[{0, col}] == 'S') {
                return {col};
            }
        }
        return {};
    }

    std::pair<std::vector<int>, int> propagate_beam(const grid<char>& g, const int row, const std::vector<int>& prev_cols, std::vector<int64_t>& col_counts) {
        std::vector<int> cols;
        int num_splitters = 0;
        for (const auto col : prev_cols) {
            const position p {row, col};
            if (g.in(p) && g[p] == '^') {
                ++num_splitters;
                cols.push_back(col - 1);
                cols.push_back(col + 1);
                const auto num_in = col_counts[col];
                col_counts[col] = 0;
                col_counts[col-1] += num_in;
                col_counts[col+1] += num_in;
            }
            else {
                cols.push_back(col);
            }
        }
        cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
        return std::make_pair(std::move(cols), num_splitters);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto beam_cols = find_start_col(input);
        std::vector<int64_t> col_counts;
        col_counts.resize(input.num_cols());
        col_counts[beam_cols.front()] = 1;
        int total_splits = 0;
        for (int row = 1; row < input.num_rows(); ++row) {
            auto [new_cols, splits] = propagate_beam(input, row, beam_cols, col_counts);
            total_splits += splits;
            beam_cols.swap(new_cols);
        }
        return std::to_string(total_splits);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto beam_cols = find_start_col(input);
        std::vector<int64_t> col_counts;
        col_counts.resize(input.num_cols());
        col_counts[beam_cols.front()] = 1;
        int total_splits = 0;
        for (int row = 1; row < input.num_rows(); ++row) {
            auto [new_cols, splits] = propagate_beam(input, row, beam_cols, col_counts);
            total_splits += splits;
            beam_cols.swap(new_cols);
        }
        const auto sum = std::accumulate(col_counts.begin(), col_counts.end(), 0ll);
        return std::to_string(sum);
    }

    aoc::registration r {2025, 7, part_1, part_2};

    TEST_SUITE("2025_day07") {
        TEST_CASE("2025_day07:example") {
            const std::vector<std::string> lines {
                ".......S.......",
                "...............",
                ".......^.......",
                "...............",
                "......^.^......",
                "...............",
                ".....^.^.^.....",
                "...............",
                "....^.^...^....",
                "...............",
                "...^.^...^.^...",
                "...............",
                "..^...^.....^..",
                "...............",
                ".^.^.^.^.^...^.",
                "..............."
            };
            const auto input = get_input(lines);
            auto beam_cols = find_start_col(input);
            std::vector<int64_t> col_counts;
            col_counts.resize(input.num_cols());
            col_counts[beam_cols.front()] = 1;
            int total_splits = 0;
            for (int row = 1; row < input.num_rows(); ++row) {
                auto [new_cols, splits] = propagate_beam(input, row, beam_cols, col_counts);
                total_splits += splits;
                beam_cols.swap(new_cols);
            }
            const auto sum = std::accumulate(col_counts.begin(), col_counts.end(), 0ll);

            CHECK_EQ(total_splits, 21);
            CHECK_EQ(sum, 40);
        }
    }

} /* namespace <anon> */