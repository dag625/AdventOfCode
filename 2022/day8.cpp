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
    --- Day 8: Treetop Tree House ---
    The expedition comes across a peculiar patch of tall trees all planted carefully in a grid. The Elves explain that a previous expedition planted these trees as a reforestation effort. Now, they're curious if this would be a good location for a tree house.

    First, determine whether there is enough tree cover here to keep a tree house hidden. To do this, you need to count the number of trees that are visible from outside the grid when looking directly along a row or column.

    The Elves have already launched a quadcopter to generate a map with the height of each tree (your puzzle input). For example:

    30373
    25512
    65332
    33549
    35390
    Each tree is represented as a single digit whose value is its height, where 0 is the shortest and 9 is the tallest.

    A tree is visible if all of the other trees between it and an edge of the grid are shorter than it. Only consider trees in the same row or column; that is, only look up, down, left, or right from any given tree.

    All of the trees around the edge of the grid are visible - since they are already on the edge, there are no trees to block the view. In this example, that only leaves the interior nine trees to consider:

    The top-left 5 is visible from the left and top. (It isn't visible from the right or bottom since other trees of height 5 are in the way.)
    The top-middle 5 is visible from the top and right.
    The top-right 1 is not visible from any direction; for it to be visible, there would need to only be trees of height 0 between it and an edge.
    The left-middle 5 is visible, but only from the right.
    The center 3 is not visible from any direction; for it to be visible, there would need to be only trees of at most height 2 between it and an edge.
    The right-middle 3 is visible from the right.
    In the bottom row, the middle 5 is visible, but the 3 and 4 are not.
    With 16 trees visible on the edge and another 5 visible in the interior, a total of 21 trees are visible in this arrangement.

    Consider your map; how many trees are visible from outside the grid?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto pos_list = input.list_positions();
        const auto num_visible = std::count_if(pos_list.begin(), pos_list.end(), [&input](position p){ return is_visible(input, p); });
        return std::to_string(num_visible);
    }

    /*
    --- Part Two ---
    Content with the amount of tree cover available, the Elves just need to know the best spot to build their tree house: they would like to be able to see a lot of trees.

    To measure the viewing distance from a given tree, look up, down, left, and right from that tree; stop if you reach an edge or at the first tree that is the same height or taller than the tree under consideration. (If a tree is right on the edge, at least one of its viewing distances will be zero.)

    The Elves don't care about distant trees taller than those found by the rules above; the proposed tree house has large eaves to keep it dry, so they wouldn't be able to see higher than the tree house anyway.

    In the example above, consider the middle 5 in the second row:

    30373
    25512
    65332
    33549
    35390
    Looking up, its view is not blocked; it can see 1 tree (of height 3).
    Looking left, its view is blocked immediately; it can see only 1 tree (of height 5, right next to it).
    Looking right, its view is not blocked; it can see 2 trees.
    Looking down, its view is blocked eventually; it can see 2 trees (one of height 3, then the tree of height 5 that blocks its view).
    A tree's scenic score is found by multiplying together its viewing distance in each of the four directions. For this tree, this is 4 (found by multiplying 1 * 1 * 2 * 2).

    However, you can do even better: consider the tree of height 5 in the middle of the fourth row:

    30373
    25512
    65332
    33549
    35390
    Looking up, its view is blocked at 2 trees (by another tree with a height of 5).
    Looking left, its view is not blocked; it can see 2 trees.
    Looking down, its view is also not blocked; it can see 1 tree.
    Looking right, its view is blocked at 2 trees (by a massive tree of height 9).
    This tree's scenic score is 8 (2 * 2 * 1 * 2); this is the ideal spot for the tree house.

    Consider each tree on your map. What is the highest scenic score possible for any tree?
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