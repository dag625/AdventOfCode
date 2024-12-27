//
// Created by Dan on 12/12/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<grid<char>> get_input(const std::vector<std::string>& lines) {
        std::vector<grid<char>> retval;
        int start = 0;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines[i].empty()) {
                const auto row_len = lines[start].size();
                retval.emplace_back(lines |
                        std::views::drop(start) |
                        std::views::take(i - start) |
                        std::views::join |
                        to<std::vector<char>>(),
                    row_len);
                start = i + 1;
            }
        }
        retval.emplace_back(lines | std::views::drop(start) | std::views::join | to<std::vector<char>>(), lines.back().size());
        return retval;
    }

    bool is_mirror_row(const int mrow, const grid<char>& g) {
        for (int top = mrow - 1, bot = mrow; top >= 0 && bot < g.num_rows(); --top, ++bot) {
            for (int c = 0; c < g.num_cols(); ++c) {
                //std::cout << mrow << " - Col " << c << ";  Top " << top << ";  Bot " << bot << ":  " << g[{top, c}] << " vs " << g[{bot, c}] << std::endl;
                if (g[{top, c}] != g[{bot, c}]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_mirror_col(const int mcol, const grid<char>& g) {
        for (int left = mcol - 1, right = mcol; left >= 0 && right < g.num_cols(); --left, ++right) {
            for (int r = 0; r < g.num_rows(); ++r) {
                //std::cout << mcol << " - Row " << r << ";  Left " << left << ";  Right " << right << ":  " << g[{r, left}] << " vs " << g[{r, right}] << std::endl;
                if (g[{r, left}] != g[{r, right}]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_smudged_mirror_row(const int mrow, const grid<char>& g) {
        int diffs = 0;
        for (int top = mrow - 1, bot = mrow; top >= 0 && bot < g.num_rows(); --top, ++bot) {
            for (int c = 0; c < g.num_cols(); ++c) {
                //std::cout << mrow << " - Col " << c << ";  Top " << top << ";  Bot " << bot << ":  " << g[{top, c}] << " vs " << g[{bot, c}] << std::endl;
                if (g[{top, c}] != g[{bot, c}]) {
                    ++diffs;
                    if (diffs > 1) {
                        return false;
                    }
                }
            }
        }
        return diffs == 1;
    }

    bool is_smudged_mirror_col(const int mcol, const grid<char>& g) {
        int diffs = 0;
        for (int left = mcol - 1, right = mcol; left >= 0 && right < g.num_cols(); --left, ++right) {
            for (int r = 0; r < g.num_rows(); ++r) {
                //std::cout << mcol << " - Row " << r << ";  Left " << left << ";  Right " << right << ":  " << g[{r, left}] << " vs " << g[{r, right}] << std::endl;
                if (g[{r, left}] != g[{r, right}]) {
                    ++diffs;
                    if (diffs > 1) {
                        return false;
                    }
                }
            }
        }
        return diffs == 1;
    }

    std::optional<int> find_mirror_row(const grid<char>& g, bool (*is_func)(int v, const grid<char>&)) {
        for (int r = 1; r < g.num_rows(); ++r) {
            if (is_func(r, g)) {
                return r;
            }
        }
        return std::nullopt;
    }

    std::optional<int> find_mirror_col(const grid<char>& g, bool (*is_func)(int v, const grid<char>&)) {
        for (int c = 1; c < g.num_cols(); ++c) {
            if (is_func(c, g)) {
                return c;
            }
        }
        return std::nullopt;
    }

    int summarize(const grid<char>& g, bool smudged) {
        const auto row = find_mirror_row(g, smudged ? &is_smudged_mirror_row : &is_mirror_row);
        const auto col = find_mirror_col(g, smudged ? &is_smudged_mirror_col : &is_mirror_col);
        if (row && col) {
            return 100 * *row + *col;
        }
        else if (row) {
            return 100 * *row;
        }
        else if (col) {
            return *col;
        }
        return 0;
    }

    /*

    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto sum = std::accumulate(input.begin(), input.end(), static_cast<int64_t>(0),
                                         [](int64_t tot, const grid<char>& g){ return tot + summarize(g, false); });
        return std::to_string(sum);
    }

    /*

    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto sum = std::accumulate(input.begin(), input.end(), static_cast<int64_t>(0),
                                         [](int64_t tot, const grid<char>& g){ return tot + summarize(g, true); });
        return std::to_string(sum);
    }

    aoc::registration r{2023, 13, part_1, part_2};

    TEST_SUITE("2023_day13") {
        TEST_CASE("2023_day13:example") {
            std::string data1 = "#.##..##."
                                "..#.##.#."
                                "##......#"
                                "##......#"
                                "..#.##.#."
                                "..##..##."
                                "#.#.##.#.";
            const int row_len1 = 9;
            const grid<char> g1 {std::vector<char>{data1.begin(), data1.end()}, row_len1};

            const auto r1 = find_mirror_row(g1, &is_mirror_row);
            const auto c1 = find_mirror_col(g1, &is_mirror_col);
            CHECK(!r1.has_value());
            CHECK(c1.has_value());
            CHECK_EQ(*c1, 5);
            CHECK_EQ(summarize(g1, false), 5);

            std::string data2 = "#...##..#"
                                "#....#..#"
                                "..##..###"
                                "#####.##."
                                "#####.##."
                                "..##..###"
                                "#....#..#";
            const int row_len2 = 9;
            const grid<char> g2 {std::vector<char>{data2.begin(), data2.end()}, row_len2};

            const auto r2 = find_mirror_row(g2, &is_mirror_row);
            const auto c2 = find_mirror_col(g2, &is_mirror_col);
            CHECK(r2.has_value());
            CHECK(!c2.has_value());
            CHECK_EQ(*r2, 4);
            CHECK_EQ(summarize(g2, false), 400);
        }
    }

}