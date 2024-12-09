//
// Created by Dan on 12/9/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "point.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/11
     */

    /*
     * HEX_CUBE_DIRECTIONS indexes:
     * 0 - n
     * 1 - nw
     * 2 - sw
     * 3 - s
     * 4 - se
     * 5 - ne
     */

    std::vector<velocity3d> parse_dirs(const std::string& s) {
        using namespace std::string_view_literals;
        const auto dirs = split(s, ',');
        std::vector<velocity3d> retval;
        retval.reserve(dirs.size());
        for (const auto d : dirs) {
            if (d == "n"sv) {
                retval.push_back(HEX_CUBE_DIRECTIONS[0]);
            }
            else if (d == "nw"sv) {
                retval.push_back(HEX_CUBE_DIRECTIONS[1]);
            }
            else if (d == "sw"sv) {
                retval.push_back(HEX_CUBE_DIRECTIONS[2]);
            }
            else if (d == "s"sv) {
                retval.push_back(HEX_CUBE_DIRECTIONS[3]);
            }
            else if (d == "se"sv) {
                retval.push_back(HEX_CUBE_DIRECTIONS[4]);
            }
            else if (d == "ne"sv) {
                retval.push_back(HEX_CUBE_DIRECTIONS[5]);
            }
        }
        return retval;
    }

    std::vector<velocity3d> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_11_input.txt");
        return parse_dirs(lines.front());
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        position3d pos {};
        for (const auto& dir : input) {
            pos += dir;
        }
        return std::to_string(hex_dist_from_origin(pos));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        int max_dist = 0;
        position3d pos {};
        for (const auto& dir : input) {
            pos += dir;
            const auto dist = hex_dist_from_origin(pos);
            if (dist > max_dist) {
                max_dist = dist;
            }
        }
        return std::to_string(max_dist);
    }

    aoc::registration r{2017, 11, part_1, part_2};

//    TEST_SUITE("2017_day11") {
//        TEST_CASE("2017_day11:example") {
//
//        }
//    }

} /* namespace <anon> */