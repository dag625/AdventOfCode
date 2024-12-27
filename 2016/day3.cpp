//
// Created by Dan on 11/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "point.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    position3d parse_point(std::string_view s) {
        const auto p = split_no_empty(s, ' ');
        return {parse<int>(p[0]), parse<int>(p[1]), parse<int>(p[2])};
    }

    std::vector<position3d> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_point) | to<std::vector<position3d>>();
    }

    bool is_possible(const position3d& p) {
        return p.x + p.y > p.z && p.z + p.x > p.y && p.y + p.z > p.x;
    }

    std::vector<position3d> tilt(const std::vector<position3d>& lines) {
        std::vector<position3d> tilted;
        const auto num = static_cast<int>(lines.size());
        tilted.reserve(num);
        for (int i = 0; i < num; i += 3) {
            tilted.emplace_back(lines[i].x, lines[i+1].x, lines[i+2].x);
            tilted.emplace_back(lines[i].y, lines[i+1].y, lines[i+2].y);
            tilted.emplace_back(lines[i].z, lines[i+1].z, lines[i+2].z);
        }
        return tilted;
    }

    /*
    --- Day 3: Squares With Three Sides ---
    Now that you can think clearly, you move deeper into the labyrinth of hallways and office furniture that makes up this part of Easter Bunny HQ. This must be a graphic design department; the walls are covered in specifications for triangles.

    Or are they?

    The design document gives the side lengths of each triangle it describes, but... 5 10 25? Some of these aren't triangles. You can't help but mark the impossible ones.

    In a valid triangle, the sum of any two sides must be larger than the remaining side. For example, the "triangle" given above is impossible, because 5 + 10 is not larger than 25.

    In your puzzle input, how many of the listed triangles are possible?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto num = std::count_if(input.begin(), input.end(), &is_possible);
        return std::to_string(num);
    }

    /*
    --- Part Two ---
    Now that you've helpfully marked up their design documents, it occurs to you that triangles are specified in groups of three vertically. Each set of three numbers in a column specifies a triangle. Rows are unrelated.

    For example, given the following specification, numbers with the same hundreds digit would be part of the same triangle:

    101 301 501
    102 302 502
    103 303 503
    201 401 601
    202 402 602
    203 403 603
    In your puzzle input, and instead reading by columns, how many of the listed triangles are possible?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = tilt(get_input(lines));
        const auto num = std::count_if(input.begin(), input.end(), &is_possible);
        return std::to_string(num);
    }

    aoc::registration r{2016, 3, part_1, part_2};

//    TEST_SUITE("2016_day3") {
//        TEST_CASE("2016_day3:example") {
//
//        }
//    }

}