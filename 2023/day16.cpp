//
// Created by Dan on 12/15/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <deque>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct tile {
        char val;
        bool energized = false;
        std::vector<velocity> froms;

        tile(char c) : val{c} {}
    };

    std::ostream& operator<<(std::ostream& os, const tile& t) {
        os << (t.energized ? '#' : t.val);
        return os;
    }

    grid<tile> to_tiles(const grid<char>& cg) {
        std::vector<tile> data;
        data.reserve(cg.size());
        for (const char c : cg) {
            data.emplace_back(c);
        }
        return {std::move(data), cg.num_cols()};
    }

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_16_input.txt");
        const auto row_len = lines.front().size();
        return {lines | std::views::join | to<std::vector<char>>(), row_len};
    }

    struct beam {
        position pos;
        velocity vel;
    };

    void do_mirror(grid<tile>& g, std::deque<beam>& remaining, position pos, velocity in_vel) {
        auto& current = g[pos];
        const auto found = std::find(current.froms.begin(), current.froms.end(), in_vel);
        if (found != current.froms.end()) {
            return;
        }
        current.froms.push_back(in_vel);
        if (current.val == '/') {
            if (in_vel == velocity{0, 1}) {
                remaining.push_back({pos, velocity{-1, 0}});
            }
            else if (in_vel == velocity{0, -1}) {
                remaining.push_back({pos, velocity{1, 0}});
            }
            else if (in_vel == velocity{1, 0}) {
                remaining.push_back({pos, velocity{0, -1}});
            }
            else if (in_vel == velocity{-1, 0}) {
                remaining.push_back({pos, velocity{0, 1}});
            }
        }
        else { // val == '\'
            if (in_vel == velocity{0, 1}) {
                remaining.push_back({pos, velocity{1, 0}});
            }
            else if (in_vel == velocity{0, -1}) {
                remaining.push_back({pos, velocity{-1, 0}});
            }
            else if (in_vel == velocity{1, 0}) {
                remaining.push_back({pos, velocity{0, 1}});
            }
            else if (in_vel == velocity{-1, 0}) {
                remaining.push_back({pos, velocity{0, -1}});
            }
        }
    }

    void do_split(grid<tile>& g, std::deque<beam>& remaining, position pos, velocity in_vel) {
        auto& current = g[pos];
        const auto found = std::find(current.froms.begin(), current.froms.end(), in_vel);
        if (found != current.froms.end()) {
            return;
        }
        current.froms.push_back(in_vel);
        if (current.val == '-') {
            if (in_vel == velocity{1, 0}) {
                remaining.push_back({pos, velocity{0, -1}});
                remaining.push_back({pos, velocity{0, 1}});
            }
            else if (in_vel == velocity{-1, 0}) {
                remaining.push_back({pos, velocity{0, -1}});
                remaining.push_back({pos, velocity{0, 1}});
            }
            else {
                remaining.push_back({pos, in_vel});
            }
        }
        else { // val == '|'
            if (in_vel == velocity{0, 1}) {
                remaining.push_back({pos, velocity{-1, 0}});
                remaining.push_back({pos, velocity{1, 0}});
            }
            else if (in_vel == velocity{0, -1}) {
                remaining.push_back({pos, velocity{-1, 0}});
                remaining.push_back({pos, velocity{1, 0}});
            }
            else {
                remaining.push_back({pos, in_vel});
            }
        }
    }

    void beam_it(grid<tile>& g, beam b) {
        std::deque<beam> remaining;
        remaining.push_back(b);
        while (!remaining.empty()) {
            const auto current = remaining.front();
            remaining.pop_front();

            const auto next_pos = current.pos + current.vel;
            if (g.in(next_pos)) {
                if (g[next_pos].val == '/' || g[next_pos].val == '\\') {
                    do_mirror(g, remaining, next_pos, current.vel);
                }
                else if (g[next_pos].val == '-' || g[next_pos].val == '|') {
                    do_split(g, remaining, next_pos, current.vel);
                }
                else {
                    remaining.push_back({next_pos, current.vel});
                }
                g[next_pos].energized = true;
            }
        }
    }

    /*
    --- Day 16: The Floor Will Be Lava ---
    With the beam of light completely focused somewhere, the reindeer leads you deeper still into the Lava Production Facility. At some point, you realize that the steel facility walls have been replaced with cave, and the doorways are just cave, and the floor is cave, and you're pretty sure this is actually just a giant cave.

    Finally, as you approach what must be the heart of the mountain, you see a bright light in a cavern up ahead. There, you discover that the beam of light you so carefully focused is emerging from the cavern wall closest to the facility and pouring all of its energy into a contraption on the opposite side.

    Upon closer inspection, the contraption appears to be a flat, two-dimensional square grid containing empty space (.), mirrors (/ and \), and splitters (| and -).

    The contraption is aligned so that most of the beam bounces around the grid, but each tile on the grid converts some of the beam's light into heat to melt the rock in the cavern.

    You note the layout of the contraption (your puzzle input). For example:

    .|...\....
    |.-.\.....
    .....|-...
    ........|.
    ..........
    .........\
    ..../.\\..
    .-.-/..|..
    .|....-|.\
    ..//.|....
    The beam enters in the top-left corner from the left and heading to the right. Then, its behavior depends on what it encounters as it moves:

    If the beam encounters empty space (.), it continues in the same direction.
    If the beam encounters a mirror (/ or \), the beam is reflected 90 degrees depending on the angle of the mirror. For instance, a rightward-moving beam that encounters a / mirror would continue upward in the mirror's column, while a rightward-moving beam that encounters a \ mirror would continue downward from the mirror's column.
    If the beam encounters the pointy end of a splitter (| or -), the beam passes through the splitter as if the splitter were empty space. For instance, a rightward-moving beam that encounters a - splitter would continue in the same direction.
    If the beam encounters the flat side of a splitter (| or -), the beam is split into two beams going in each of the two directions the splitter's pointy ends are pointing. For instance, a rightward-moving beam that encounters a | splitter would split into two beams: one that continues upward from the splitter's column and one that continues downward from the splitter's column.
    Beams do not interact with other beams; a tile can have many beams passing through it at the same time. A tile is energized if that tile has at least one beam pass through it, reflect in it, or split in it.

    In the above example, here is how the beam of light bounces around the contraption:

    >|<<<\....
    |v-.\^....
    .v...|->>>
    .v...v^.|.
    .v...v^...
    .v...v^..\
    .v../2\\..
    <->-/vv|..
    .|<<<2-|.\
    .v//.|.v..
    Beams are only shown on empty tiles; arrows indicate the direction of the beams. If a tile contains beams moving in multiple directions, the number of distinct directions is shown instead. Here is the same diagram but instead only showing whether a tile is energized (#) or not (.):

    ######....
    .#...#....
    .#...#####
    .#...##...
    .#...##...
    .#...##...
    .#..####..
    ########..
    .#######..
    .#...#.#..
    Ultimately, in this example, 46 tiles become energized.

    The light isn't energizing enough tiles to produce lava; to debug the contraption, you need to start by analyzing the current situation. With the beam starting in the top-left heading right, how many tiles end up being energized?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto tiles = to_tiles(input);
        beam_it(tiles, {{0, -1}, {0, 1}});
        const auto num = std::count_if(tiles.begin(), tiles.end(), [](const tile& t){ return t.energized; });
        return std::to_string(num);//2067
    }

    /*
    --- Part Two ---
    As you try to work out what might be wrong, the reindeer tugs on your shirt and leads you to a nearby control panel. There, a collection of buttons lets you align the contraption so that the beam enters from any edge tile and heading away from that edge. (You can choose either of two directions for the beam if it starts on a corner; for instance, if the beam starts in the bottom-right corner, it can start heading either left or upward.)

    So, the beam could start on any tile in the top row (heading downward), any tile in the bottom row (heading upward), any tile in the leftmost column (heading right), or any tile in the rightmost column (heading left). To produce lava, you need to find the configuration that energizes as many tiles as possible.

    In the above example, this can be achieved by starting the beam in the fourth tile from the left in the top row:

    .|<2<\....
    |v-v\^....
    .v.v.|->>>
    .v.v.v^.|.
    .v.v.v^...
    .v.v.v^..\
    .v.v/2\\..
    <-2-/vv|..
    .|<<<2-|.\
    .v//.|.v..
    Using this configuration, 51 tiles are energized:

    .#####....
    .#.#.#....
    .#.#.#####
    .#.#.##...
    .#.#.##...
    .#.#.##...
    .#.#####..
    ########..
    .#######..
    .#...#.#..
    Find the initial beam configuration that energizes the largest number of tiles; how many tiles are energized in that configuration?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto tiles = to_tiles(input);
        std::size_t max = 0;
        const int num_rows = static_cast<int>(tiles.num_rows()), num_cols = static_cast<int>(tiles.num_cols());
        const auto count_pred = [](const tile& t){ return t.energized; };
        for (int r = 0; r < num_rows; ++r) {
            const beam left {{r, -1}, {0, 1}}, right {{r, num_cols}, {0, -1}};
            auto lt = tiles, rt = tiles;
            beam_it(lt, left);
            beam_it(rt, right);
            if (const auto v = std::count_if(lt.begin(), lt.end(), count_pred); v > max) {
                max = v;
            }
            if (const auto v = std::count_if(rt.begin(), rt.end(), count_pred); v > max) {
                max = v;
            }
        }
        for (int c = 0; c < num_cols; ++c) {
            const beam top {{-1, c}, {1, 0}}, bottom {{num_rows, c}, {-1, 0}};
            auto tt = tiles, bt = tiles;
            beam_it(tt, top);
            beam_it(bt, bottom);
            if (const auto v = std::count_if(tt.begin(), tt.end(), count_pred); v > max) {
                max = v;
            }
            if (const auto v = std::count_if(bt.begin(), bt.end(), count_pred); v > max) {
                max = v;
            }
        }
        return std::to_string(max);
    }

    aoc::registration r{2023, 16, part_1, part_2};


    TEST_SUITE("2023_day16") {
        TEST_CASE("2023_day16:example") {
            std::string data = ".|...\\...."
                               "|.-.\\....."
                               ".....|-..."
                               "........|."
                               ".........."
                               ".........\\"
                               "..../.\\\\.."
                               ".-.-/..|.."
                               ".|....-|.\\"
                               "..//.|....";
            const int row_len = 10;
            const grid<char> input {std::vector<char>{data.begin(), data.end()}, row_len};
            auto tiles = to_tiles(input);
            beam_it(tiles, {{0, -1}, {0, 1}});
            const auto num = std::count_if(tiles.begin(), tiles.end(), [](const tile& t){ return t.energized; });
            CHECK_EQ(num, 46);
        }
    }

}