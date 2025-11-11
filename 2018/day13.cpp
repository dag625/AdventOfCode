//
// Created by Dan on 11/11/2025.
//

#include <iostream>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "grid.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/13

    !!! Note that the problem coordinate system uses columns are constant X and
    rows are constant Y which is the opposite of the aoc::grid coordinates!  We
    should use our coordinates in calculations and then just report the answers
    as "Y,X".
     */

    enum class turn : int  {
        left = 0,
        straight,
        right
    };

    turn cycle(turn t) {
        switch (t) {
        case turn::left: return turn::straight;
        case turn::straight: return turn::right;
        case turn::right: return turn::left;
        }
        throw std::logic_error{fmt::format("Invalid turn:  {}", static_cast<int>(t))};
    }

    velocity vel_from_dir(const char dir) {
        switch (dir) {
        case '^': return velocity{-1, 0};
        case 'v': return velocity{1, 0};
        case '<': return velocity{0, -1};
        case '>': return velocity{0, 1};
        default: throw std::logic_error{fmt::format("Invalid direction:  {}", dir)};
        }
    }

    struct cart {
        position pos;
        turn next_turn = turn::left;
        int id = 0;
        char dir = '\0';

        cart() = default;
        cart(grid<char>& g, int i, position p, turn n = turn::left) : pos{p}, id{i}, next_turn{n}, dir{g[p]} {
            //By inspection, carts aren't at intersections or corners.
            if (dir == '^' || dir == 'v') {
                g[p] = '|';
            }
            else {
                g[p] = '-';
            }
        }

        [[nodiscard]] auto operator<=>(const cart& other) const noexcept {
            //Top row move first, left to right, then on down, row by row.
            return std::tie(pos.x, pos.y) <=> std::tie(other.pos.x, other.pos.y);
        }

        [[nodiscard]] static bool is(const char c) { return c == '^' || c == 'v' || c == '<' || c == '>'; }
        void turn() {
            const auto current = next_turn;
            next_turn = cycle(next_turn);
            if (dir == '^') {
                switch (current) {
                case turn::left: dir = '<'; break;
                case turn::straight: break;
                case turn::right: dir = '>'; break;
                default: throw std::logic_error{fmt::format("Invalid cart or turn:  {} or {}", dir, static_cast<int>(next_turn))};
                }
            }
            else if (dir == 'v') {
                switch (current) {
                case turn::left: dir = '>'; break;
                case turn::straight: break;
                case turn::right: dir = '<'; break;
                default: throw std::logic_error{fmt::format("Invalid cart or turn:  {} or {}", dir, static_cast<int>(next_turn))};
                }
            }
            else if (dir == '<') {
                switch (current) {
                case turn::left: dir = 'v'; break;
                case turn::straight: break;
                case turn::right: dir = '^'; break;
                default: throw std::logic_error{fmt::format("Invalid cart or turn:  {} or {}", dir, static_cast<int>(next_turn))};
                }
            }
            else if (dir == '>') {
                switch (current) {
                case turn::left: dir = '^'; break;
                case turn::straight: break;
                case turn::right: dir = 'v'; break;
                default: throw std::logic_error{fmt::format("Invalid cart or turn:  {} or {}", dir, static_cast<int>(next_turn))};
                }
            }
        }
        [[nodiscard]] velocity next(const grid<char>& g) {
            const char track = g[pos];
            if (track == '+') {
                turn();
            }
            else if (track == '/') {
                switch (dir) {
                case '^': dir = '>'; break;
                case 'v': dir = '<'; break;
                case '<': dir = 'v'; break;
                case '>': dir = '^'; break;
                default: throw std::logic_error{fmt::format("Invalid direction:  {}", dir)};
                }
            }
            else if (track == '\\') {
                switch (dir) {
                case '^': dir = '<'; break;
                case 'v': dir = '>'; break;
                case '<': dir = '^'; break;
                case '>': dir = 'v'; break;
                default: throw std::logic_error{fmt::format("Invalid direction:  {}", dir)};
                }
            }
            return vel_from_dir(dir);
        }
        void step(const grid<char>& g) {
            pos += next(g);
        }
    };

    std::pair<grid<char>, std::vector<cart>> get_input(const std::vector<std::string>& lines) {
        auto g = to_grid(lines);
        std::vector<cart> carts;
        int next_id = 0;
        for (const auto& p : g.list_positions()) {
            const char v = g[p];
            if (cart::is(v)) {
                carts.emplace_back(g, next_id++, p);
            }
        }
        return {std::move(g), std::move(carts)};
    }

    std::optional<position> step(const grid<char>& g, std::vector<cart>& carts) {
        std::optional<position> retval;
        for (auto& c : carts) {
            c.step(g);
            if (!retval && std::any_of(carts.begin(), carts.end(),
                [&c](const cart& c2) { return c.pos == c2.pos && c.id != c2.id; }))
            {
                retval = c.pos;
            }
        }
        return retval;
    }

    void step_and_remove(const grid<char>& g, std::vector<cart>& carts) {
        std::vector<int> ids_to_remove;
        const auto dup = carts;
        for (auto& c : carts) {
            c.step(g);
            const auto found = std::find_if(carts.begin(), carts.end(),
                [&c](const cart& c2) { return c.pos == c2.pos && c.id != c2.id; });
            if (found != carts.end())
            {
                ids_to_remove.push_back(c.id);
                ids_to_remove.push_back(found->id);
            }
        }
        carts.erase(std::remove_if(carts.begin(), carts.end(),
            [&ids_to_remove](const cart& c){ return std::find(ids_to_remove.begin(), ids_to_remove.end(), c.id) != ids_to_remove.end(); }),
            carts.end());
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto [g, carts] = get_input(lines);
        std::optional<position> crash;
        while (!crash) {
            crash = step(g, carts);
        }
        return fmt::format("{},{}", crash->y, crash->x);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto [g, carts] = get_input(lines);
        while (carts.size() > 1) {
            std::sort(carts.begin(), carts.end());
            step_and_remove(g, carts);
        }
        return fmt::format("{},{}", carts.front().pos.y, carts.front().pos.x);
    }

    aoc::registration r {2018, 13, part_1, part_2};

//    TEST_SUITE("2018_day13") {
//        TEST_CASE("2018_day13:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */