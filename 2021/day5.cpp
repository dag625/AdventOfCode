//
// Created by Dan on 12/4/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <string_view>
#include <ranges>
#include <regex>
#include <set>
#include <algorithm>

#include "utilities.h"
#include "point.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct line {
        position start;
        position stop;
    };

    struct point {
        position pos;
        mutable int count = 1;

        point(position p) : pos{std::move(p)} {}

        bool operator==(const point& rhs) const noexcept { return pos == rhs.pos; }
        bool operator< (const point& rhs) const noexcept { return pos <  rhs.pos; }
    };

    bool operator<(const point& lhs, const position& rhs) noexcept {
        return lhs.pos < rhs;
    }

    bool operator<(const position& lhs, const point& rhs) noexcept {
        return lhs < rhs.pos;
    }

    bool operator==(const point& lhs, const position& rhs) noexcept {
        return lhs.pos == rhs;
    }

    bool operator==(const position& lhs, const point& rhs) noexcept {
        return lhs == rhs.pos;
    }

    velocity get_line_unit_vel(const line& l) {
        velocity retval = to_vel(l.stop - l.start);
        if (retval.dx) {
            retval.dx = retval.dx > 0 ? 1 : -1;
        }
        if (retval.dy) {
            retval.dy = retval.dy > 0 ? 1 : -1;
        }
        return retval;
    }

    std::vector<line> horizontal_and_vertical_only(std::vector<line> input) {
        input.erase(std::remove_if(input.begin(), input.end(), [](const line& l){
            return l.start.x != l.stop.x && l.start.y != l.stop.y;
        }), input.end());
        return input;
    }

    std::vector<line> get_input(const std::vector<std::string>& lines) {
        const std::regex re {R"((\d+),(\d+)\s*->\s*(\d+),(\d+))"};
        std::vector<line> retval;
        retval.reserve(lines.size());
        std::ranges::transform(lines, std::back_inserter(retval), [&re](const std::string& s) -> line {
            std::smatch results;
            if (!std::regex_match(s, results, re)) {
                throw std::runtime_error{"Line does not have expected format."};
            }
            return {{std::stoi(results[1].str()), std::stoi(results[2].str())}, {std::stoi(results[3].str()), std::stoi(results[4].str())}};
        });
        return retval;
    }

    int64_t find_multi_points(const std::vector<line>& lines) {
        std::vector<point> points;
        points.reserve(200000);
        for (const auto& l : lines) {
            const auto vel = get_line_unit_vel(l);
            for (auto pos = l.start; pos != l.stop + vel; pos += vel) {
                const auto found = std::lower_bound(points.begin(), points.end(), pos);
                if (found == points.end() || found->pos != pos) {
                    points.emplace(found, pos);
                }
                else {
                    ++found->count;
                }
            }
        }
        return std::ranges::count_if(points, [](const point& p){ return p.count > 1; });
    }

    int64_t find_multi_points_set(const std::vector<line>& lines) {
        std::set<point> points;
        for (const auto& l : lines) {
            const auto vel = get_line_unit_vel(l);
            for (auto pos = l.start; pos != l.stop + vel; pos += vel) {
                const auto found = points.find(pos);
                if (found == points.end()) {
                    points.emplace(pos);
                }
                else {
                    ++found->count;
                }
            }
        }
        return std::ranges::count_if(points, [](const point& p){ return p.count > 1; });
    }

    /*
    --- Day 5: Hydrothermal Venture ---
    You come across a field of hydrothermal vents on the ocean floor! These vents constantly produce large, opaque clouds, so it would be best to avoid them if possible.

    They tend to form in lines; the submarine helpfully produces a list of nearby lines of vents (your puzzle input) for you to review. For example:

    0,9 -> 5,9
    8,0 -> 0,8
    9,4 -> 3,4
    2,2 -> 2,1
    7,0 -> 7,4
    6,4 -> 2,0
    0,9 -> 2,9
    3,4 -> 1,4
    0,0 -> 8,8
    5,5 -> 8,2
    Each line of vents is given as a line segment in the format x1,y1 -> x2,y2 where x1,y1 are the coordinates of one end the line segment and x2,y2 are the coordinates of the other end. These line segments include the points at both ends. In other words:

    An entry like 1,1 -> 1,3 covers points 1,1, 1,2, and 1,3.
    An entry like 9,7 -> 7,7 covers points 9,7, 8,7, and 7,7.
    For now, only consider horizontal and vertical lines: lines where either x1 = x2 or y1 = y2.

    So, the horizontal and vertical lines from the above list would produce the following diagram:

    .......1..
    ..1....1..
    ..1....1..
    .......1..
    .112111211
    ..........
    ..........
    ..........
    ..........
    222111....
    In this diagram, the top left corner is 0,0 and the bottom right corner is 9,9. Each position is shown as the number of lines which cover that point or . if no line covers that point. The top-left pair of 1s, for example, comes from 2,2 -> 2,1; the very bottom row is formed by the overlapping lines 0,9 -> 5,9 and 0,9 -> 2,9.

    To avoid the most dangerous areas, you need to determine the number of points where at least two lines overlap. In the above example, this is anywhere in the diagram with a 2 or larger - a total of 5 points.

    Consider only horizontal and vertical lines. At how many points do at least two lines overlap?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        return std::to_string(find_multi_points_set(horizontal_and_vertical_only(get_input(lines))));
    }

    /*
    --- Part Two ---
    Unfortunately, considering only horizontal and vertical lines doesn't give you the full picture; you need to also consider diagonal lines.

    Because of the limits of the hydrothermal vent mapping system, the lines in your list will only ever be horizontal, vertical, or a diagonal line at exactly 45 degrees. In other words:

    An entry like 1,1 -> 3,3 covers points 1,1, 2,2, and 3,3.
    An entry like 9,7 -> 7,9 covers points 9,7, 8,8, and 7,9.
    Considering all lines from the above example would now produce the following diagram:

    1.1....11.
    .111...2..
    ..2.1.111.
    ...1.2.2..
    .112313211
    ...1.2....
    ..1...1...
    .1.....1..
    1.......1.
    222111....
    You still need to determine the number of points where at least two lines overlap. In the above example, this is still anywhere in the diagram with a 2 or larger - now a total of 12 points.

    Consider all of the lines. At how many points do at least two lines overlap?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        return std::to_string(find_multi_points_set(get_input(lines)));
    }

    aoc::registration r {2021, 5, part_1, part_2};

//    TEST_SUITE("2021_day5") {
//        TEST_CASE("2021_day5:example") {
//            std::vector<line> lines = {
//                    {0,9}, {5,9},
//                    {8,0}, {0,8},
//                    {9,4}, {3,4},
//                    {2,2}, {2,1},
//                    {7,0}, {7,4},
//                    {6,4}, {2,0},
//                    {0,9}, {2,9},
//                    {3,4}, {1,4},
//                    {0,0}, {8,8},
//                    {5,5}, {8,2}
//            };
//            lines = horizontal_and_vertical_only(std::move(lines));
//
//        }
//    }

}
