//
// Created by Dan on 12/13/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <iostream>
#include <array>

#include "utilities.h"
#include "letters.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::pair<int, int> parse_coord(std::string_view s) {
        const auto parts = split(s, ',');
        int x = 0, y = 0;
        auto res = std::from_chars(parts[0].data(), parts[0].data() + parts[0].size(), x);
        if (res.ec != std::errc{}) {
            throw std::system_error{std::make_error_code(res.ec)};
        }
        res = std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), y);
        if (res.ec != std::errc{}) {
            throw std::system_error{std::make_error_code(res.ec)};
        }
        return {x, y};
    }

    struct fold {
        int idx = 0;
        bool along_x = false;
    };

    fold parse_fold(std::string_view s) {
        const auto parts = split(s, '=');
        int idx = 0;
        auto res = std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), idx);
        if (res.ec != std::errc{}) {
            throw std::system_error{std::make_error_code(res.ec)};
        }
        return {idx, parts[0].back() == 'x'};
    }

    grid<char> coords_to_grid(const std::vector<std::pair<int, int>>& coords) {
        int max_x = 0, max_y = 0;
        for (const auto& p : coords) {
            if (p.second > max_x) {
                max_x = p.second;
            }
            if (p.first > max_y) {
                max_y = p.first;
            }
        }
        std::vector<char> data ((max_x + 1) * (max_y + 1), '.');
        grid<char> g {data, static_cast<std::size_t>(max_y + 1)};
        for (const auto& c : coords) {
            g[c.second][c.first] = '#';
        }
        return g;
    }

    struct letter_data {
        char letter;
        std::array<char, 30> data;
    };

    std::pair<grid<char>, std::vector<fold>> get_input(const std::vector<std::string>& lines) {
        const auto split = std::find_if(lines.begin(), lines.end(), [](std::string_view l){ return l.empty(); });
        const auto split_idx = std::distance(lines.begin(), split);
        const auto coords = lines |
                std::views::take(split_idx) |
                std::views::transform([](std::string_view s){ return parse_coord(s); }) |
                to<std::vector<std::pair<int, int>>>();
        auto folds = lines |
                std::views::drop(split_idx + 1) |
                std::views::transform([](std::string_view s){ return parse_fold(s); }) |
                to<std::vector<fold>>();
        return {coords_to_grid(coords), std::move(folds)};
    }

    grid<char> fold_grid(const grid<char>& g, const fold& f) {
        std::size_t new_rows = 0, new_cols = 0;
        if (f.along_x) {
            new_rows = g.num_rows();
            new_cols = std::max(f.idx, (static_cast<int>(g.num_cols()) - 1) - f.idx);
        }
        else {
            new_rows = std::max(f.idx, (static_cast<int>(g.num_rows()) - 1) - f.idx);
            new_cols = g.num_cols();
        }
        grid<char> retval {new_rows, new_cols};
        for (const auto p : retval.list_positions()) {
            position other = p;
            if (f.along_x) {
                other.y += 2 * (f.idx - p.y);
            }
            else {
                other.x += 2 * (f.idx - p.x);
            }
            char other_c = '.';
            if (g.in(other)) {
                other_c = g[other];
            }
            retval[p] = g[p] == '#' || other_c == '#' ? '#' : '.';
        }
        return retval;
    }

    /*
    --- Day 13: Transparent Origami ---
    You reach another volcanically active part of the cave. It would be nice if you could do some kind of thermal imaging so you could tell ahead of time which caves are too hot to safely enter.

    Fortunately, the submarine seems to be equipped with a thermal camera! When you activate it, you are greeted with:

    Congratulations on your purchase! To activate this infrared thermal imaging
    camera system, please enter the code found on page 1 of the manual.
    Apparently, the Elves have never used this feature. To your surprise, you manage to find the manual; as you go to open it, page 1 falls out. It's a large sheet of transparent paper! The transparent paper is marked with random dots and includes instructions on how to fold it up (your puzzle input). For example:

    6,10
    0,14
    9,10
    0,3
    10,4
    4,11
    6,0
    6,12
    4,1
    0,13
    10,12
    3,4
    3,0
    8,4
    1,10
    2,14
    8,10
    9,0

    fold along y=7
    fold along x=5
    The first section is a list of dots on the transparent paper. 0,0 represents the top-left coordinate. The first value, x, increases to the right. The second value, y, increases downward. So, the coordinate 3,0 is to the right of 0,0, and the coordinate 0,7 is below 0,0. The coordinates in this example form the following pattern, where # is a dot on the paper and . is an empty, unmarked position:

    ...#..#..#.
    ....#......
    ...........
    #..........
    ...#....#.#
    ...........
    ...........
    ...........
    ...........
    ...........
    .#....#.##.
    ....#......
    ......#...#
    #..........
    #.#........
    Then, there is a list of fold instructions. Each instruction indicates a line on the transparent paper and wants you to fold the paper up (for horizontal y=... lines) or left (for vertical x=... lines). In this example, the first fold instruction is fold along y=7, which designates the line formed by all of the positions where y is 7 (marked here with -):

    ...#..#..#.
    ....#......
    ...........
    #..........
    ...#....#.#
    ...........
    ...........
    -----------
    ...........
    ...........
    .#....#.##.
    ....#......
    ......#...#
    #..........
    #.#........
    Because this is a horizontal line, fold the bottom half up. Some of the dots might end up overlapping after the fold is complete, but dots will never appear exactly on a fold line. The result of doing this fold looks like this:

    #.##..#..#.
    #...#......
    ......#...#
    #...#......
    .#.#..#.###
    ...........
    ...........
    Now, only 17 dots are visible.

    Notice, for example, the two dots in the bottom left corner before the transparent paper is folded; after the fold is complete, those dots appear in the top left corner (at 0,0 and 0,1). Because the paper is transparent, the dot just below them in the result (at 0,3) remains visible, as it can be seen through the transparent paper.

    Also notice that some dots can end up overlapping; in this case, the dots merge together and become a single dot.

    The second fold instruction is fold along x=5, which indicates this line:

    #.##.|#..#.
    #...#|.....
    .....|#...#
    #...#|.....
    .#.#.|#.###
    .....|.....
    .....|.....
    Because this is a vertical line, fold left:

    #####
    #...#
    #...#
    #...#
    #####
    .....
    .....
    The instructions made a square!

    The transparent paper is pretty big, so for now, focus on just completing the first fold. After the first fold in the example above, 17 dots are visible - dots that end up overlapping after the fold is completed count as a single dot.

    How many dots are visible after completing just the first fold instruction on your transparent paper?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [start_grid, folds] = get_input(lines);
        grid<char> folded = fold_grid(start_grid, folds.front());
        return std::to_string(std::count_if(folded.begin(), folded.end(), [](char c){ return c == '#'; }));
    }

    /*
    --- Part Two ---
    Finish folding the transparent paper according to the instructions. The manual says the code is always eight capital letters.

    What code do you use to activate the infrared thermal imaging camera system?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [start_grid, folds] = get_input(lines);
        grid<char> folded = start_grid;
        for (const auto& f : folds) {
            folded = fold_grid(folded, f);
        }
        //std::cout << "Done:\n" << folded << "\n\n";
        std::string retval;
        for (std::size_t col = 0; col < folded.num_cols();) {
            std::vector<stride_span<const char>> cols;
            for (int i = 0; i < 5; ++i, ++col) {
                cols.push_back(folded.const_column_span(col));
            }
            retval.push_back(to_char(cols));
        }
        return retval;
    }

    aoc::registration r {2021, 13, part_1, part_2};

    TEST_SUITE("2021_day13") {
        TEST_CASE("2021_day13:example") {
            const auto start_grid = coords_to_grid({
                {6,10},
                {0,14},
                {9,10},
                {0,3},
                {10,4},
                {4,11},
                {6,0},
                {6,12},
                {4,1},
                {0,13},
                {10,12},
                {3,4},
                {3,0},
                {8,4},
                {1,10},
                {2,14},
                {8,10},
                {9,0}
            });
            const std::vector<fold> folds = {
                {7, false},
                {5, true}
            };

            //std::cout << "Test start:\n" << start_grid << "\n\n";
            grid<char> folded = fold_grid(start_grid, folds.front());
//            for (const auto& f : folds) {
//                folded = fold_grid(folded, f);
//                //std::cout << "After fold:\n" << folded << "\n\n";
//            }
            const auto res = std::count_if(folded.begin(), folded.end(), [](char c){ return c == '#'; });
            //std::cout << "Test result:\n" << folded << "\n\n";
            REQUIRE_EQ(res, 17);
        }
    }

}