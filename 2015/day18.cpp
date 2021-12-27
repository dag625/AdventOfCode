//
// Created by Dan on 12/27/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include "utilities.h"
#include "grid.h"
#include "point_nd.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct cell {
        bool active = false;
        bool was_active = false;

        cell() = default;
        explicit cell(bool act) : active{act} { update(); }
        void update() { was_active = active; }
        void set_on() { active = true; was_active = true; }
    };

    cell to_cell(char c) {
        return cell{c == '#'};
    }

    grid<cell> to_cells(const grid<char>& ch) {
        return {ch.buffer() | std::views::transform(&to_cell) | to<std::vector<cell>>(), ch.num_cols()};
    }

    grid<cell> get_input(const fs::path &input_dir) {
        auto lines = read_file_lines(input_dir / "2015" / "day_18_input.txt");
        return to_cells(to_grid(lines));
    }

    void update(grid<cell>& g, const position p) {
        int num_on = 0;
        for (const auto n : g.neighbors(p)) {
            if (g[n].was_active) {
                ++num_on;
            }
        }
        auto& light = g[p];
        if (light.was_active) {
            light.active = (num_on == 2 || num_on == 3);
        }
        else {
            light.active = num_on == 3;
        }
    }

    void update_stuck(grid<cell>& g, const position p) {
        int num_on = 0;
        const auto nbs = g.neighbors(p);
        if (nbs.size() == 3) {
            //A corner
            return;
        }
        for (const auto n : nbs) {
            if (g[n].was_active) {
                ++num_on;
            }
        }
        auto& light = g[p];
        if (light.was_active) {
            light.active = (num_on == 2 || num_on == 3);
        }
        else {
            light.active = num_on == 3;
        }
    }

    void iterate(grid<cell>& g, void (*update_func)(grid<cell>&, position)) {
        for (const auto pos : g.list_positions()) {
            update_func(g, pos);
        }
        std::for_each(g.begin(), g.end(), [](cell& c){ c.update(); });
    }

    /*
    --- Day 18: Like a GIF For Your Yard ---
    After the million lights incident, the fire code has gotten stricter: now, at most ten thousand lights are allowed. You arrange them in a 100x100 grid.

    Never one to let you down, Santa again mails you instructions on the ideal lighting configuration. With so few lights, he says, you'll have to resort to animation.

    Start by setting your lights to the included initial configuration (your puzzle input). A # means "on", and a . means "off".

    Then, animate your grid in steps, where each step decides the next configuration based on the current one. Each light's next state (either on or off) depends on its current state and the current states of the eight lights adjacent to it (including diagonals). Lights on the edge of the grid might have fewer than eight neighbors; the missing ones always count as "off".

    For example, in a simplified 6x6 grid, the light marked A has the neighbors numbered 1 through 8, and the light marked B, which is on an edge, only has the neighbors marked 1 through 5:

    1B5...
    234...
    ......
    ..123.
    ..8A4.
    ..765.
    The state a light should have next is based on its current state (on or off) plus the number of neighbors that are on:

    A light which is on stays on when 2 or 3 neighbors are on, and turns off otherwise.
    A light which is off turns on if exactly 3 neighbors are on, and stays off otherwise.
    All of the lights update simultaneously; they all consider the same current state before moving to the next.

    Here's a few steps from an example configuration of another 6x6 grid:

    Initial state:
    .#.#.#
    ...##.
    #....#
    ..#...
    #.#..#
    ####..

    After 1 step:
    ..##..
    ..##.#
    ...##.
    ......
    #.....
    #.##..

    After 2 steps:
    ..###.
    ......
    ..###.
    ......
    .#....
    .#....

    After 3 steps:
    ...#..
    ......
    ...#..
    ..##..
    ......
    ......

    After 4 steps:
    ......
    ......
    ..##..
    ..##..
    ......
    ......
    After 4 steps, this example has four lights on.

    In your grid of 100x100 lights, given your initial configuration, how many lights are on after 100 steps?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        auto cells = get_input(input_dir);
        for (int i = 0; i < 100; ++i) {
            iterate(cells, &update);
        }
        return std::to_string(std::count_if(cells.begin(), cells.end(), [](const cell& c){ return c.active; }));
    }

    /*
    --- Part Two ---
    You flip the instructions over; Santa goes on to point out that this is all just an implementation of Conway's Game of Life. At least, it was, until you notice that something's wrong with the grid of lights you bought: four lights, one in each corner, are stuck on and can't be turned off. The example above will actually run like this:

    Initial state:
    ##.#.#
    ...##.
    #....#
    ..#...
    #.#..#
    ####.#

    After 1 step:
    #.##.#
    ####.#
    ...##.
    ......
    #...#.
    #.####

    After 2 steps:
    #..#.#
    #....#
    .#.##.
    ...##.
    .#..##
    ##.###

    After 3 steps:
    #...##
    ####.#
    ..##.#
    ......
    ##....
    ####.#

    After 4 steps:
    #.####
    #....#
    ...#..
    .##...
    #.....
    #.#..#

    After 5 steps:
    ##.###
    .##..#
    .##...
    .##...
    #.#...
    ##...#
    After 5 steps, this example now has 17 lights on.

    In your grid of 100x100 lights, given your initial configuration, but with the four corners always in the on state, how many lights are on after 100 steps?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto cells = get_input(input_dir);
        cells[{0,0}].set_on();
        cells[{0, static_cast<int>(cells.num_cols() - 1)}].set_on();
        cells[{static_cast<int>(cells.num_rows() - 1),0}].set_on();
        cells[{static_cast<int>(cells.num_rows() - 1),static_cast<int>(cells.num_cols() - 1)}].set_on();
        for (int i = 0; i < 100; ++i) {
            iterate(cells, &update_stuck);
        }
        return std::to_string(std::count_if(cells.begin(), cells.end(), [](const cell& c){ return c.active; }));
    }

    aoc::registration r {2015, 18, part_1, part_2};

//    TEST_SUITE("2015_day18") {
//        TEST_CASE("2015_day18:example") {
//
//        }
//    }

}