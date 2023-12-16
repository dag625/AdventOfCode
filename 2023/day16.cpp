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

    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto tiles = to_tiles(input);
        beam_it(tiles, {{0, -1}, {0, 1}});
        const auto num = std::count_if(tiles.begin(), tiles.end(), [](const tile& t){ return t.energized; });
        return std::to_string(num);//2067
    }

    /*

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