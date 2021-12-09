//
// Created by Dan on 12/8/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <charconv>
#include <regex>
#include <deque>

#include "utilities.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct spot {
        bool in_basin = false;
        uint8_t height;

        spot() = default;
        spot(uint8_t h) : height{h} {}
    };

    grid<uint8_t> to_data(const std::vector<std::string>& lines, const int num_rows = 100, const int num_cols = 100) {
        std::vector<uint8_t> data;
        data.reserve(num_rows*num_cols);
        for (const auto& s : lines) {
            auto nums = s | std::views::transform([](char c){ return static_cast<uint8_t>(c - '0'); });
            data.insert(data.end(), nums.begin(), nums.end());
        }
        return {std::move(data), static_cast<std::size_t>(num_rows)};
    }

    grid<spot> to_spots(const grid<uint8_t>& data) {
        std::vector<spot> spots;
        spots.reserve(data.num_rows()*data.num_cols());
        auto nums = data | std::views::transform([](uint8_t c){ return spot{c}; });
        spots.insert(spots.end(), nums.begin(), nums.end());
        return {std::move(spots), data.num_rows()};
    }

    grid<uint8_t> get_input(const fs::path& input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_9_input.txt");
        return to_data(lines);
    }

    std::vector<int> get_basin_sizes(grid<spot>& spots_grid) {
        std::vector<int> sizes;
        std::deque<position> to_check;
        for (const auto p : spots_grid.list_positions()) {
            if (!spots_grid[p].in_basin && spots_grid[p].height != 9u) {
                to_check.push_back(p);
                int size = 0;
                while (!to_check.empty()) {
                    const auto current = to_check.front();
                    to_check.pop_front();
                    if (spots_grid[current].in_basin) {
                        continue;
                    }
                    spots_grid[current].in_basin = true;
                    ++size;
                    for (const auto& vel : CARDINAL_DIRECTIONS) {
                        const auto n = current + vel;
                        if (spots_grid.in(n) && !spots_grid[n].in_basin && spots_grid[n].height != 9) {
                            to_check.push_back(n);
                        }
                    }
                }
                sizes.push_back(size);
            }
        }
        return sizes;
    }

    /*
    --- Day 9: Smoke Basin ---
    These caves seem to be lava tubes. Parts are even still volcanically active; small hydrothermal vents release smoke into the caves that slowly settles like rain.

    If you can model how the smoke flows through the caves, you might be able to avoid it and be that much safer. The submarine generates a heightmap of the floor of the nearby caves for you (your puzzle input).

    Smoke flows to the lowest point of the area it's in. For example, consider the following heightmap:

    2199943210
    3987894921
    9856789892
    8767896789
    9899965678
    Each number corresponds to the height of a particular location, where 9 is the highest and 0 is the lowest a location can be.

    Your first goal is to find the low points - the locations that are lower than any of its adjacent locations. Most locations have four adjacent locations (up, down, left, and right); locations on the edge or corner of the map have three or two adjacent locations, respectively. (Diagonal locations do not count as adjacent.)

    In the above example, there are four low points, all highlighted: two are in the first row (a 1 and a 0), one is in the third row (a 5), and one is in the bottom row (also a 5). All other locations on the heightmap have some lower adjacent location, and so are not low points.

    The risk level of a low point is 1 plus its height. In the above example, the risk levels of the low points are 2, 1, 6, and 6. The sum of the risk levels of all low points in the heightmap is therefore 15.

    Find all of the low points on your heightmap. What is the sum of the risk levels of all low points on your heightmap?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        int64_t risk = 0;
        for (const auto pos : input.list_positions()) {
            const auto val = input[pos];
            if (std::all_of(std::begin(CARDINAL_DIRECTIONS), std::end(CARDINAL_DIRECTIONS),
                            [&input, pos, val](const velocity& vel){ const auto n = pos + vel; return !input.in(n) || val < input[n]; }))
            {
                risk += val + 1;
            }
        }
        return std::to_string(risk);
    }

    /*
    --- Part Two ---
    Next, you need to find the largest basins so you know what areas are most important to avoid.

    A basin is all locations that eventually flow downward to a single low point. Therefore, every low point has a basin, although some basins are very small. Locations of height 9 do not count as being in any basin, and all other locations will always be part of exactly one basin.

    The size of a basin is the number of locations within the basin, including the low point. The example above has four basins.

    The top-left basin, size 3:

    2199943210
    3987894921
    9856789892
    8767896789
    9899965678
    The top-right basin, size 9:

    2199943210
    3987894921
    9856789892
    8767896789
    9899965678
    The middle basin, size 14:

    2199943210
    3987894921
    9856789892
    8767896789
    9899965678
    The bottom-right basin, size 9:

    2199943210
    3987894921
    9856789892
    8767896789
    9899965678
    Find the three largest basins and multiply their sizes together. In the above example, this is 9 * 14 * 9 = 1134.

    What do you get if you multiply together the sizes of the three largest basins?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto spots_grid = to_spots(get_input(input_dir));
        std::vector<int> sizes = get_basin_sizes(spots_grid);
        std::ranges::sort(sizes, [](int a, int b){ return a > b; });
        return std::to_string(sizes[0] * sizes[1] * sizes[2]);
    }

    aoc::registration r {2021, 9, part_1, part_2};

    TEST_SUITE("2021_day9") {
        TEST_CASE("2021_day9:example") {
            auto grid = to_spots(to_data({
                     "2199943210",
                     "3987894921",
                     "9856789892",
                     "8767896789",
                     "9899965678"
                }, 5, 10));
            auto sizes = get_basin_sizes(grid);
            std::ranges::sort(sizes, [](int a, int b){ return a > b; });
            REQUIRE_EQ(sizes.size(), 4);
            REQUIRE_EQ(sizes[0], 14);
            REQUIRE_EQ(sizes[1], 9);
            REQUIRE_EQ(sizes[2], 9);
            REQUIRE_EQ(sizes[3], 3);
        }
    }

}