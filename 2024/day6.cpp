//
// Created by Dan on 12/6/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <compare>
#include <iostream>
#include <set>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/6
     */

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_6_input.txt");
        return to_grid(lines);
    }

    position find_start(const grid<char>& g) {
        for (const auto p : g.list_positions()) {
            if (g[p] == '^') {
                return p;
            }
        }
        throw std::runtime_error{"No start position found."};
    }

    velocity turn_right(const velocity v) noexcept { return {v.dy, -v.dx}; }
    velocity turn_left(const velocity v) noexcept { return {-v.dy, v.dx}; }
    velocity turn_back(const velocity v) noexcept { return -v; }

    std::pair<position, velocity> walk(const grid<char>& g, const position c, const velocity v) {
        auto next = c + v;
        if (!g.in(next) || g[next] == '.' || g[next] == '^') {
            return {next, v};
        }
        else { //turn right
            return walk(g, c, turn_right(v));
        }
    }

    struct visited {
        position pos;
        velocity in_dir;

        auto operator<=>(const visited& rhs) const noexcept { return std::tie(pos, in_dir) <=> std::tie(rhs.pos, rhs.in_dir); }
        bool operator==(const visited& rhs) const noexcept { return std::tie(pos, in_dir) == std::tie(rhs.pos, rhs.in_dir); }
    };

    bool loops(const grid<char>& g, const position start, const velocity dir) {
        std::vector<visited> seen;
        visited current {start, dir};
        while (g.in(current.pos)) {
            const auto pos = std::lower_bound(seen.begin(), seen.end(), current);
            if (pos == seen.end() || *pos != current) {
                seen.insert(pos, current);
            }
            else {
                return true;
            }

            const auto [new_pos, new_vel] = walk(g, current.pos, current.in_dir);
            current.pos = new_pos;
            current.in_dir = new_vel;
        }
        return false;
    }

    std::vector<position> walk_out(const grid<char>& g, const position start, velocity dir) {
        auto current = start;
        std::vector<position> visited;
        while (g.in(current)) {
            const auto pos = std::lower_bound(visited.begin(), visited.end(), current);
            if (pos == visited.end() || *pos != current) {
                visited.insert(pos, current);
            }

            const auto [new_pos, new_vel] = walk(g, current, dir);
            current = new_pos;
            dir = new_vel;
        }
        return visited;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto current = find_start(input);
        velocity dir {-1, 0};//Up, ^
        std::vector<position> visited = walk_out(input, current, dir);
        return std::to_string(visited.size());
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto current = find_start(input);
        const velocity dir {-1, 0};//Up, ^
        std::vector<position> visited = walk_out(input, current, dir), to_check;
        to_check.reserve(visited.size() * 4);
        for (const auto pos : visited) {
            for (const auto v : CARDINAL_DIRECTIONS) {
                const auto cp = pos + v;
                if (input[cp] == '.') {
                    const auto found = std::lower_bound(to_check.begin(), to_check.end(), cp);
                    if (found == to_check.end() || *found != cp) {
                        to_check.insert(found, cp);
                    }
                }
            }
        }
        int num_good_blocks = 0;
        for (const auto block : to_check) {
            auto blocked = input;
            blocked[block] = '#';
            if (loops(blocked, current, dir)) {
                ++num_good_blocks;
            }
        }
        return std::to_string(num_good_blocks);
        //Brute force takes ~40s on release...
    }

    aoc::registration r{2024, 6, part_1, part_2};

    TEST_SUITE("2024_day06") {
        TEST_CASE("2024_day06:example") {
            std::vector<std::string> str_input = {
                    "....#.....",
                    ".........#",
                    "..........",
                    "..#.......",
                    ".......#..",
                    "..........",
                    ".#..^.....",
                    "........#.",
                    "#.........",
                    "......#..."
            };
            const auto input = to_grid(str_input);
            const auto current = find_start(input);
            const velocity dir {-1, 0};//Up, ^
            int num_good_blocks = 0;
            for (const auto block : input.list_positions()) {
                const char item = input[block];
                if (item == '.') {
                    auto blocked = input;
                    blocked[block] = '#';
                    if (loops(blocked, current, dir)) {
                        ++num_good_blocks;
                    }
                }
            }
            CHECK_EQ(num_good_blocks, 6);
        }
    }

} /* namespace <anon> */