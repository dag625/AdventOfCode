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

namespace fs = std::filesystem;

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

    std::pair<grid<char>, std::vector<path_el>> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_22_input.txt", false);
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

    /*
    --- Day 22: Monkey Map ---
    The monkeys take you on a surprisingly easy trail through the jungle. They're even going in roughly the right direction according to your handheld device's Grove Positioning System.

    As you walk, the monkeys explain that the grove is protected by a force field. To pass through the force field, you have to enter a password; doing so involves tracing a specific path on a strangely-shaped board.

    At least, you're pretty sure that's what you have to do; the elephants aren't exactly fluent in monkey.

    The monkeys give you notes that they took when they last saw the password entered (your puzzle input).

    For example:

            ...#
            .#..
            #...
            ....
    ...#.......#
    ........#...
    ..#....#....
    ..........#.
            ...#....
            .....#..
            .#......
            ......#.

    10R5L5R10L4R5L5
    The first half of the monkeys' notes is a map of the board. It is comprised of a set of open tiles (on which you can move, drawn .) and solid walls (tiles which you cannot enter, drawn #).

    The second half is a description of the path you must follow. It consists of alternating numbers and letters:

    A number indicates the number of tiles to move in the direction you are facing. If you run into a wall, you stop moving forward and continue with the next instruction.
    A letter indicates whether to turn 90 degrees clockwise (R) or counterclockwise (L). Turning happens in-place; it does not change your current tile.
    So, a path like 10R5 means "go forward 10 tiles, then turn clockwise 90 degrees, then go forward 5 tiles".

    You begin the path in the leftmost open tile of the top row of tiles. Initially, you are facing to the right (from the perspective of how the map is drawn).

    If a movement instruction would take you off of the map, you wrap around to the other side of the board. In other words, if your next tile is off of the board, you should instead look in the direction opposite of your current facing as far as you can until you find the opposite edge of the board, then reappear there.

    For example, if you are at A and facing to the right, the tile in front of you is marked B; if you are at C and facing down, the tile in front of you is marked D:

            ...#
            .#..
            #...
            ....
    ...#.D.....#
    ........#...
    B.#....#...A
    .....C....#.
            ...#....
            .....#..
            .#......
            ......#.
    It is possible for the next tile (after wrapping around) to be a wall; this still counts as there being a wall in front of you, and so movement stops before you actually wrap to the other side of the board.

    By drawing the last facing you had with an arrow on each tile you visit, the full path taken by the above example looks like this:

            >>v#
            .#v.
            #.v.
            ..v.
    ...#...v..v#
    >>>v...>#.>>
    ..#v...#....
    ...>>>>v..#.
            ...#....
            .....#..
            .#......
            ......#.
    To finish providing the password to this strange input device, you need to determine numbers for your final row, column, and facing as your final position appears from the perspective of the original map. Rows start from 1 at the top and count downward; columns start from 1 at the left and count rightward. (In the above example, row 1, column 1 refers to the empty space with no tile on it in the top-left corner.) Facing is 0 for right (>), 1 for down (v), 2 for left (<), and 3 for up (^). The final password is the sum of 1000 times the row, 4 times the column, and the facing.

    In the above example, the final row is 6, the final column is 8, and the final facing is 0. So, the final password is 1000 * 6 + 4 * 8 + 0: 6032.

    Follow the path given in the monkeys' notes. What is the final password?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto [map, path] = get_input(input_dir);
        direction facing = direction::right;
        int start_y = static_cast<int>(std::distance(map.begin(), std::find_if_not(map.begin(), map.end(), [](char c){ return c == ' '; })));
        position pos {0, start_y};
        for (const auto& p : path) {
            std::visit(path_walker{map, facing, pos}, p);
        }
        return std::to_string(1000 * (pos.x + 1) + 4 * (pos.y + 1) + static_cast<int>(facing));
    }

    /*
    --- Part Two ---
    As you reach the force field, you think you hear some Elves in the distance. Perhaps they've already arrived?

    You approach the strange input device, but it isn't quite what the monkeys drew in their notes. Instead, you are met with a large cube; each of its six faces is a square of 50x50 tiles.

    To be fair, the monkeys' map does have six 50x50 regions on it. If you were to carefully fold the map, you should be able to shape it into a cube!

    In the example above, the six (smaller, 4x4) faces of the cube are:

            1111
            1111
            1111
            1111
    222233334444
    222233334444
    222233334444
    222233334444
            55556666
            55556666
            55556666
            55556666
    You still start in the same position and with the same facing as before, but the wrapping rules are different. Now, if you would walk off the board, you instead proceed around the cube. From the perspective of the map, this can look a little strange. In the above example, if you are at A and move to the right, you would arrive at B facing down; if you are at C and move down, you would arrive at D facing up:

            ...#
            .#..
            #...
            ....
    ...#.......#
    ........#..A
    ..#....#....
    .D........#.
            ...#..B.
            .....#..
            .#......
            ..C...#.
    Walls still block your path, even if they are on a different face of the cube. If you are at E facing up, your movement is blocked by the wall marked by the arrow:

            ...#
            .#..
         -->#...
            ....
    ...#..E....#
    ........#...
    ..#....#....
    ..........#.
            ...#....
            .....#..
            .#......
            ......#.
    Using the same method of drawing the last facing you had with an arrow on each tile you visit, the full path taken by the above example now looks like this:

            >>v#
            .#v.
            #.v.
            ..v.
    ...#..^...v#
    .>>>>>^.#.>>
    .^#....#....
    .^........#.
            ...#..v.
            .....#v.
            .#v<<<<.
            ..v...#.
    The final password is still calculated from your final position and facing from the perspective of the map. In this example, the final row is 5, the final column is 7, and the final facing is 3, so the final password is 1000 * 5 + 4 * 7 + 3 = 5031.

    Fold the map into a cube, then follow the path given in the monkeys' notes. What is the final password?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto [map, path] = get_input(input_dir);
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