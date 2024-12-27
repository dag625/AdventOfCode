//
// Created by Dan on 12/5/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <bit>

#include "utilities.h"
#include "parse.h"
#include "grid.h"
#include "grid_dijkstra.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/13
     */

    constexpr position START {1, 1};
    constexpr position P1_DEST {31, 39};

    constexpr uint64_t calc_poly(position p, const uint32_t increment) noexcept {
        return p.x*p.x + 3*p.x + 2*p.x*p.y + p.y + p.y*p.y + increment;
    }

    constexpr char grid_val(position p, const uint32_t increment) noexcept {
        const auto res = calc_poly(p, increment);
        const auto num_ones = std::popcount(res);
        return num_ones % 2 == 1 ? '#' : '.';
    }

    grid<char> create_maze(const std::size_t num_rows, const std::size_t num_cols, const uint32_t increment) {
        grid<char> retval {num_rows, num_cols};
        for (const auto p : retval.list_positions()) {
            retval[p] = grid_val(p, increment);
        }
        return retval;
    }

    std::optional<int64_t> move_cost(const grid<char>& g, const position current, const position next) {
        if ((std::abs(current.x - next.x) + std::abs(current.y - next.y)) != 1 || !g.in(next) || g[next] == '#') {
            return std::nullopt;
        }
        else {
            return 1;
        }
    }

    uint32_t get_input(const std::vector<std::string>& lines) {
        return parse<uint32_t>(lines.front());
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t steps;
        const std::size_t nrows = 64, ncols = 64;
        auto maze = create_maze(nrows, ncols, input);
        const auto result = grid_dijkstra(maze, START, &move_cost);
        //if (result[P1_DEST].cost < std::numeric_limits<int64_t>::max()) {
        steps = result[P1_DEST].cost;
        //}
        return std::to_string(steps);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t steps;
        const std::size_t nrows = 64, ncols = 64;
        auto maze = create_maze(nrows, ncols, input);
        const auto result = grid_dijkstra(maze, START, &move_cost);
        const auto num_reached = std::count_if(result.begin(), result.end(),
                                               [](const grid_dijkstra_pos& cell){ return cell.cost <= 50; });
        return std::to_string(num_reached);
    }

    aoc::registration r{2016, 13, part_1, part_2};

//    TEST_SUITE("2016_day13") {
//        TEST_CASE("2016_day13:example") {
//
//        }
//    }

} /* namespace <anon> */