//
// Created by Dan on 12/17/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>
#include <array>
#include <optional>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    enum class direction : uint32_t {
        up = 3,
        down = 1,
        left = 2,
        right = 0
    };

    struct instruction {
        direction dir = direction::up;
        velocity dv {};
        int64_t len = 0;
        uint32_t color = 0;
    };

    instruction parse_ins(std::string_view s) {
        const auto parts = split(s, ' ');
        instruction retval{};
        switch (parts[0].front()) {
            case 'U': retval.dir = direction::up; retval.dv = velocity{-1, 0}; break;
            case 'D': retval.dir = direction::down; retval.dv = velocity{1, 0}; break;
            case 'L': retval.dir = direction::left; retval.dv = velocity{0, -1}; break;
            case 'R': retval.dir = direction::right; retval.dv = velocity{0, 1}; break;
            default: break;
        }
        retval.len = parse<int64_t>(parts[1]);
        auto color = parts[2];
        color.remove_prefix(2);
        color.remove_suffix(1);
        retval.color = parse<uint32_t>(color, 16);
        return retval;
    }

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_ins) | to<std::vector<instruction>>();
    }

    instruction correct(instruction ins) {
        ins.dir = static_cast<direction>(ins.color & 0x0fu);
        ins.len = static_cast<int64_t>(ins.color >> 4);
        switch (ins.dir) {
            case direction::up: ins.dv = velocity{-1, 0}; break;
            case direction::down: ins.dv = velocity{1, 0}; break;
            case direction::left: ins.dv = velocity{0, -1}; break;
            case direction::right: ins.dv = velocity{0, 1}; break;
            default: break;
        }
        return ins;
    }

    struct line {
        int64_t col = 0;
        int64_t start_row = 0;
        int64_t stop_row = 0;

        [[nodiscard]] bool contains(const int64_t row) const {
            return row >= start_row && row <= stop_row;
        }

        [[nodiscard]] bool operator<(const line& rhs) const {
            //Order by row started in, then by column started in
            return std::tie(start_row, col) < std::tie(rhs.start_row, rhs.col);
        }
    };

    std::vector<line> dig_smarter(const std::vector<instruction>& instructions) {
        std::vector<line> retval;
        position current {};
        for (const auto& ins : instructions) {
            if (ins.dir == direction::up) {
                retval.emplace_back(current.y, current.x - ins.len, current.x);
            }
            else if (ins.dir == direction::down) {
                retval.emplace_back(current.y, current.x, current.x + ins.len);
            }
//            else if (ins.dir == direction::left) {
//                retval.emplace_back(true, current.x, current.y - ins.len, current.y);
//            }
//            else if (ins.dir == direction::right) {
//                retval.emplace_back(true, current.x, current.y, current.y + ins.len);
//            }
            current += ins.dv * static_cast<int>(ins.len);
        }
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    struct col_info {
        int64_t num = 0;
        bool is_corner = false;
        bool is_start = false;

        [[nodiscard]] bool operator<(const col_info& rhs) const { return num < rhs.num; }
    };

    std::vector<col_info> get_columns(const std::vector<line>& lines, int64_t row) {
        auto item = std::find_if(lines.begin(), lines.end(), [row](const line& l){ return l.contains(row); });
        std::vector<col_info> cols;
        while (item != lines.end()) {
            if (item->contains(row)) {
                const bool start_corner = item->start_row == row, stop_corner = item->stop_row == row;
                cols.emplace_back(item->col, start_corner || stop_corner, start_corner);
            }
            ++item;
        }
        std::sort(cols.begin(), cols.end());
        return cols;
    }

    int64_t cells_in_row(const std::vector<line>& lines, int64_t row) {
        const auto cols = get_columns(lines, row);

        bool inside = false;
        int64_t last_col = 0;
        int64_t retval = 0;
        std::optional<bool> ending_corner;
        for (const auto& col : cols) {
            if (inside) {
                if (col.is_corner) {
                    if (ending_corner && *ending_corner == col.is_start) {
                        retval += col.num - last_col + 1;
                        inside = false;
                    }
                    else if (ending_corner) {
                        ending_corner = std::nullopt;
                    }
                    else {
                        ending_corner = !col.is_start;
                    }
                }
                else {
                    retval += col.num - last_col + 1;
                    inside = false;
                }
            }
            else {
                last_col = col.num;
                inside = true;
                if (col.is_corner) {
                    ending_corner = col.is_start;
                }
                else {
                    ending_corner = std::nullopt;
                }
            }
        }
        return retval;
    }

    int64_t cells_inside(const std::vector<line>& lines) {
        int64_t retval = 0;
        std::vector<int64_t> counts;
        std::vector<int64_t> boundaries;
        boundaries.reserve(lines.size() * 2);
        for (const auto& l : lines) {
            boundaries.push_back(l.start_row);
            boundaries.push_back(l.stop_row);
        }
        std::sort(boundaries.begin(), boundaries.end());
        boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());
        for (const auto [start, stop] : boundaries | std::views::adjacent<2>) {
            const auto first_row_count = cells_in_row(lines, start);
            const auto num_rows = stop - start - 1;
            int64_t row_count = 0;
            if (num_rows > 0) {
                row_count = cells_in_row(lines, start + 1);
            }

            counts.push_back(first_row_count);
            counts.insert(counts.end(), num_rows, row_count);
            retval += first_row_count + row_count * num_rows;
        }
        const auto last = cells_in_row(lines, boundaries.back());
        counts.push_back(last);
        retval += last;
        return retval;
    }


    /*
    --- Day 18: Lavaduct Lagoon ---
    Thanks to your efforts, the machine parts factory is one of the first factories up and running since the lavafall came back. However, to catch up with the large backlog of parts requests, the factory will also need a large supply of lava for a while; the Elves have already started creating a large lagoon nearby for this purpose.

    However, they aren't sure the lagoon will be big enough; they've asked you to take a look at the dig plan (your puzzle input). For example:

    R 6 (#70c710)
    D 5 (#0dc571)
    L 2 (#5713f0)
    D 2 (#d2c081)
    R 2 (#59c680)
    D 2 (#411b91)
    L 5 (#8ceee2)
    U 2 (#caa173)
    L 1 (#1b58a2)
    U 2 (#caa171)
    R 2 (#7807d2)
    U 3 (#a77fa3)
    L 2 (#015232)
    U 2 (#7a21e3)
    The digger starts in a 1 meter cube hole in the ground. They then dig the specified number of meters up (U), down (D), left (L), or right (R), clearing full 1 meter cubes as they go. The directions are given as seen from above, so if "up" were north, then "right" would be east, and so on. Each trench is also listed with the color that the edge of the trench should be painted as an RGB hexadecimal color code.

    When viewed from above, the above example dig plan would result in the following loop of trench (#) having been dug out from otherwise ground-level terrain (.):

    #######
    #.....#
    ###...#
    ..#...#
    ..#...#
    ###.###
    #...#..
    ##..###
    .#....#
    .######
    At this point, the trench could contain 38 cubic meters of lava. However, this is just the edge of the lagoon; the next step is to dig out the interior so that it is one meter deep as well:

    #######
    #######
    #######
    ..#####
    ..#####
    #######
    #####..
    #######
    .######
    .######
    Now, the lagoon can contain a much more respectable 62 cubic meters of lava. While the interior is dug out, the edges are also painted according to the color codes in the dig plan.

    The Elves are concerned the lagoon won't be large enough; if they follow their dig plan, how many cubic meters of lava could it hold?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto line_data = dig_smarter(input);
        const auto num = cells_inside(line_data);
        return std::to_string(num);
    }

    /*
    --- Part Two ---
    The Elves were right to be concerned; the planned lagoon would be much too small.

    After a few minutes, someone realizes what happened; someone swapped the color and instruction parameters when producing the dig plan. They don't have time to fix the bug; one of them asks if you can extract the correct instructions from the hexadecimal codes.

    Each hexadecimal code is six hexadecimal digits long. The first five hexadecimal digits encode the distance in meters as a five-digit hexadecimal number. The last hexadecimal digit encodes the direction to dig: 0 means R, 1 means D, 2 means L, and 3 means U.

    So, in the above example, the hexadecimal codes can be converted into the true instructions:

    #70c710 = R 461937
    #0dc571 = D 56407
    #5713f0 = R 356671
    #d2c081 = D 863240
    #59c680 = R 367720
    #411b91 = D 266681
    #8ceee2 = L 577262
    #caa173 = U 829975
    #1b58a2 = L 112010
    #caa171 = D 829975
    #7807d2 = L 491645
    #a77fa3 = U 686074
    #015232 = L 5411
    #7a21e3 = U 500254
    Digging out this loop and its interior produces a lagoon that can hold an impressive 952408144115 cubic meters of lava.

    Convert the hexadecimal color codes into the correct instructions; if the Elves follow this new dig plan, how many cubic meters of lava could the lagoon hold?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto corrected = input | std::views::transform(&correct) | to<std::vector<instruction>>();
        const auto line_data = dig_smarter(corrected);
        const auto num = cells_inside(line_data);
        return std::to_string(num);
    }

    aoc::registration r{2023, 18, part_1, part_2};

    TEST_SUITE("2023_day18") {
        TEST_CASE("2023_day18:example") {
            using namespace std::string_literals;
            const std::vector<std::string> lines {
                    "R 6 (#70c710)"s,
                    "D 5 (#0dc571)"s,
                    "L 2 (#5713f0)"s,
                    "D 2 (#d2c081)"s,
                    "R 2 (#59c680)"s,
                    "D 2 (#411b91)"s,
                    "L 5 (#8ceee2)"s,
                    "U 2 (#caa173)"s,
                    "L 1 (#1b58a2)"s,
                    "U 2 (#caa171)"s,
                    "R 2 (#7807d2)"s,
                    "U 3 (#a77fa3)"s,
                    "L 2 (#015232)"s,
                    "U 2 (#7a21e3)"s
            };
            const auto input = lines | std::views::transform(&parse_ins) | to<std::vector<instruction>>();
            const auto p1_data = dig_smarter(input);
            const auto p1 = cells_inside(p1_data);
            CHECK_EQ(p1, 62);

            const auto corrected = input | std::views::transform(&correct) | to<std::vector<instruction>>();
            const auto p2_data = dig_smarter(corrected);
            const auto p2 = cells_inside(p2_data);
            CHECK_EQ(p2, 952408144115ll);
        }
    }

}