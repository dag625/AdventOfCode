//
// Created by Dan on 12/12/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <tuple>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/12
     */

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_12_input.txt");
        return to_grid(lines);
    }

    struct dir {
        velocity v;
        bool is_in;
    };

    /*
     * .O.
     * .XO
     * ...
     */
    constexpr std::array<dir, 2> TYPE1 = {dir{{1, 0}, false}, dir{{0, 1}, false}};
    /*
     * .XO
     * .XX
     * ...
     */
    constexpr std::array<dir, 3> TYPE2 = {dir{{1, 0}, true}, dir{{0, 1}, true}, dir{{1, 1}, false}};

    template <std::size_t N>
    std::array<dir, N> rotate(const std::array<dir, N>& t) {
        //Clockwise
        std::array<dir, N> retval = t;
        for (auto& d : retval) {
            const auto x = d.v.dx;
            d.v.dx = d.v.dy;
            d.v.dy = -x;
        }
        return retval;
    }

    template <std::size_t N>
    int matches(const grid<char>& g, const std::vector<position>& contains, const position p, std::array<dir, N> dirs) {
        const char v = g[p];
        int count = 0;
        for (int i = 0; i < 4; ++i) {
            bool matched = true;
            for (const auto& d : dirs) {
                const auto n = p + d.v;
                const auto found = std::lower_bound(contains.begin(), contains.end(), n);
                matched = matched && ((found != contains.end() && *found == n) == d.is_in);
            }
            if (matched) {
                ++count;
            }
            dirs = rotate(dirs);
        }
        return count;
    }

    int count_corners(const grid<char>& g, const std::vector<position>& contains) {
        int corners = 0;
        for (const auto p : contains) {
            corners += matches(g, contains, p, TYPE1);
            corners += matches(g, contains, p, TYPE2);
        }
        return corners;
    }

    std::tuple<char, int, std::vector<position>> get_price(const position p, const grid<char>& g) {
        const char region = g[p];
        int perimeter = 0;
        std::vector<position> contains, to_check;
        to_check.push_back(p);
        while (!to_check.empty()) {
            const auto current = to_check.front();
            to_check.erase(to_check.begin());

            const auto found = std::lower_bound(contains.begin(), contains.end(), current);
            if (found == contains.end() || *found != current) {
                contains.insert(found, current);
                const auto neighbors = g.cardinal_neighbors(current);
                for (const auto n : neighbors) {
                    if (g[n] != region) {
                        ++perimeter;
                    }
                    else {
                        to_check.push_back(n);
                    }
                }
                if (neighbors.size() < 4) {
                    //Gotta fence the outside
                    perimeter += static_cast<int>(4 - neighbors.size());
                }
            }
        }
        return {region, perimeter, std::move(contains)};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        std::vector<position> visited;
        int total = 0;
        for (const auto p : input.list_positions()) {
            const auto found = std::lower_bound(visited.begin(), visited.end(), p);
            if (found == visited.end() || *found != p) {
                const auto [r, perimeter, contains] = get_price(p, input);
                const int mid = static_cast<int>(visited.size());
                visited.insert(visited.end(), contains.begin(), contains.end());
                std::inplace_merge(visited.begin(), visited.begin() + mid, visited.end());
                total += perimeter * static_cast<int>(contains.size());
            }
        }
        return std::to_string(total);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        std::vector<position> visited;
        int total = 0;
        for (const auto p : input.list_positions()) {
            const auto found = std::lower_bound(visited.begin(), visited.end(), p);
            if (found == visited.end() || *found != p) {
                const auto [r, perimeter, contains] = get_price(p, input);
                const int mid = static_cast<int>(visited.size());
                visited.insert(visited.end(), contains.begin(), contains.end());
                std::inplace_merge(visited.begin(), visited.begin() + mid, visited.end());
                total += static_cast<int>(contains.size()) * count_corners(input, contains);
            }
        }
        return std::to_string(total);
    }

    aoc::registration r{2024, 12, part_1, part_2};

    TEST_SUITE("2024_day12") {
        TEST_CASE("2024_day12:example") {
            const std::vector<std::string> lines {
                    "AAAA",
                    "BBCD",
                    "BBCC",
                    "EEEC"
            };
            const auto input = to_grid(lines);
            std::vector<position> visited;
            int total = 0;
            for (const auto p : input.list_positions()) {
                const auto found = std::lower_bound(visited.begin(), visited.end(), p);
                if (found == visited.end() || *found != p) {
                    const auto [r, perimeter, contains] = get_price(p, input);
                    const int mid = static_cast<int>(visited.size());
                    visited.insert(visited.end(), contains.begin(), contains.end());
                    std::inplace_merge(visited.begin(), visited.begin() + mid, visited.end());
                    fmt::println("Region {} had area {} and perimeter {}.", r, contains.size(), perimeter);
                    total += perimeter * static_cast<int>(contains.size());
                }
            }
            CHECK_EQ(total, 140);
        }
    }

} /* namespace <anon> */