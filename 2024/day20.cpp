//
// Created by Dan on 12/20/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/20
     */

    grid<char> get_input(const std::vector<std::string>& lines) {
        return to_grid(lines);
    }

    std::pair<position, position> find_start_end(grid<char>& g) {
        std::pair<position, position> retval;
        bool found_s = false, found_e = false;
        for (const auto p : g.list_positions()) {
            if (g[p] == 'S') {
                retval.first = p;
                g[p] = '.';
                found_s = true;
            }
            else if (g[p] == 'E') {
                retval.second = p;
                g[p] = '.';
                found_e = true;
            }
            if (found_s && found_e) {
                break;
            }
        }
        return retval;
    }

    std::vector<position> get_path(const grid<char>& g, const position start, const position end) {
        std::vector<position> retval;
        retval.push_back(start);
        while (retval.back() != end) {
            const auto neighbors = g.cardinal_neighbors(retval.back());
            for (const auto n : neighbors) {
                if (g[n] == '.') {
                    if (retval.back() == start || n != *(retval.end() - 2)) {
                        retval.push_back(n);
                        break;
                    }
                }
            }
        }
        return retval;
    }

    using cheat = std::ptrdiff_t;

    constexpr std::ptrdiff_t MAX_CHEAT_P1 = 2;
    constexpr std::ptrdiff_t MAX_CHEAT_P2 = 20;

    std::ptrdiff_t manhattan_dist(const position a, const position b) {
        const auto del = b - a;
        return std::abs(del.x) + std::abs(del.y);
    }

    std::vector<cheat> can_cheat(const grid<char>& g, const int64_t from, const std::ptrdiff_t max, const std::vector<position>& path) {
        std::vector<cheat> retval;
        for (int64_t dest = from + 1; dest < path.size(); ++dest) {
            const auto dist = manhattan_dist(path[from], path[dest]);
            if (dist <= max) {
                retval.emplace_back(dest - from - dist);
            }
        }
        return retval;
    }

    std::vector<cheat> find_cheats(const grid<char>& g, const std::ptrdiff_t max, const std::vector<position>& path) {
        std::vector<cheat> retval;
        for (const auto [idx, p] : path | std::views::enumerate) {
            const auto can = can_cheat(g, idx, max, path);
            retval.insert(retval.end(), can.begin(), can.end());
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        const auto [start, end] = find_start_end(input);
        const auto path = get_path(input, start, end);
        const auto cheats = find_cheats(input, MAX_CHEAT_P1, path);
        const auto best = std::count_if(cheats.begin(), cheats.end(), [](const cheat& c){ return c >= 100; });
        return std::to_string(best);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);        const auto [start, end] = find_start_end(input);
        const auto path = get_path(input, start, end);
        const auto cheats = find_cheats(input, MAX_CHEAT_P2, path);
        const auto best = std::count_if(cheats.begin(), cheats.end(), [](const cheat& c){ return c >= 100; });
        return std::to_string(best);
    }

    aoc::registration r{2024, 20, part_1, part_2};

    TEST_SUITE("2024_day20") {
        TEST_CASE("2024_day20:example") {
            const std::vector<std::string> lines {
                "###############",
                "#...#...#.....#",
                "#.#.#.#.#.###.#",
                "#S#...#.#.#...#",
                "#######.#.#.###",
                "#######.#.#...#",
                "#######.#.###.#",
                "###..E#...#...#",
                "###.#######.###",
                "#...###...#...#",
                "#.#####.#.###.#",
                "#.#...#.#.#...#",
                "#.#.#.#.#.#.###",
                "#...#...#...###",
                "###############"
            };
            auto input = to_grid(lines);
            const auto [start, end] = find_start_end(input);
            const auto path = get_path(input, start, end);
            const auto cheats = find_cheats(input, MAX_CHEAT_P1, path);
            CHECK_EQ(cheats.size(), 44);

            auto cheat20s = find_cheats(input, MAX_CHEAT_P2, path);
            std::sort(cheat20s.begin(), cheat20s.end());
            std::vector<cheat> len50s;
            std::copy_if(cheat20s.begin(), cheat20s.end(), std::back_inserter(len50s), [](const cheat& c){ return c == 50; });
            std::sort(len50s.begin(), len50s.end());
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 50; }), 32);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 52; }), 31);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 54; }), 29);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 56; }), 39);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 58; }), 25);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 60; }), 23);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 62; }), 20);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 64; }), 19);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 66; }), 12);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 68; }), 14);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 70; }), 12);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 72; }), 22);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 74; }),  4);
            CHECK_EQ(std::count_if(cheat20s.begin(), cheat20s.end(), [](const cheat& c){ return c == 76; }),  3);
        }
    }

} /* namespace <anon> */