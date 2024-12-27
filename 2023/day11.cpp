//
// Created by Dan on 12/10/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr char GALAXY = '#';

    grid<char> get_input(const std::vector<std::string>& lines) {
        const auto row_len = lines.front().size();
        return {lines | std::views::join | to<std::vector<char>>(), row_len};
    }

    std::vector<int> empty_rows(const grid<char>& g) {
        std::vector<int> retval;
        retval.reserve(g.num_rows());
        for (int r = 0; r < g.num_rows(); ++r) {
            const auto row = g.row_span(r);
            if (std::all_of(row.begin(), row.end(), [](const char ch){ return ch != GALAXY; })) {
                retval.push_back(r);
            }
        }
        return retval;
    }

    std::vector<int> empty_cols(const grid<char>& g) {
        std::vector<int> retval;
        retval.reserve(g.num_cols());
        for (int c = 0; c < g.num_cols(); ++c) {
            const auto col = g.column_span(c);
            if (std::all_of(col.begin(), col.end(), [](const char ch){ return ch != GALAXY; })) {
                retval.push_back(c);
            }
        }
        return retval;
    }

    std::vector<position> get_galaxies(const grid<char>& g) {
        std::vector<position> retval;
        for (const auto p : g.list_positions()) {
            if (g[p] == GALAXY) {
                retval.push_back(p);
            }
        }
        return retval;
    }

    int64_t get_distance(position a, position b, const std::vector<int>& erow, const std::vector<int>& ecol, int64_t exp_factor = 2) {
        --exp_factor;
        int64_t dist = std::abs(a.x - b.x) + std::abs(a.y - b.y);
        int min_x = std::min(a.x, b.x), min_y = std::min(a.y, b.y), max_x = std::max(a.x, b.x), max_y = std::max(a.y, b.y);
        for (const auto r : erow) {
            if (r > min_x && r < max_x) {
                dist += exp_factor;
            }
            else if (r > max_x) {
                break;
            }
        }
        for (const auto c : ecol) {
            if (c > min_y && c < max_y) {
                dist += exp_factor;
            }
            else if (c > max_y) {
                break;
            }
        }
        return dist;
    }

    /*
    --- Day 11: Cosmic Expansion ---
    You continue following signs for "Hot Springs" and eventually come across an observatory. The Elf within turns out to be a researcher studying cosmic expansion using the giant telescope here.

    He doesn't know anything about the missing machine parts; he's only visiting for this research project. However, he confirms that the hot springs are the next-closest area likely to have people; he'll even take you straight there once he's done with today's observation analysis.

    Maybe you can help him with the analysis to speed things up?

    The researcher has collected a bunch of data and compiled the data into a single giant image (your puzzle input). The image includes empty space (.) and galaxies (#). For example:

    ...#......
    .......#..
    #.........
    ..........
    ......#...
    .#........
    .........#
    ..........
    .......#..
    #...#.....
    The researcher is trying to figure out the sum of the lengths of the shortest path between every pair of galaxies. However, there's a catch: the universe expanded in the time it took the light from those galaxies to reach the observatory.

    Due to something involving gravitational effects, only some space expands. In fact, the result is that any rows or columns that contain no galaxies should all actually be twice as big.

    In the above example, three columns and two rows contain no galaxies:

       v  v  v
     ...#......
     .......#..
     #.........
    >..........<
     ......#...
     .#........
     .........#
    >..........<
     .......#..
     #...#.....
       ^  ^  ^
    These rows and columns need to be twice as big; the result of cosmic expansion therefore looks like this:

    ....#........
    .........#...
    #............
    .............
    .............
    ........#....
    .#...........
    ............#
    .............
    .............
    .........#...
    #....#.......
    Equipped with this expanded universe, the shortest path between every pair of galaxies can be found. It can help to assign every galaxy a unique number:

    ....1........
    .........2...
    3............
    .............
    .............
    ........4....
    .5...........
    ............6
    .............
    .............
    .........7...
    8....9.......
    In these 9 galaxies, there are 36 pairs. Only count each pair once; order within the pair doesn't matter. For each pair, find any shortest path between the two galaxies using only steps that move up, down, left, or right exactly one . or # at a time. (The shortest path between two galaxies is allowed to pass through another galaxy.)

    For example, here is one of the shortest paths between galaxies 5 and 9:

    ....1........
    .........2...
    3............
    .............
    .............
    ........4....
    .5...........
    .##.........6
    ..##.........
    ...##........
    ....##...7...
    8....9.......
    This path has length 9 because it takes a minimum of nine steps to get from galaxy 5 to galaxy 9 (the eight locations marked # plus the step onto galaxy 9 itself). Here are some other example shortest path lengths:

    Between galaxy 1 and galaxy 7: 15
    Between galaxy 3 and galaxy 6: 17
    Between galaxy 8 and galaxy 9: 5
    In this example, after expanding the universe, the sum of the shortest path between all 36 pairs of galaxies is 374.

    Expand the universe, then find the length of the shortest path between every pair of galaxies. What is the sum of these lengths?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto erows = empty_rows(input), ecols = empty_cols(input);
        const auto galaxies = get_galaxies(input);
        int64_t sum = 0;
        for (int i = 0; i < galaxies.size(); ++i) {
            for (int j = i + 1; j < galaxies.size(); ++j) {
                sum += get_distance(galaxies[i], galaxies[j], erows, ecols);
            }
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    The galaxies are much older (and thus much farther apart) than the researcher initially estimated.

    Now, instead of the expansion you did before, make each empty row or column one million times larger. That is, each empty row should be replaced with 1000000 empty rows, and each empty column should be replaced with 1000000 empty columns.

    (In the example above, if each empty row or column were merely 10 times larger, the sum of the shortest paths between every pair of galaxies would be 1030. If each empty row or column were merely 100 times larger, the sum of the shortest paths between every pair of galaxies would be 8410. However, your universe will need to expand far beyond these values.)

    Starting with the same initial image, expand the universe according to these new rules, then find the length of the shortest path between every pair of galaxies. What is the sum of these lengths?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto erows = empty_rows(input), ecols = empty_cols(input);
        const auto galaxies = get_galaxies(input);
        int64_t sum = 0;
        for (int i = 0; i < galaxies.size(); ++i) {
            for (int j = i + 1; j < galaxies.size(); ++j) {
                sum += get_distance(galaxies[i], galaxies[j], erows, ecols, 1000000);
            }
        }
        return std::to_string(sum);
    }

    aoc::registration r{2023, 11, part_1, part_2};

//    TEST_SUITE("2023_day11") {
//        TEST_CASE("2023_day11:example") {
//
//        }
//    }

}