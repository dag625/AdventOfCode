//
// Created by Dan on 12/16/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "grid_dir_dijkstra.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/16
     */

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_16_input.txt");
        return to_grid(lines);
    }

    std::pair<position, position> get_start_end(grid<char>& g) {
        std::pair<position, position> retval;
        for (const auto p : g.list_positions()) {
            if (g[p] == 'S') {
                retval.first = p;
                g[p] = '.';
            }
            else if (g[p] == 'E') {
                retval.second = p;
                g[p] = '.';
            }
        }
        return retval;
    }

    int64_t cost(const velocity to_current, const velocity to_next) {
        const bool is180deg = to_next == -to_current;
        //If we turn by 90deg, then the coordinate that is 0 becomes +/-1, and the coordinate that is +/-1 becomes 0
        //As we are only moving cardinally, we only need to check one coordinate
        //Initial 1 is to actually move.
        const auto is90deg = to_next.dx * to_current.dx == 0 && to_next.dy * to_current.dy == 0;
        return 1 + static_cast<int>(is90deg) * 1000 +
               static_cast<int>(is180deg) * 2000;//Turning around is turning twice
    }

    std::optional<int64_t> cost_p1(const grid<char>& g, const position current, const velocity to_current, const position next) {
        const velocity to_next {next.x - current.x, next.y - current.y};
        const bool is180deg = to_next == -to_current;
        if (!g.in(next) || g[next] == '#' || is180deg) {
            return std::nullopt;
        }
        else {
            return cost(to_current, to_next);
        }
    }

    int count_back_to_path(const grid<grid_dir_dijkstra_pos>& g, position current, const std::vector<position>& path) {
        int retval = 0;
        auto found = std::find(path.begin(), path.end(), current);
        while (found == path.end()) {
            ++retval;
            current = g[current].prev;
            found = std::find(path.begin(), path.end(), current);
        }
        return retval;
    }

    velocity delta(const position to, const position from) {
        return {to.x - from.x, to.y - from.y};
    }

    void find_equal_path_tiles_rec(const grid<grid_dir_dijkstra_pos>& g, const position current, const position prev, std::vector<position>& all) {
        const auto neighbors = g.cardinal_neighbors(current);
        for (const auto n : neighbors) {
            if (g[n].cost == std::numeric_limits<int64_t>::max() || std::find(all.begin(), all.end(), n) != all.end()) {
                //n is a wall or already found.
                continue;
            }
            else if (g[current].prev == n) {
                //On the best path
                all.push_back(n);
                if (g[current].cost > 0) {
                    find_equal_path_tiles_rec(g, n, current, all);
                }
                //else n is the start
            }
            else if (current == prev) {
                //Current is the end, just need the cost to step in
                const auto alt_cost = cost(g[n].to_current, {current.x - n.x, current.y - n.y}) + g[n].cost;
                if (alt_cost == g[current].cost) {
                    all.push_back(n);
                    find_equal_path_tiles_rec(g, n, current, all);
                }
            }
            else {
                //We want to check the cost to go from 'n' to 'prev' is the same as the path itself,
                //to cover the case where there is a difference in the number of turns.
                const velocity n_to_curr = delta(current, n);
                const velocity to_n = delta(n, g[n].prev);
                const velocity to_current = delta(current, g[current].prev);
                const velocity to_prev = delta(prev, current);
                const auto alt_curr_to_prev = cost(n_to_curr, to_prev);
                const auto alt_n_to_curr = cost(to_n, n_to_curr);
                const auto alt_to_n = g[n].cost;
                const auto alt_cost =
                        alt_curr_to_prev + //current to prev
                        alt_n_to_curr + //n to current
                        alt_to_n; // to n
                const auto curr_cost = g[current].cost;
                const auto path_cost = g[prev].cost;
                if (alt_cost == path_cost) {
                    all.push_back(n);
                    find_equal_path_tiles_rec(g, n, current, all);
                }
            }
        }
    }

    std::vector<position> find_equal_path_tiles(const grid<grid_dir_dijkstra_pos>& g, const std::vector<position>& path) {
        std::vector<position> all;
        all.push_back(path.back());
        find_equal_path_tiles_rec(g, path.back(), path.back(), all);
        return all;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        const auto [start, end] = get_start_end(input);
        const velocity init_v {0, 1}; //East
        const auto res = grid_dir_dijkstra(input, start, init_v, &cost_p1);
        return std::to_string(res[end].cost);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        const auto [start, end] = get_start_end(input);
        const velocity init_v {0, 1}; //East
        const auto res = grid_dir_dijkstra(input, start, init_v, &cost_p1);
        const auto path = grid_dir_dijkstra_get_path(res, start, end);
        const auto all = find_equal_path_tiles(res, path);
        return std::to_string(all.size());
    }

    aoc::registration r{2024, 16, part_1, part_2};

    TEST_SUITE("2024_day16") {
        TEST_CASE("2024_day16:example_p1_1") {
            const std::vector<std::string> lines {
                    "###############",
                    "#.......#....E#",
                    "#.#.###.#.###.#",
                    "#.....#.#...#.#",
                    "#.###.#####.#.#",
                    "#.#.#.......#.#",
                    "#.#.#####.###.#",
                    "#...........#.#",
                    "###.#.#####.#.#",
                    "#...#.....#.#.#",
                    "#.#.#.###.#.#.#",
                    "#.....#...#.#.#",
                    "#.###.#.#.#.#.#",
                    "#S..#.....#...#",
                    "###############"
            };
            auto input = to_grid(lines);
            const auto [start, end] = get_start_end(input);
            const velocity init_v {0, 1}; //East
            const auto res = grid_dir_dijkstra(input, start, init_v, &cost_p1);
            const auto path = grid_dir_dijkstra_get_path(res, start, end);
            for (const auto p : path) {
                input[p] = 'O';
            }
            input[start] = 'S';
            input[end] = 'E';
            std::cout << "\nPath:\n";
            input.display(std::cout);
            std::cout << std::endl;
            const auto all = find_equal_path_tiles(res, path);
            CHECK_EQ(res[end].cost, 7036);
            CHECK_EQ(all.size(), 45);
        }
        TEST_CASE("2024_day16:example_p1_2") {
            const std::vector<std::string> lines {
                    "#################",
                    "#...#...#...#..E#",
                    "#.#.#.#.#.#.#.#.#",
                    "#.#.#.#...#...#.#",
                    "#.#.#.#.###.#.#.#",
                    "#...#.#.#.....#.#",
                    "#.#.#.#.#.#####.#",
                    "#.#...#.#.#.....#",
                    "#.#.#####.#.###.#",
                    "#.#.#.......#...#",
                    "#.#.###.#####.###",
                    "#.#.#...#.....#.#",
                    "#.#.#.#####.###.#",
                    "#.#.#.........#.#",
                    "#.#.#.#########.#",
                    "#S#.............#",
                    "#################"
            };
            auto input = to_grid(lines);        const auto [start, end] = get_start_end(input);
            const velocity init_v {0, 1}; //East
            const auto res = grid_dir_dijkstra(input, start, init_v, &cost_p1);
            const auto path = grid_dir_dijkstra_get_path(res, start, end);
            for (const auto p : path) {
                input[p] = 'O';
            }
            input[start] = 'S';
            input[end] = 'E';
            std::cout << "\nPath:\n";
            input.display(std::cout);
            std::cout << std::endl;
            const auto all = find_equal_path_tiles(res, path);
            CHECK_EQ(res[end].cost, 11048);
            CHECK_EQ(all.size(), 64);
        }
    }

} /* namespace <anon> */