//
// Created by Dan on 12/8/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <array>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    grid<int> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_8_input.txt");
        grid<int> retval{lines.size(), lines.front().size()};
        for (std::size_t r = 0; r < retval.num_rows(); ++r) {
            for (std::size_t c = 0; c < retval.num_cols(); ++c) {
                retval[r][c] = lines[r][c] - '0';
            }
        }
        return retval;
    }

    bool is_visible(const grid<int>& g, position p) {
        if (g.cardinal_neighbors(p).size() < 4) {
            return true;
        }

        const int height = g[p];

        const auto row = g.row(p.x);
        for (auto idx = p.y - 1; idx >= 0; --idx) {
            if (row[idx] >= height) {
                break;
            }
            else if (idx == 0) {
                return true;
            }
        }
        for (auto idx = p.y + 1; idx < g.num_cols(); ++idx) {
            if (row[idx] >= height) {
                break;
            }
            else if (idx == g.num_cols() - 1) {
                return true;
            }
        }

        const auto col = g.column(p.y);
        for (auto idx = p.x - 1; idx >= 0; --idx) {
            if (col[idx] >= height) {
                break;
            }
            else if (idx == 0) {
                return true;
            }
        }
        for (auto idx = p.x + 1; idx < g.num_rows(); ++idx) {
            if (col[idx] >= height) {
                break;
            }
            else if (idx == g.num_rows() - 1) {
                return true;
            }
        }

        return false;
    }

    int calc_scenic_score(const grid<int>& g, position p) {
        const int height = g[p];

        const auto row = g.row(p.x);
        std::array<int, 4> dists{};
        for (auto idx = p.y - 1; idx >= 0; --idx) {
            ++dists[0];
            if (row[idx] >= height) {
                break;
            }
        }
        for (auto idx = p.y + 1; idx < g.num_cols(); ++idx) {
            ++dists[1];
            if (row[idx] >= height) {
                break;
            }
        }

        const auto col = g.column(p.y);
        for (auto idx = p.x - 1; idx >= 0; --idx) {
            ++dists[2];
            if (col[idx] >= height) {
                break;
            }
        }
        for (auto idx = p.x + 1; idx < g.num_rows(); ++idx) {
            ++dists[3];
            if (col[idx] >= height) {
                break;
            }
        }

        return dists[0] * dists[1] * dists[2] * dists[3];
    }

    /*

    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto pos_list = input.list_positions();
        const auto num_visible = std::count_if(pos_list.begin(), pos_list.end(), [&input](position p){ return is_visible(input, p); });
        return std::to_string(num_visible);
    }

    /*

    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto pos_list = input.list_positions();
        int best_score = 0;
        for (const auto pos : pos_list) {
            const auto score = calc_scenic_score(input, pos);
            if (score > best_score) {
                best_score = score;
            }
        }
        return std::to_string(best_score);
    }

    aoc::registration r{2022, 8, part_1, part_2};

//    TEST_SUITE("2022_day08") {
//        TEST_CASE("2022_day08:example") {
//
//        }
//    }

}