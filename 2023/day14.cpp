//
// Created by Dan on 12/13/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_14_input.txt");
        const auto row_len = lines.front().size();
        return {lines | std::views::join | to<std::vector<char>>(), row_len};
    }

    void roll_up_col(grid<char>& g, int col) {
        int next_avail = 0;
        for (int r = 0; r < g.num_rows(); ++r) {
            if (g[{r, col}] == 'O') {
                if (r != next_avail) {
                    g[{next_avail, col}] = 'O';
                    g[{r, col}] = '.';
                }
                ++next_avail;
            }
            else if (g[{r, col}] == '#') {
                next_avail = r + 1;
            }
        }
    }

    void roll_down_col(grid<char>& g, int col) {
        int next_avail = static_cast<int>(g.num_rows()) - 1;
        for (int r = next_avail; r >= 0; --r) {
            if (g[{r, col}] == 'O') {
                if (r != next_avail) {
                    g[{next_avail, col}] = 'O';
                    g[{r, col}] = '.';
                }
                --next_avail;
            }
            else if (g[{r, col}] == '#') {
                next_avail = r - 1;
            }
        }
    }

    void roll_left_row(grid<char>& g, int row) {
        int next_avail = 0;
        for (int c = 0; c < g.num_cols(); ++c) {
            if (g[{row, c}] == 'O') {
                if (c != next_avail) {
                    g[{row, next_avail}] = 'O';
                    g[{row, c}] = '.';
                }
                ++next_avail;
            }
            else if (g[{row, c}] == '#') {
                next_avail = c + 1;
            }
        }
    }

    void roll_right_row(grid<char>& g, int row) {
        int next_avail = static_cast<int>(g.num_cols()) - 1;
        for (int c = next_avail; c >= 0; --c) {
            if (g[{row, c}] == 'O') {
                if (c != next_avail) {
                    g[{row, next_avail}] = 'O';
                    g[{row, c}] = '.';
                }
                --next_avail;
            }
            else if (g[{row, c}] == '#') {
                next_avail = c - 1;
            }
        }
    }

    void roll_up(grid<char>& g) {
        for (int c = 0; c < g.num_cols(); ++c) {
            roll_up_col(g, c);
        }
    }

    void roll_down(grid<char>& g) {
        for (int c = 0; c < g.num_cols(); ++c) {
            roll_down_col(g, c);
        }
    }

    void roll_right(grid<char>& g) {
        for (int r = 0; r < g.num_rows(); ++r) {
            roll_right_row(g, r);
        }
    }

    void roll_left(grid<char>& g) {
        for (int r = 0; r < g.num_rows(); ++r) {
            roll_left_row(g, r);
        }
    }

    void spin_cycle(grid<char>& g) {
        roll_up(g);
        roll_left(g);
        roll_down(g);
        roll_right(g);
    }

    int64_t load(const grid<char>& g) {
        int64_t retval = 0;
        for (const auto p : g.list_positions()) {
            if (g[p] == 'O') {
                retval += static_cast<int64_t>(g.num_rows()) - p.x;
            }
        }
        return retval;
    }

    std::pair<int, int> detect_cycle(const std::vector<int64_t>& loads) {
        const auto size = static_cast<int>(loads.size());

        int length = 0;
        //Assume the length > 1 (inspection says it is)
        for (int len = 2; len < loads.size() / 2; ++len) {
            const auto start = size - 2 * len, mid = size - len;
            if (std::equal(loads.begin() + start, loads.begin() + mid, loads.begin() + mid, loads.end())) {
                length = len;
                break;
            }
        }

        for (int offset = 0; offset < size - length; ++offset) {
            if (std::equal(loads.begin() + offset, loads.begin() + offset + length,
                           loads.begin() +  offset + length, loads.begin() + offset + 2 * length))
            {
                return {offset, length};
            }
        }
        return {0, 0};
    }

    /*
    --- Day 14: Parabolic Reflector Dish ---
    You reach the place where all of the mirrors were pointing: a massive parabolic reflector dish attached to the side of another large mountain.

    The dish is made up of many small mirrors, but while the mirrors themselves are roughly in the shape of a parabolic reflector dish, each individual mirror seems to be pointing in slightly the wrong direction. If the dish is meant to focus light, all it's doing right now is sending it in a vague direction.

    This system must be what provides the energy for the lava! If you focus the reflector dish, maybe you can go where it's pointing and use the light to fix the lava production.

    Upon closer inspection, the individual mirrors each appear to be connected via an elaborate system of ropes and pulleys to a large metal platform below the dish. The platform is covered in large rocks of various shapes. Depending on their position, the weight of the rocks deforms the platform, and the shape of the platform controls which ropes move and ultimately the focus of the dish.

    In short: if you move the rocks, you can focus the dish. The platform even has a control panel on the side that lets you tilt it in one of four directions! The rounded rocks (O) will roll when the platform is tilted, while the cube-shaped rocks (#) will stay in place. You note the positions of all of the empty spaces (.) and rocks (your puzzle input). For example:

    O....#....
    O.OO#....#
    .....##...
    OO.#O....O
    .O.....O#.
    O.#..O.#.#
    ..O..#O..O
    .......O..
    #....###..
    #OO..#....
    Start by tilting the lever so all of the rocks will slide north as far as they will go:

    OOOO.#.O..
    OO..#....#
    OO..O##..O
    O..#.OO...
    ........#.
    ..#....#.#
    ..O..#.O.O
    ..O.......
    #....###..
    #....#....
    You notice that the support beams along the north side of the platform are damaged; to ensure the platform doesn't collapse, you should calculate the total load on the north support beams.

    The amount of load caused by a single rounded rock (O) is equal to the number of rows from the rock to the south edge of the platform, including the row the rock is on. (Cube-shaped rocks (#) don't contribute to load.) So, the amount of load caused by each rock in each row is as follows:

    OOOO.#.O.. 10
    OO..#....#  9
    OO..O##..O  8
    O..#.OO...  7
    ........#.  6
    ..#....#.#  5
    ..O..#.O.O  4
    ..O.......  3
    #....###..  2
    #....#....  1
    The total load is the sum of the load caused by all of the rounded rocks. In this example, the total load is 136.

    Tilt the platform so that the rounded rocks all roll north. Afterward, what is the total load on the north support beams?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        roll_up(input);
        const auto res = load(input);
        return std::to_string(res);
    }

    /*
    --- Part Two ---
    The parabolic reflector dish deforms, but not in a way that focuses the beam. To do that, you'll need to move the rocks to the edges of the platform. Fortunately, a button on the side of the control panel labeled "spin cycle" attempts to do just that!

    Each cycle tilts the platform four times so that the rounded rocks roll north, then west, then south, then east. After each tilt, the rounded rocks roll as far as they can before the platform tilts in the next direction. After one cycle, the platform will have finished rolling the rounded rocks in those four directions in that order.

    Here's what happens in the example above after each of the first few cycles:

    After 1 cycle:
    .....#....
    ....#...O#
    ...OO##...
    .OO#......
    .....OOO#.
    .O#...O#.#
    ....O#....
    ......OOOO
    #...O###..
    #..OO#....

    After 2 cycles:
    .....#....
    ....#...O#
    .....##...
    ..O#......
    .....OOO#.
    .O#...O#.#
    ....O#...O
    .......OOO
    #..OO###..
    #.OOO#...O

    After 3 cycles:
    .....#....
    ....#...O#
    .....##...
    ..O#......
    .....OOO#.
    .O#...O#.#
    ....O#...O
    .......OOO
    #...O###.O
    #.OOO#...O
    This process should work if you leave it running long enough, but you're still worried about the north support beams. To make sure they'll survive for a while, you need to calculate the total load on the north support beams after 1000000000 cycles.

    In the above example, after 1000000000 cycles, the total load on the north support beams is 64.

    Run the spin cycle for 1000000000 cycles. Afterward, what is the total load on the north support beams?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        const auto num_to_detect = 200;
        std::vector<int64_t> loads;
        loads.reserve(num_to_detect);
        for (int i = 0; i < num_to_detect; ++i) {
            spin_cycle(input);
            const auto ld = load(input);
            loads.push_back(ld);
        }
        const auto [offset, len] = detect_cycle(loads);
        const auto cycling = 1000000000 - offset - 1;
        const auto cycle_idx = cycling % len;
        const auto load_idx = offset + cycle_idx;
        return std::to_string(loads[load_idx]);
    }

    aoc::registration r{2023, 14, part_1, part_2};

    TEST_SUITE("2023_day14") {
        TEST_CASE("2023_day14:example") {
            std::string data = "O....#...."
                               "O.OO#....#"
                               ".....##..."
                               "OO.#O....O"
                               ".O.....O#."
                               "O.#..O.#.#"
                               "..O..#O..O"
                               ".......O.."
                               "#....###.."
                               "#OO..#....";
            const int row_len = 10;
            grid<char> input {std::vector<char>{data.begin(), data.end()}, row_len};

            const auto num_to_detect = 200;
            std::vector<int64_t> loads;
            loads.reserve(num_to_detect);
            for (int i = 0; i < num_to_detect; ++i) {
                spin_cycle(input);
                const auto ld = load(input);
                loads.push_back(ld);
            }
            const auto [offset, len] = detect_cycle(loads);
            const auto cycling = 1000000000 - offset - 1;
            const auto cycle_idx = cycling % len;
            const auto load_idx = offset + cycle_idx;
            CHECK_EQ(loads[load_idx], 64);
        }
    }

}