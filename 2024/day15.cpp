//
// Created by Dan on 12/15/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/15
     */

    std::pair<grid<char>, std::string> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_15_input.txt");
        const auto mid = std::find(lines.begin(), lines.end(), "");
        return {to_grid(lines.begin(), mid),
                std::ranges::subrange(mid + 1, lines.end()) | std::views::join | std::ranges::to<std::string>()};
    }

    constexpr std::array<velocity, 128> init_vels() {
        std::array<velocity, 128> retval{};
        retval['^'] = {-1, 0};
        retval['v'] = {1, 0};
        retval['<'] = {0, -1};
        retval['>'] = {0, 1};
        return retval;
    }

    constexpr auto DIR_VELS = init_vels();

    position find_current(grid<char>& g) {
        for (const auto p : g.list_positions()) {
            if (g[p] == '@') {
                g[p] = '.';
                return p;
            }
        }
        return {-1, -1};
    }

    position do_step(grid<char>& g, const position current, const char dir) {
        const auto vel = DIR_VELS[dir];
        const auto next = current + vel;
        if (!g.in(next) || g[next] == '#') {
            //Wall or border, nothing moves.
            return current;
        }
        else if (g[next] == '.') {
            //Free space, we can move whatever's in place but that's where it ends.
            std::swap(g[current], g[next]);
            return next;
        }
        else {
            //A box.  See if it can move.  If it does, so do we.
            const auto cascade = do_step(g, next, dir);
            if (cascade != next) {
                std::swap(g[current], g[next]);
                return next;
            }
            else {
                return current;
            }
        }
    }

    position do_steps(grid<char>& g, position current, const std::string& dirs) {
        for (const char dir : dirs) {
            current = do_step(g, current, dir);
        }
        return current;
    }

    std::pair<grid<char>, position> embiggen(const grid<char>& orig, const position start) {
        grid<char> retval {orig.num_rows(), 2 * orig.num_cols()};
        for (int r = 0; r < orig.num_rows(); ++r) {
            for (int c = 0; c < orig.num_cols(); ++c) {
                if (orig[r][c] == '#' || orig[r][c] == '.') {
                    retval[r][2*c]   = orig[r][c];
                    retval[r][2*c+1] = orig[r][c];
                }
                else {
                    retval[r][2*c]   = '[';
                    retval[r][2*c+1] = ']';
                }
            }
        }
        return {std::move(retval), {start.x, 2*start.y}};
    }

    bool can_move(const grid<char>& g, const position current, const char dir, std::vector<position>& affected) {
        const auto vel = DIR_VELS[dir];
        const auto next = current + vel;
        if (!g.in(next) || g[next] == '#') {
            //Wall or border, nothing moves.  Don't bother adding to affected.
            return false;
        }
        else if (g[next] == '.') {
            affected.push_back(current);
            return true;
        }
        else if (g[next] == '[') {
            const bool left = can_move(g, next, dir, affected);
            const bool right = can_move(g, {next.x, next.y + 1}, dir, affected);
            const bool retval = left && right;
            affected.push_back(current);
            return retval;
        }
        else {
            const bool left = can_move(g, {next.x, next.y - 1}, dir, affected);
            const bool right = can_move(g, next, dir, affected);
            const bool retval = left && right;
            affected.push_back(current);
            return retval;
        }
    }

    position do_big_step(grid<char>& g, const position current, const char dir) {
        const auto vel = DIR_VELS[dir];
        const auto next = current + vel;
        if (!g.in(next) || g[next] == '#') {
            //Wall or border, nothing moves.
            return current;
        }
        else if (g[next] == '.') {
            //Free space, we can move whatever's in place but that's where it ends.
            std::swap(g[current], g[next]);
            return next;
        }
        else if (dir == '<' || dir == '>') {
            //A box.  See if it can move.  If it does, so do we.
            const auto cascade = do_big_step(g, next, dir);
            if (cascade != next) {
                std::swap(g[current], g[next]);
                return next;
            }
            else {
                return current;
            }
        }
        else {
            //Up or down, need to be careful about overlapping.
            std::vector<position> affected;
            bool movable = false;
            if (g[next] == '[') {
                const bool left = can_move(g, next, dir, affected);
                const bool right = can_move(g, {next.x, next.y + 1}, dir, affected);
                movable = left && right;
            }
            else {
                const bool left = can_move(g, {next.x, next.y - 1}, dir, affected);
                const bool right = can_move(g, next, dir, affected);
                movable = left && right;
            }
            if (movable) {
                //Sort so the items in the direction of motion are first in the list.
                if (vel.dx > 0) {
                    std::sort(affected.begin(), affected.end(), std::greater{});
                }
                else {
                    std::sort(affected.begin(), affected.end(), std::less{});
                }
                affected.erase(std::unique(affected.begin(), affected.end()), affected.end());

                //Shift everything by vel.
                for (const auto& p : affected) {
                    const auto n = p + vel;
                     std::swap(g[p], g[n]);
                }
                return next;
            }
            else {
                return current;
            }
        }
    }

    position do_big_steps(grid<char>& g, position current, const std::string& dirs) {
        for (const char dir : dirs) {
            current = do_big_step(g, current, dir);
        }
        return current;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        auto [g, dirs] = get_input(input_dir);
        const auto start = find_current(g);
        const auto finish = do_steps(g, start, dirs);
        int64_t sum = 0;
        for (const auto p : g.list_positions()) {
            if (g[p] == 'O') {
                sum += 100 * p.x + p.y;
            }
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        auto [small_g, dirs] = get_input(input_dir);
        const auto small_start = find_current(small_g);
        auto [g, start] = embiggen(small_g, small_start);
        const auto finish = do_big_steps(g, start, dirs);
        int64_t sum = 0;
        for (const auto p : g.list_positions()) {
            if (g[p] == '[') {
                sum += 100 * p.x + p.y;
            }
        }
        return std::to_string(sum);
    }

    aoc::registration r{2024, 15, part_1, part_2};

    TEST_SUITE("2024_day15") {
        TEST_CASE("2024_day15:example_small") {
            const std::vector<std::string> lines {
                    "########",
                    "#..O.O.#",
                    "##@.O..#",
                    "#...O..#",
                    "#.#.O..#",
                    "#...O..#",
                    "#......#",
                    "########"
            };
            const std::string dirs = "<^^>>>vv<v>>v<<";
            auto g = to_grid(lines);
            const auto start = find_current(g);
            const auto finish = do_steps(g, start, dirs);
            int64_t sum = 0;
            for (const auto p : g.list_positions()) {
                if (g[p] == 'O') {
                    sum += 100 * p.x + p.y;
                }
            }
            std::cout << std::endl;
            g.display(std::cout);
            std::cout << std::endl;
            CHECK_EQ(sum, 2028);
        }
        TEST_CASE("2024_day15:example_large") {
            const std::vector<std::string> lines {
                    "##########",
                    "#..O..O.O#",
                    "#......O.#",
                    "#.OO..O.O#",
                    "#..O@..O.#",
                    "#O#..O...#",
                    "#O..O..O.#",
                    "#.OO.O.OO#",
                    "#....O...#",
                    "##########"
            };
            const std::string dirs = "<vv>^<v^>v>^vv^v>v<>v^v<v<^vv<<<^><<><>>v<vvv<>^v^>^<<<><<v<<<v^vv^v>^vvv<<^>^v^^><<>>><>^<<><^vv^^<>vvv<>><^^v>^>vv<>v<<<<v<^v>^<^^>>>^<v<v><>vv>v^v^<>><>>>><^^>vv>v<^^^>>v^v^<^^>v^^>v^<^v>v<>>v^v^<v>v^^<^^vv<<<v<^>>^^^^>>>v^<>vvv^><v<<<>^^^vv^<vvv>^>v<^^^^v<>^>vvvv><>>v^<<^^^^^^><^><>>><>^^<<^^v>>><^<v>^<vv>>v>>>^v><>^v><<<<v>>v<v<v>vvv>^<><<>^><^>><>^v<><^vvv<^^<><v<<<<<><^v<<<><<<^^<v<^^^><^>>^<v^><<<^>>^v<v^v<v^>^>>^v>vv>^<<^v<>><<><<v<<v><>v<^vv<<<>^^v^>^^>>><<^v>>v^v><^^>>^<>vv^<><^^>^^^<><vvvvv^v<v<<>^v<v>v<<^><<><<><<<^^<<<^<<>><<><^^^>^^<>^>v<>^^>vv<^v^v<vv>^<><v<^v>^^^>>>^^vvv^>vvv<>>>^<^>>>>>^<<^v>^vvv<>^<><<v>v^^>>><<^^<>>^v^<v^vv<>v^<<>^<^v^v><^<<<><<^<v><v<>vv>>v><v^<vv<>v^<<^";
            auto g = to_grid(lines);
            const auto start = find_current(g);
            const auto finish = do_steps(g, start, dirs);
            int64_t sum = 0;
            for (const auto p : g.list_positions()) {
                if (g[p] == 'O') {
                    sum += 100 * p.x + p.y;
                }
            }
            CHECK_EQ(sum, 10092);
        }
        TEST_CASE("2024_day15:example_part2") {
            const std::vector<std::string> lines {
                    "#######",
                    "#...#.#",
                    "#.....#",
                    "#..OO@#",
                    "#..O..#",
                    "#.....#",
                    "#######"
            };
            const std::string dirs = "<vv<<^^<<^^";
            auto sg = to_grid(lines);
            const auto sstart = find_current(sg);
            auto [g, start] = embiggen(sg, sstart);
            std::cout << std::endl;
            g.display(std::cout, start);
            std::cout << std::endl;
            const auto finish = do_big_steps(g, start, dirs);
            int64_t sum = 0;
            for (const auto p : g.list_positions()) {
                if (g[p] == '[') {
                    sum += 100 * p.x + p.y;
                }
            }
            std::cout << std::endl;
            g.display(std::cout, finish);
            std::cout << std::endl;
            CHECK_EQ(sum, 618);
        }
    }

} /* namespace <anon> */