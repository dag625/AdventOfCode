//
// Created by Dan on 12/20/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <compare>
#include <iostream>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct tile_pos {
        position pos;
        position grid_pos;

        [[nodiscard]] bool operator==(const tile_pos& rhs) const = default;
        [[nodiscard]] auto operator<=>(const tile_pos& rhs) const = default;
    };

    std::string int64_to_string(int64_t v) { return std::to_string(v); }

    grid<char> get_input(const std::vector<std::string>& lines) {
        const auto row_len = lines.front().size();
        return {lines | std::views::join | to<std::vector<char>>(), row_len};
    }

    std::vector<position> take_step(const grid<char>& g, const std::vector<position>& current) {
        std::vector<position> retval;
        for (const auto& p : current) {
            const auto neighbors = g.cardinal_neighbors(p);
            for (const auto& n : neighbors) {
                if (g[n] != '#') {
                    retval.push_back(n);
                }
            }
        }
        std::sort(retval.begin(), retval.end());
        retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        return retval;
    }

    std::vector<tile_pos> take_step(const grid<char>& g, const std::vector<tile_pos>& current) {
        std::vector<tile_pos> retval;
        for (const auto& p : current) {
            for (const auto& v : CARDINAL_DIRECTIONS) {
                auto n = p.pos + v;
                position grid_pos = p.grid_pos;
                if (n.x < 0) {
                    n.x = static_cast<int>(g.num_rows()) - 1;
                    --grid_pos.x;
                }
                else if (n.x >= g.num_rows()) {
                    n.x = 0;
                    ++grid_pos.x;
                }
                else if (n.y < 0) {
                    n.y = static_cast<int>(g.num_cols()) - 1;
                    --grid_pos.y;
                }
                else if (n.y >= g.num_cols()) {
                    n.y = 0;
                    ++grid_pos.y;
                }
                if (g[n] != '#') {
                    retval.push_back({n, grid_pos});
                }
            }
        }
        std::sort(retval.begin(), retval.end());
        retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        return retval;
    }

    /*
    --- Day 21: Step Counter ---
    You manage to catch the airship right as it's dropping someone else off on their all-expenses-paid trip to Desert Island! It even helpfully drops you off near the gardener and his massive farm.

    "You got the sand flowing again! Great work! Now we just need to wait until we have enough sand to filter the water for Snow Island and we'll have snow again in no time."

    While you wait, one of the Elves that works with the gardener heard how good you are at solving problems and would like your help. He needs to get his steps in for the day, and so he'd like to know which garden plots he can reach with exactly his remaining 64 steps.

    He gives you an up-to-date map (your puzzle input) of his starting position (S), garden plots (.), and rocks (#). For example:

    ...........
    .....###.#.
    .###.##..#.
    ..#.#...#..
    ....#.#....
    .##..S####.
    .##..#...#.
    .......##..
    .##.#.####.
    .##..##.##.
    ...........
    The Elf starts at the starting position (S) which also counts as a garden plot. Then, he can take one step north, south, east, or west, but only onto tiles that are garden plots. This would allow him to reach any of the tiles marked O:

    ...........
    .....###.#.
    .###.##..#.
    ..#.#...#..
    ....#O#....
    .##.OS####.
    .##..#...#.
    .......##..
    .##.#.####.
    .##..##.##.
    ...........
    Then, he takes a second step. Since at this point he could be at either tile marked O, his second step would allow him to reach any garden plot that is one step north, south, east, or west of any tile that he could have reached after the first step:

    ...........
    .....###.#.
    .###.##..#.
    ..#.#O..#..
    ....#.#....
    .##O.O####.
    .##.O#...#.
    .......##..
    .##.#.####.
    .##..##.##.
    ...........
    After two steps, he could be at any of the tiles marked O above, including the starting position (either by going north-then-south or by going west-then-east).

    A single third step leads to even more possibilities:

    ...........
    .....###.#.
    .###.##..#.
    ..#.#.O.#..
    ...O#O#....
    .##.OS####.
    .##O.#...#.
    ....O..##..
    .##.#.####.
    .##..##.##.
    ...........
    He will continue like this until his steps for the day have been exhausted. After a total of 6 steps, he could reach any of the garden plots marked O:

    ...........
    .....###.#.
    .###.##.O#.
    .O#O#O.O#..
    O.O.#.#.O..
    .##O.O####.
    .##.O#O..#.
    .O.O.O.##..
    .##.#.####.
    .##O.##.##.
    ...........
    In this example, if the Elf's goal was to get exactly 6 more steps today, he could use them to reach any of 16 garden plots.

    However, the Elf actually needs to get 64 steps today, and the map he's handed you is much larger than the example map.

    Starting from the garden plot marked S on your map, how many garden plots could the Elf reach in exactly 64 steps?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto pos_list = input.list_positions();
        const auto found_start = std::find_if(pos_list.begin(), pos_list.end(), [&input](const position p){ return input[p] == 'S'; });
        std::vector<position> locations;
        locations.push_back(*found_start);
        for (int i = 0; i < 64; ++i) {
            locations = take_step(input, locations);
        }
        return std::to_string(locations.size());
    }

    /*
    --- Part Two ---
    The Elf seems confused by your answer until he realizes his mistake: he was reading from a list of his favorite numbers that are both perfect squares and perfect cubes, not his step counter.

    The actual number of steps he needs to get today is exactly 26501365.

    He also points out that the garden plots and rocks are set up so that the map repeats infinitely in every direction.

    So, if you were to look one additional map-width or map-height out from the edge of the example map above, you would find that it keeps repeating:

    .................................
    .....###.#......###.#......###.#.
    .###.##..#..###.##..#..###.##..#.
    ..#.#...#....#.#...#....#.#...#..
    ....#.#........#.#........#.#....
    .##...####..##...####..##...####.
    .##..#...#..##..#...#..##..#...#.
    .......##.........##.........##..
    .##.#.####..##.#.####..##.#.####.
    .##..##.##..##..##.##..##..##.##.
    .................................
    .................................
    .....###.#......###.#......###.#.
    .###.##..#..###.##..#..###.##..#.
    ..#.#...#....#.#...#....#.#...#..
    ....#.#........#.#........#.#....
    .##...####..##..S####..##...####.
    .##..#...#..##..#...#..##..#...#.
    .......##.........##.........##..
    .##.#.####..##.#.####..##.#.####.
    .##..##.##..##..##.##..##..##.##.
    .................................
    .................................
    .....###.#......###.#......###.#.
    .###.##..#..###.##..#..###.##..#.
    ..#.#...#....#.#...#....#.#...#..
    ....#.#........#.#........#.#....
    .##...####..##...####..##...####.
    .##..#...#..##..#...#..##..#...#.
    .......##.........##.........##..
    .##.#.####..##.#.####..##.#.####.
    .##..##.##..##..##.##..##..##.##.
    .................................
    This is just a tiny three-map-by-three-map slice of the inexplicably-infinite farm layout; garden plots and rocks repeat as far as you can see. The Elf still starts on the one middle tile marked S, though - every other repeated S is replaced with a normal garden plot (.).

    Here are the number of reachable garden plots in this new infinite version of the example map for different numbers of steps:

    In exactly 6 steps, he can still reach 16 garden plots.
    In exactly 10 steps, he can reach any of 50 garden plots.
    In exactly 50 steps, he can reach 1594 garden plots.
    In exactly 100 steps, he can reach 6536 garden plots.
    In exactly 500 steps, he can reach 167004 garden plots.
    In exactly 1000 steps, he can reach 668697 garden plots.
    In exactly 5000 steps, he can reach 16733044 garden plots.
    However, the step count the Elf needs is much larger! Starting from the garden plot marked S on your infinite map, how many garden plots could the Elf reach in exactly 26501365 steps?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const int64_t NUM_STEPS = 26501365ll;
        const auto pos_list = input.list_positions();
        const auto found_start = std::find_if(pos_list.begin(), pos_list.end(), [&input](const position p){ return input[p] == 'S'; });
        std::vector<tile_pos> locations;
        locations.push_back({*found_start, {}});
        std::vector<int64_t> counts;
        for (int i = 0; i < (65 + 4 * 131 + 2); ++i) {
            counts.push_back(static_cast<int64_t>(locations.size()));
            locations = take_step(input, locations);
        }

        const int64_t x_0 = 2;
        const int64_t x_1 = 3;
        const int64_t x_2 = 4;
        const int64_t tx_0 = 65 + x_0 * 131;
        const int64_t tx_1 = 65 + x_1 * 131;
        const int64_t tx_2 = 65 + x_2 * 131;
        const int64_t y_0 = counts[tx_0];
        const int64_t y_1 = counts[tx_1];
        const int64_t y_2 = counts[tx_2];

        const auto a = static_cast<double>(y_0) / ((x_0 - x_1) * (x_0 - x_2)) +
                static_cast<double>(y_1) / ((x_1 - x_0) * (x_1 - x_2)) +
                static_cast<double>(y_2) / ((x_2 - x_1) * (x_2 - x_0));
        const auto b = static_cast<double>(y_0 * (-x_1 - x_2)) / ((x_0 - x_1) * (x_0 - x_2)) +
                static_cast<double>(y_1 * (-x_0 - x_2)) / ((x_1 - x_0) * (x_1 - x_2)) +
                static_cast<double>(y_2 * (-x_1 - x_0)) / ((x_2 - x_1) * (x_2 - x_0));
        const auto c = static_cast<double>(y_0 * (x_1 * x_2)) / ((x_0 - x_1) * (x_0 - x_2)) +
                static_cast<double>(y_1 * (x_0 * x_2)) / ((x_1 - x_0) * (x_1 - x_2)) +
                static_cast<double>(y_2 * (x_1 * x_0)) / ((x_2 - x_1) * (x_2 - x_0));

        const auto grid_size = static_cast<int64_t>(input.num_rows());
        const auto xi = (NUM_STEPS - grid_size / 2) / grid_size;
        const auto x = static_cast<double>(xi);
        const auto res = a * x * x + b * x + c;
        const auto result = static_cast<int64_t>(res);

        return std::to_string(result);
    }

    aoc::registration r{2023, 21, part_1, part_2};

//    TEST_SUITE("2023_day21") {
//        TEST_CASE("2023_day21:example") {
//
//        }
//    }

}