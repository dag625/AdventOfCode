//
// Created by Dan on 12/22/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <iostream>
#include <array>

#include "utilities.h"
#include "grid.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    enum class direction : int {
        right = 0,
        down = 1,
        left = 2,
        up = 3,
        none = -1
    };

    std::ostream& operator<<(std::ostream& os, direction d) {
        switch (d) {
            case direction::right: os << "Right"; break;
            case direction::left: os << "Left"; break;
            case direction::up: os << "Up"; break;
            case direction::down: os << "Down"; break;
            default: os << "Unknown"; break;
        }
        return os;
    }

    enum class turn : int {
        left,
        right,
        none
    };

    velocity get_dir(direction d) noexcept {
        switch (d) {
            case direction::right: return {0,1};
            case direction::left: return {0,-1};
            case direction::up: return {-1,0};
            case direction::down: return {1,0};
            default: return {0,0};
        }
    }

    direction reverse(direction d) noexcept {
        switch (d) {
            case direction::right: return direction::left;
            case direction::left: return direction::right;
            case direction::up: return direction::down;
            case direction::down: return direction::up;
            default: return d;
        }
    }

    turn get_turn(char c) noexcept {
        switch (c) {
            case 'R': return turn::right;
            case 'L': return turn::left;
            default: return turn::none;
        }
    }

    direction do_turn(direction current, turn t) noexcept {
        auto c = static_cast<int>(current);
        if (t == turn::right) {
            ++c;
            c %= 4;
        }
        else {
            --c;
            if (c < 0) {
                c = 3;
            }
        }
        return static_cast<direction>(c);
    }

    using path_el = std::variant<int, turn>;

    std::vector<path_el> parse_path(std::string_view s) {
        std::vector<path_el> retval;
        while (!s.empty()) {
            const auto t = get_turn(s.front());
            if (t == turn::none) {
                int val = 0;
                const auto res = std::from_chars(s.data(), s.data() + s.size(), val);
                retval.emplace_back(val);
                s.remove_prefix(std::distance(s.data(), res.ptr));
            }
            else {
                retval.emplace_back(t);
                s.remove_prefix(1);
            }
        }
        return retval;
    }

    std::pair<grid<char>, std::vector<path_el>> lines_to_input(std::vector<std::string> lines) {
        std::size_t num_cols = 0;
        for (const auto& l : lines) {
            if (l.empty()) {
                break;
            }
            if (l.size() > num_cols) {
                num_cols = l.size();
            }
        }
        std::vector<char> grid_raw;
        for (auto& l : lines) {
            if (l.empty()) {
                break;
            }
            if (l.size() < num_cols) {
                l.insert(l.size(), num_cols - l.size(), ' ');
            }
            grid_raw.insert(grid_raw.end(), l.begin(), l.end());
        }
        return {{std::move(grid_raw), num_cols}, parse_path(lines.back())};
    }

    std::pair<grid<char>, std::vector<path_el>> get_input(const std::vector<std::string>& lines) {
        return lines_to_input(lines);
    }

    struct path_walker {
        const grid<char>& map;
        direction& facing;
        position& pos;
        path_walker(const grid<char>& m, direction& d, position& p) : map{m}, facing{d}, pos{p} {}
        void operator()(const turn t) noexcept { facing = do_turn(facing, t); }
        void operator()(const int dist) {
            const auto vel = get_dir(facing);
            for (int i = 0; i < dist; ++i) {
                const auto np = pos + vel;
                if (!map.in(np) || map[np] == ' ') {
                    switch (facing) {
                        case direction::up: wrap_up(np); break;
                        case direction::down: wrap_down(np); break;
                        case direction::left: wrap_left(np); break;
                        case direction::right: wrap_right(np); break;
                        default: break;
                    }
                }
                else if (map[np] == '#') {
                    break;
                }
                else {
                    pos = np;
                }
            }
        }

        void wrap_up(position np) {
            while (np.x < 0 || map[np] == ' ') {
                if (np.x < 0) {
                    np.x = static_cast<int>(map.num_rows()) - 1;
                }
                else {
                    --np.x;
                }
            }
            if (map[np] == '.') {
                pos = np;
            }
        }
        void wrap_down(position np) {
            while (np.x >= map.num_rows() || map[np] == ' ') {
                if (np.x >= map.num_rows()) {
                    np.x = 0;
                }
                else {
                    ++np.x;
                }
            }
            if (map[np] == '.') {
                pos = np;
            }
        }
        void wrap_left(position np) {
            while (np.y < 0 || map[np] == ' ') {
                if (np.y < 0) {
                    np.y = static_cast<int>(map.num_cols()) - 1;
                }
                else {
                    --np.y;
                }
            }
            if (map[np] == '.') {
                pos = np;
            }
        }
        void wrap_right(position np) {
            while (np.y >= map.num_cols() || map[np] == ' ') {
                if (np.y >= map.num_cols()) {
                    np.y = 0;
                }
                else {
                    ++np.y;
                }
            }
            if (map[np] == '.') {
                pos = np;
            }
        }
    };

    struct cube_edge {
        std::pair<position, position> a;
        std::pair<position, position> b;
        direction a_dir;
        direction b_dir;
    };

    /*
--> +y,=x
      +----+----+   |
      | Dd | cC |   |
      |E   |   B|   v
      |e   |   b|   +x,=y
      |    | aA |
      +----+----+
      |    |
      |f  a|
      |F  A|
      |    |
 +----+----+
 | fF |    |
 |e   |   b|
 |E   |   B|
 |    | gG |
 +----+----+
 |    |
 |D  g|
 |d  G|
 | cC |
 +----+

     */

    constexpr std::array<cube_edge, 7> EDGES {
            cube_edge{{{ -1, 50},{ -1, 99}}, {{150, -1},{199, -1}}, direction::up, direction::left},     //Dd
            cube_edge{{{ -1,100},{ -1,149}}, {{200,  0},{200, 49}}, direction::up, direction::down},     //cC
            cube_edge{{{  0, 49},{ 49, 49}}, {{149, -1},{100, -1}}, direction::left, direction::left},   //Ee
            cube_edge{{{  0,150},{ 49,150}}, {{149,100},{100,100}}, direction::right, direction::right}, //Bb
            cube_edge{{{ 50,100},{ 50,149}}, {{ 50,100},{ 99,100}}, direction::down, direction::right},  //aA
            cube_edge{{{ 50, 49},{ 99, 49}}, {{ 99,  0},{ 99, 49}}, direction::left, direction::up},     //fF
            cube_edge{{{150, 50},{150, 99}}, {{150, 50},{199, 50}}, direction::down, direction::right}   //gG
    };

    bool is_in_edge(position pos, const std::pair<position, position>& edge, const direction edge_dir) {
        if (edge_dir == direction::up || edge_dir == direction::down) {
            return edge.first.x == pos.x &&
                pos.y >= std::min(edge.first.y, edge.second.y) &&
                pos.y <= std::max(edge.first.y, edge.second.y);
        }
        else {
            return edge.first.y == pos.y &&
                   pos.x >= std::min(edge.first.x, edge.second.x) &&
                   pos.x <= std::max(edge.first.x, edge.second.x);
        }
    }

    std::pair<position, direction> translate_edge(position pos,
                                                  const std::pair<position, position>& from, const direction from_dir,
                                                  const std::pair<position, position>& to, const direction to_dir)
    {
        const auto from_diff = pos - from.first;
        int idx = 0;
        if (from_diff.x == 0) {
            idx = std::abs(from_diff.y);
        }
        else {
            idx = std::abs(from_diff.x);
        }
        const auto to_del = to_vel((to.second - to.first) / 49);
        position to_pos = to.first;
        for (int i = 0; i < idx; ++i) {
            to_pos += to_del;
        }
        const auto r_dir = reverse(to_dir);
        to_pos += get_dir(r_dir);
        return {to_pos, r_dir};
    }

    std::optional<std::pair<position, direction>> move_with_edge(position pos, direction dir, const cube_edge& edge) {
        if (dir == edge.a_dir && is_in_edge(pos, edge.a, edge.a_dir)) {
            return translate_edge(pos, edge.a, edge.a_dir, edge.b, edge.b_dir);
        }
        else if (dir == edge.b_dir && is_in_edge(pos, edge.b, edge.b_dir)) {
            return translate_edge(pos, edge.b, edge.b_dir, edge.a, edge.a_dir);
        }
        else {
            return std::nullopt;
        }
    }

    std::pair<position, direction> move_with_edge(position pos, direction dir) {
        for (const auto& edge : EDGES) {
            auto result = move_with_edge(pos, dir, edge);
            if (result) {
                return *result;
            }
        }
        return {pos, dir};
    }

    struct cube_walker {
        const grid<char>& map;
        direction& facing;
        position& pos;
        cube_walker(const grid<char>& m, direction& d, position& p) : map{m}, facing{d}, pos{p} {}
        void operator()(const turn t) noexcept { facing = do_turn(facing, t); }
        void operator()(const int dist) {
            for (int i = 0; i < dist; ++i) {
                const auto vel = get_dir(facing);
                const auto np = pos + vel;
                if (!map.in(np) || map[np] == ' ') {
                    const auto [ep, edir] = move_with_edge(np, facing);
                    if (map[ep] == '.') {
                        pos = ep;
                        facing = edir;
                    }
                }
                else if (map[np] == '#') {
                    break;
                }
                else {
                    pos = np;
                }
            }
        }
    };

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [map, path] = get_input(lines);
        direction facing = direction::right;
        int start_y = static_cast<int>(std::distance(map.begin(), std::find_if_not(map.begin(), map.end(), [](char c){ return c == ' '; })));
        position pos {0, start_y};
        for (const auto& p : path) {
            std::visit(path_walker{map, facing, pos}, p);
        }
        return std::to_string(1000 * (pos.x + 1) + 4 * (pos.y + 1) + static_cast<int>(facing));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [map, path] = get_input(lines);
        direction facing = direction::right;
        int start_y = static_cast<int>(std::distance(map.begin(), std::find_if_not(map.begin(), map.end(), [](char c){ return c == ' '; })));
        position pos {0, start_y};
        for (const auto& p : path) {
            std::visit(cube_walker{map, facing, pos}, p);
        }
        return std::to_string(1000 * (pos.x + 1) + 4 * (pos.y + 1) + static_cast<int>(facing));
    }

    aoc::registration r{2022, 22, part_1, part_2};

    TEST_SUITE("2022_day22") {
        TEST_CASE("2022_day22:example") {
            std::vector<std::string> lines {
                "        ...#",
                "        .#..",
                "        #...",
                "        ....",
                "...#.......#",
                "........#...",
                "..#....#....",
                "..........#.",
                "        ...#....",
                "        .....#..",
                "        .#......",
                "        ......#.",
                "",
                "10R5L5R10L4R5L5"
            };
            const auto [map, path] = lines_to_input(lines);
            direction facing = direction::right;
            int start_y = static_cast<int>(std::distance(map.begin(), std::find_if_not(map.begin(), map.end(), [](char c){ return c == ' '; })));
            position pos {0, start_y};
            for (const auto& p : path) {
                std::visit(path_walker{map, facing, pos}, p);
            }
            CHECK_EQ(facing, direction::right);
            CHECK_EQ(pos.x + 1, 6);
            CHECK_EQ(pos.y + 1, 8);
        }
    }

}