//
// Created by Dan on 12/12/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "grid_dijkstra.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    grid<char> get_input(const std::vector<std::string>& lines) {
        std::vector<char> data;
        data.reserve(lines.size() * lines[0].size());
        for (const auto& l : lines) {
            data.insert(data.end(), l.begin(), l.end());
        }
        return {std::move(data), lines[0].size()};
    }

    std::pair<position, position> find_start_stop(const grid<char>& g) {
        position start, stop;
        int found = 0;
        for (const auto p : g.list_positions()) {
            if (g[p] == 'S') {
                start = p;
                ++found;
            }
            else if (g[p] == 'E') {
                stop = p;
                ++found;
            }
            if (found >= 2) {
                break;
            }
        }
        return {start, stop};
    }

    int normalize(char c) noexcept {
        if (c == 'S') {
            return 0;
        }
        else if (c == 'E') {
            return 25;
        }
        else {
            return static_cast<int>(c - 'a');
        }
    }

    std::optional<int64_t> cost(const grid<char>& g, position current, position next) {
        const auto cval = normalize(g[current]);
        const auto nval = normalize(g[next]);
        return nval - cval < 2 ? std::optional<int64_t>{1} : std::nullopt;
    }

    std::optional<int64_t> rev_cost(const grid<char>& g, position current, position next) {
        const auto cval = normalize(g[current]);
        const auto nval = normalize(g[next]);
        return cval - nval < 2 ? std::optional<int64_t>{1} : std::nullopt;
    }

    /*
    --- Day 12: Hill Climbing Algorithm ---
    You try contacting the Elves using your handheld device, but the river you're following must be too low to get a decent signal.

    You ask the device for a heightmap of the surrounding area (your puzzle input). The heightmap shows the local area from above broken into a grid; the elevation of each square of the grid is given by a single lowercase letter, where a is the lowest elevation, b is the next-lowest, and so on up to the highest elevation, z.

    Also included on the heightmap are marks for your current position (S) and the location that should get the best signal (E). Your current position (S) has elevation a, and the location that should get the best signal (E) has elevation z.

    You'd like to reach E, but to save energy, you should do it in as few steps as possible. During each step, you can move exactly one square up, down, left, or right. To avoid needing to get out your climbing gear, the elevation of the destination square can be at most one higher than the elevation of your current square; that is, if your current elevation is m, you could step to elevation n, but not to elevation o. (This also means that the elevation of the destination square can be much lower than the elevation of your current square.)

    For example:

    Sabqponm
    abcryxxl
    accszExk
    acctuvwj
    abdefghi
    Here, you start in the top-left corner; your goal is near the middle. You could start by moving down or right, but eventually you'll need to head toward the e at the bottom. From there, you can spiral around to the goal:

    v..v<<<<
    >v.vv<<^
    .>vv>E^^
    ..v>>>^^
    ..>>>>>^
    In the above diagram, the symbols indicate whether the path exits each square moving up (^), down (v), left (<), or right (>). The location that should get the best signal is still E, and . marks unvisited squares.

    This path reaches the goal in 31 steps, the fewest possible.

    What is the fewest steps required to move from your current position to the location that should get the best signal?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [start, stop] = find_start_stop(input);
        const auto result = grid_dijkstra(input, start, &cost);
        return std::to_string(result[stop].cost); //429 too low
    }

    /*
    --- Part Two ---
    As you walk up the hill, you suspect that the Elves will want to turn this into a hiking trail. The beginning isn't very scenic, though; perhaps you can find a better starting point.

    To maximize exercise while hiking, the trail should start as low as possible: elevation a. The goal is still the square marked E. However, the trail should still be direct, taking the fewest steps to reach its goal. So, you'll need to find the shortest path from any square at elevation a to the square marked E.

    Again consider the example from above:

    Sabqponm
    abcryxxl
    accszExk
    acctuvwj
    abdefghi
    Now, there are six choices for starting position (five marked a, plus the square marked S that counts as being at elevation a). If you start at the bottom-left square, you can reach the goal most quickly:

    ...v<<<<
    ...vv<<^
    ...v>E^^
    .>v>>>^^
    >^>>>>>^
    This path reaches the goal in only 29 steps, the fewest possible.

    What is the fewest steps required to move starting from any square with elevation a to the location that should get the best signal?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [start, stop] = find_start_stop(input);
        const auto result = grid_dijkstra(input, stop, &rev_cost);
        int64_t min_cost = std::numeric_limits<int64_t>::max();
        for (const auto p : input.list_positions()) {
            if ((input[p] == 'a' || p == start) && min_cost > result[p].cost) {
                min_cost = result[p].cost;
            }
        }
        return std::to_string(min_cost);
    }

    aoc::registration r{2022, 12, part_1, part_2};

    TEST_SUITE("2022_day12") {
        TEST_CASE("2022_day12:example") {
            std::string raw = "SabqponmabcryxxlaccszExkacctuvwjabdefghi";
            std::vector<char> data {raw.begin(), raw.end()};
            grid<char> input {std::move(data), 8};
            const auto [start, stop] = find_start_stop(input);
            const auto result = grid_dijkstra(input, start, &cost);
            CHECK_EQ(result[stop].cost, 31);
        }
    }

}