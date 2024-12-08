//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"
#include "grid_dijkstra.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/24
     */

    template <std::size_t N>
    std::array<position, N> get_poi(grid<char>& grid) {
        std::array<position, N> poi{};
        for (const auto p : grid.list_positions()) {
            if (isdigit(grid[p])) {
                poi[grid[p] - '0'] = p;
                grid[p] = '.';
            }
        }
        return poi;
    }

    std::pair<grid<char>, std::array<position, 8>> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_24_input.txt");
        auto grid = to_grid(lines);
        auto poi = get_poi<8>(grid);
        return {std::move(grid), poi};
    }

    std::optional<int64_t> move_cost(const grid<char>& g, const position current, const position next) {
        if (!g.in(next) || g[next] == '#') {
            return std::nullopt;
        }
        else {
            return 1;
        }
    }

    template<std::size_t N>
    std::array<std::array<int64_t, N>, N> get_dists(const grid<char>& grid, const std::array<position, N>& poi) {
        std::array<std::array<int64_t, N>, N> dists{};
        for (int i = 0; i < poi.size(); ++i) {
            const auto res = grid_dijkstra(grid, poi[i], &move_cost);
            for (int j = 0; j < poi.size(); ++j) {
                if (i != j) {
                    dists[i][j] = res[poi[j]].cost;
                }
            }
        }
        return dists;
    }

    template<std::size_t N>
    int64_t find_shortest(const std::array<std::array<int64_t, N>, N>& dists) {
        auto order = std::views::iota(1, static_cast<int>(N)) | std::ranges::to<std::vector>();
        int64_t retval = std::numeric_limits<int64_t>::max();
        do {
            int last = 0;
            int64_t len = 0;
            for (const int idx : order) {
                len += dists[last][idx];
                last = idx;
            }
            if (len < retval) {
                retval = len;
            }
        } while (std::next_permutation(order.begin(), order.end()));
        return retval;
    }

    template<std::size_t N>
    int64_t find_shortest_and_return(const std::array<std::array<int64_t, N>, N>& dists) {
        auto order = std::views::iota(1, static_cast<int>(N)) | std::ranges::to<std::vector>();
        int64_t retval = std::numeric_limits<int64_t>::max();
        do {
            int last = 0;
            int64_t len = 0;
            for (const int idx : order) {
                len += dists[last][idx];
                last = idx;
            }
            len += dists[last][0]; //return to start
            if (len < retval) {
                retval = len;
            }
        } while (std::next_permutation(order.begin(), order.end()));
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto [grid, poi] = get_input(input_dir);
        const auto dists = get_dists(grid, poi);
        const auto shortest = find_shortest(dists);
        return std::to_string(shortest);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto [grid, poi] = get_input(input_dir);
        const auto dists = get_dists(grid, poi);
        const auto shortest = find_shortest_and_return(dists);
        return std::to_string(shortest);
    }

    aoc::registration r{2016, 24, part_1, part_2};

    TEST_SUITE("2016_day24") {
        TEST_CASE("2016_day24:example") {
            const std::vector<std::string> lines {
                    "###########",
                    "#0.1.....2#",
                    "#.#######.#",
                    "#4.......3#",
                    "###########"
            };
            auto grid = to_grid(lines);
            auto poi = get_poi<5>(grid);
            const auto dists = get_dists(grid, poi);
            const auto shortest = find_shortest(dists);
            CHECK_EQ(shortest, 14);
        }
    }

} /* namespace <anon> */