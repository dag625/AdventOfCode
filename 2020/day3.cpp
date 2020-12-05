//
// Created by Daniel Garcia on 12/3/20.
//

#include "day3.h"

#include "utilities.h"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <cstdint>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        constexpr char OPEN_SPACE = '.';
        constexpr char TREE_SPACE = '#';
        constexpr char VALID_VALUES[] = { OPEN_SPACE, TREE_SPACE, '\0' };

        std::vector<std::string> get_input(const fs::path& input_dir) {
            auto input = read_file_lines(input_dir / "2020" / "day_3_input.txt");
            if (input.empty() || input.front().empty() ||
                    !std::all_of(input.begin(), input.end(),
                                 [&input](const std::string& a){ return input.front().size() == a.size() &&
                                                    a.find_first_not_of(VALID_VALUES) == std::string::npos; }))
            {
                throw std::runtime_error{"Invalid input data.  Input must be a non-empty "
                                         "set of non-empty lines of the same length containing "
                                         "only '.' and '#' characters."};
            }
            return input;
        }

        struct velocity {
            int dx = 0;
            int dy = 0;
        };

        struct position {
            int x = 0;
            int y = 0;

            position& operator+=(velocity v) noexcept {
                x += v.dx;
                y += v.dy;
                return *this;
            }

            position& operator-=(velocity v) noexcept {
                x -= v.dx;
                y -= v.dy;
                return *this;
            }

            bool wrap(const std::vector<std::string>& map) noexcept {
                if (y < 0 || y >= map.size()) {
                    //False if we are vertically outside the map.
                    return false;
                }
                const auto size = map.front().size();
                while (x < 0) {
                    x += size;
                }
                while (x >= size) {
                    x -= size;
                }
                return true;
            }
        };

        position operator+(position a, position b) noexcept {
            return {a.x + b.x, a.y + b.y};
        }

        position operator-(position a, position b) noexcept {
            return {a.x - b.x, a.y - b.y};
        }

        position operator+(position p, velocity v) noexcept {
            return p += v;
        }

        position operator-(position p, velocity v) noexcept {
            return p -= v;
        }

        position top_left(const std::vector<std::string>&) noexcept {
            return {0, 0};
        }

        int trees_in_space(const std::vector<std::string>& map, const position p) noexcept {
            //We assume wrap() has been called on the position since the last change.
            constexpr int denom = TREE_SPACE - OPEN_SPACE;
            return static_cast<int>(map[p.y][p.x] - OPEN_SPACE) / denom;
        }

        int num_trees_in_path(const std::vector<std::string>& map, position pos, const velocity vel) noexcept {
            int num_trees = 0;
            while (pos.wrap(map)) {
                num_trees += trees_in_space(map, pos);
                pos += vel;
            }
            return num_trees;
        }

    }

    /*
    With the toboggan login problems resolved, you set off toward the airport. While travel by toboggan might be easy, it's certainly not safe: there's very minimal steering and the area is covered in trees. You'll need to see which angles will take you near the fewest trees.

    Due to the local geology, trees in this area only grow on exact integer coordinates in a grid. You make a map (your puzzle input) of the open squares (.) and trees (#) you can see. For example:

    ..##.......
    #...#...#..
    .#....#..#.
    ..#.#...#.#
    .#...##..#.
    ..#.##.....
    .#.#.#....#
    .#........#
    #.##...#...
    #...##....#
    .#..#...#.#
    These aren't the only trees, though; due to something you read about once involving arboreal genetics and biome stability, the same pattern repeats to the right many times:

    ..##.........##.........##.........##.........##.........##.......  --->
    #...#...#..#...#...#..#...#...#..#...#...#..#...#...#..#...#...#..
    .#....#..#..#....#..#..#....#..#..#....#..#..#....#..#..#....#..#.
    ..#.#...#.#..#.#...#.#..#.#...#.#..#.#...#.#..#.#...#.#..#.#...#.#
    .#...##..#..#...##..#..#...##..#..#...##..#..#...##..#..#...##..#.
    ..#.##.......#.##.......#.##.......#.##.......#.##.......#.##.....  --->
    .#.#.#....#.#.#.#....#.#.#.#....#.#.#.#....#.#.#.#....#.#.#.#....#
    .#........#.#........#.#........#.#........#.#........#.#........#
    #.##...#...#.##...#...#.##...#...#.##...#...#.##...#...#.##...#...
    #...##....##...##....##...##....##...##....##...##....##...##....#
    .#..#...#.#.#..#...#.#.#..#...#.#.#..#...#.#.#..#...#.#.#..#...#.#  --->
    You start on the open square (.) in the top-left corner and need to reach the bottom (below the bottom-most row on your map).

    The toboggan can only follow a few specific slopes (you opted for a cheaper model that prefers rational numbers); start by counting all the trees you would encounter for the slope right 3, down 1:

    From your starting position at the top-left, check the position that is right 3 and down 1. Then, check the position that is right 3 and down 1 from there, and so on until you go past the bottom of the map.

    The locations you'd check in the above example are marked here with O where there was an open square and X where there was a tree:

    ..##.........##.........##.........##.........##.........##.......  --->
    #..O#...#..#...#...#..#...#...#..#...#...#..#...#...#..#...#...#..
    .#....X..#..#....#..#..#....#..#..#....#..#..#....#..#..#....#..#.
    ..#.#...#O#..#.#...#.#..#.#...#.#..#.#...#.#..#.#...#.#..#.#...#.#
    .#...##..#..X...##..#..#...##..#..#...##..#..#...##..#..#...##..#.
    ..#.##.......#.X#.......#.##.......#.##.......#.##.......#.##.....  --->
    .#.#.#....#.#.#.#.O..#.#.#.#....#.#.#.#....#.#.#.#....#.#.#.#....#
    .#........#.#........X.#........#.#........#.#........#.#........#
    #.##...#...#.##...#...#.X#...#...#.##...#...#.##...#...#.##...#...
    #...##....##...##....##...#X....##...##....##...##....##...##....#
    .#..#...#.#.#..#...#.#.#..#...X.#.#..#...#.#.#..#...#.#.#..#...#.#  --->
    In this example, traversing the map using this slope would cause you to encounter 7 trees.

    Starting at the top-left corner of your map and following a slope of right 3 and down 1, how many trees would you encounter?
    */
    void solve_day_3_1(const fs::path& input_dir) {
        const auto map = get_input(input_dir);
        std::cout << '\t' << num_trees_in_path(map, top_left(map), {3, 1}) << std::endl;
    }

    /*
    Time to check the rest of the slopes - you need to minimize the probability of a sudden arboreal stop, after all.

    Determine the number of trees you would encounter if, for each of the following slopes, you start at the top-left corner and traverse the map all the way to the bottom:

    Right 1, down 1.
    Right 3, down 1. (This is the slope you already checked.)
    Right 5, down 1.
    Right 7, down 1.
    Right 1, down 2.
    In the above example, these slopes would find 2, 7, 3, 4, and 2 tree(s) respectively; multiplied together, these produce the answer 336.

    What do you get if you multiply together the number of trees encountered on each of the listed slopes?
    */
    void solve_day_3_2(const fs::path& input_dir) {
        const auto map = get_input(input_dir);
        const auto pos = top_left(map);
        const velocity vels[] = { {1, 1}, {3, 1}, {5, 1}, {7, 1}, {1, 2} };
        std::cout << '\t' << std::accumulate(std::begin(vels), std::end(vels), 1LL,
                                             [pos, &map](int64_t acc, velocity vel){ return acc * num_trees_in_path(map, pos, vel); }) << std::endl;
    }

} /* namespace aoc2020 */