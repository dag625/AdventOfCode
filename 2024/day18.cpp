//
// Created by Dan on 12/18/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "grid_dijkstra.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/18
     */

    constexpr std::size_t GRID_SIZE = 71; //0-70 inclusive
    constexpr int GRID_MAX = static_cast<int>(GRID_SIZE) - 1; //0-70 inclusive

    position parse_pos(std::string_view s) {
        const auto parts = split(s, ',');
        return {parse<int>(parts[0]), parse<int>(parts[1])};
    }

    std::vector<position> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_18_input.txt");
        return lines | std::views::transform(&parse_pos) | std::ranges::to<std::vector>();
    }

    grid<char> init_grid(const std::vector<position>::const_iterator begin, const std::vector<position>::const_iterator end) {
        grid<char> g {GRID_SIZE, GRID_SIZE};
        for (const auto p : g.list_positions()) {
            const auto found = std::find(begin, end, p);
            if (found == end) {
                g[p] = '.';
            }
            else {
                g[p] = '#';
            }
        }
        return g;
    }

    std::optional<int64_t> cost(const grid<char>& g, position current, position next) {
         if (g.in(next) && g[next] != '#') {
             return 1;
         }
         else {
             return std::nullopt;
         }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto g = init_grid(input.begin(), input.begin() + 1024);
        const auto res = grid_dijkstra(g, {0,0}, &cost);
        const auto path = grid_dijkstra_get_path(res, {0,0}, {GRID_MAX, GRID_MAX});
        return std::to_string(res[{GRID_MAX, GRID_MAX}].cost);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto g = init_grid(input.begin(), input.begin() + 1024);//Known good point from part 1.
        const position goal {GRID_MAX, GRID_MAX};
        auto res = grid_dijkstra(g, {0,0}, &cost);
        auto path = grid_dijkstra_get_path(res, {0,0}, goal);

        auto next = input.begin() + 1024;
        while (next != input.end()) {
            g[*next] = '#';
            const auto found = std::find(path.begin(), path.end(), *next);
            if (found != path.end()) {
                //Blocked the path, re-calculate
                res = grid_dijkstra(g, {0,0}, &cost);
                if (res[goal].cost == std::numeric_limits<int64_t>::max()) {
                    //Never made it, next is the answer.
                    break;
                }
                path = grid_dijkstra_get_path(res, {0,0}, goal);
            }
            //Else we've not blocked the path, don't need to find a new one.
            ++next;
        }
        return fmt::format("{},{}", next->x, next->y);//maybe inverted...?
    }

    aoc::registration r{2024, 18, part_1, part_2};

//    TEST_SUITE("2024_day18") {
//        TEST_CASE("2024_day18:example") {
//
//        }
//    }

} /* namespace <anon> */