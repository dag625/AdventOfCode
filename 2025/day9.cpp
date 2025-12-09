//
// Created by Dan on 12/9/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "point.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2025/day/9
     */

    position parse_pos(std::string_view str) {
        const auto parts = split(str, ',');
        return {parse32(parts[0]), parse32(parts[1])};
    }

    std::vector<position> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_pos) | std::ranges::to<std::vector>();
    }

    struct line {
        int coord = 0;
        int start = 0;
        int finish = 0;
        position end1;
        position end2;

        [[nodiscard]] int size() const { return finish - start + 1; }

        auto operator<=>(const line& other) const noexcept = default;
    };

    int64_t calc_area(const position& p1, const position& p2) {
        const auto dx = static_cast<int64_t>(std::abs(p1.x - p2.x)) + 1;
        const auto dy = static_cast<int64_t>(std::abs(p1.y - p2.y)) + 1;
        const auto area = dx * dy;
        return area;
    }

    /**
     * Sanity check the input to make sure that every point is actually a corner.
     * @param corners List of points to check.
     */
    void verify_points(const std::vector<position>& corners) {
        int last_x = corners.back().x, last_y = corners.back().y;
        int x_streak = 0, y_streak = 0;
        for (const auto& p : corners) {
            if (p.x == last_x) {
                ++x_streak;
                if (x_streak > 1) {
                    throw std::logic_error{"Found three positions in a row with same x coordinate."};
                }
            }
            else {
                x_streak = 0;
            }
            if (p.y == last_y) {
                ++y_streak;
                if (y_streak > 1) {
                    throw std::logic_error{"Found three positions in a row with same y coordinate."};
                }
            }
            else {
                y_streak = 0;
            }
            last_x = p.x;
            last_y = p.y;
        }
    }

    std::pair<std::vector<line>, std::vector<line>> find_lines(const std::vector<position>& corners) {
        verify_points(corners);
        std::vector<line> const_x, const_y;

        //Note that the order of end1 and end2 should be the same as their order in the 'corners' list!

        if (corners.front().x == corners.back().x) {
            const_x.emplace_back(corners.front().x,
            std::min(corners.front().y, corners.back().y),
            std::max(corners.front().y, corners.back().y),
            corners.front(), corners.back());
        }
        else {
            const_y.emplace_back(corners.front().y,
            std::min(corners.front().x, corners.back().x),
            std::max(corners.front().x, corners.back().x),
            corners.front(), corners.back());
        }

        for (const auto& p : corners | std::views::slide(2)) {
            if (p.front().x == p.back().x) {
                const_x.emplace_back(p.front().x,
                std::min(p.front().y, p.back().y),
                std::max(p.front().y, p.back().y),
                p.front(), p.back());
            }
            else {
                const_y.emplace_back(p.front().y,
                std::min(p.front().x, p.back().x),
                std::max(p.front().x, p.back().x),
                p.front(), p.back());
            }
        }

        std::sort(const_x.begin(), const_x.end());
        std::sort(const_y.begin(), const_y.end());

        return {std::move(const_x), std::move(const_y)};
    }

    bool any_in(const std::vector<position>& corners, const position& c1, const position& c2, const bool top_is_1) {
        const bool is_1_rightward = c1.x > c2.x;
        for (const auto& p : corners) {
            if (p == c1 || p == c2) {
                continue;
            }
            const bool is_in_y = top_is_1 ? (p.y < c1.y && p.y > c2.y) : (p.y > c1.y && p.y < c2.y);
            const bool is_in_x = is_1_rightward ? (p.x < c1.x && p.x > c2.x) : (p.x > c1.x && p.x < c2.x);
            if (is_in_x && is_in_y) {
                return true;
            }
        }
        return false;
    }

    int64_t find_max_area(const std::vector<position>& corners, const position& anchor, const bool only_y_greater_than_anchor) {
        int64_t retval = 0;
        position max_p{};
        for (const auto& p : corners) {
            if ((only_y_greater_than_anchor && p.y > anchor.y) ||
                (!only_y_greater_than_anchor && p.y < anchor.y))
            {
                const auto area = calc_area(p, anchor);
                if (area > retval && !any_in(corners, anchor, p, !only_y_greater_than_anchor)) {
                    retval = area;
                    max_p = p;
                }
            }
        }
        return retval;
    }

    int64_t find_max_area(const std::vector<position>& corners, const position& anchor1, const position& anchor2) {
        //Because by inspection we know the anchors come from lines of constant Y.
        const bool only_y_greater_than_anchor = anchor1.y > anchor2.y;
        const auto a1 = find_max_area(corners, anchor1, only_y_greater_than_anchor);
        const auto a2 = find_max_area(corners, anchor2, !only_y_greater_than_anchor);
        return std::max(a1, a2);
    }

    void print_lines(const std::vector<line>& lines, const char coord_axis) {
        fmt::print("{} Lines:\n", coord_axis);
        for (const auto& p : lines) {
            fmt::print("\t{:8} - {}\n", p.coord, p.size());
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        //Do it the dumb way:
        int64_t max_area = 0;
        for (int i = 0; i < input.size(); ++i) {
            for (int j = i + 1; j < input.size(); ++j) {
                // const auto dx = static_cast<int64_t>(std::abs(input[i].x - input[j].x)) + 1;
                // const auto dy = static_cast<int64_t>(std::abs(input[i].y - input[j].y)) + 1;
                // const auto area = dx * dy;
                const auto area = calc_area(input[i], input[j]);
                if (area > max_area) {
                    max_area = area;
                }
            }
        }
        return std::to_string(max_area);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [x_lines, y_lines] = find_lines(input);
        /*
         * From inspecting input (and seeing other people's graphs online) the lines form
         * a round-ish shape with a single "cut" jutting into the middle, with two long lines,
         * which are themselves connected by a single short line in the other axis (e.g. if
         * the long lines are Y lines, then the "cap" of the "cut" points will share an X
         * coordinate and the points will be next to each other in the 'input' list;
         * theoretically they could share both X coordinates, but we are interested in the
         * neighboring coordinates).
         *
         * Making use of this knowledge, we say the rectangle must have one corner at one of the
         * corners of the "cut", and then find the other point can be any of the other points on
         * the same side of the "cut" that maximizes the area.
         *
         * From printing out the lines, the long "cut" lines are in 'y_lines'.
         */
        // print_lines(x_lines, 'X');
        // print_lines(y_lines, 'Y');

        constexpr int CUT_LEN_HEURISTIC = 10000;
        const auto cut1 = std::find_if(y_lines.cbegin(), y_lines.cend(), [](const line& l){ return l.size() > CUT_LEN_HEURISTIC; });
        const auto cut2 = std::find_if(cut1 + 1, y_lines.cend(), [](const line& l){ return l.size() > CUT_LEN_HEURISTIC; });

        const auto e11 = std::find(input.cbegin(), input.cend(), cut1->end1);
        const auto e12 = std::find(input.cbegin(), input.cend(), cut1->end2);
        const auto e21 = std::find(input.cbegin(), input.cend(), cut2->end1);
        const auto e22 = std::find(input.cbegin(), input.cend(), cut2->end2);
        const auto i11 = std::distance(input.cbegin(), e11);
        const auto i12 = std::distance(input.cbegin(), e12);
        const auto i21 = std::distance(input.cbegin(), e21);
        const auto i22 = std::distance(input.cbegin(), e22);

        int64_t result = 0;
        if (std::abs(std::distance(e11, e21)) == 1) {
            result = find_max_area(input, *e11, *e21);
        }
        else if (std::abs(std::distance(e11, e22)) == 1) {
            result = find_max_area(input, *e11, *e22);
        }
        else if (std::abs(std::distance(e12, e21)) == 1) {
            result = find_max_area(input, *e12, *e21);
        }
        else if (std::abs(std::distance(e12, e22)) == 1) {
            result = find_max_area(input, *e12, *e22);
        }

        return std::to_string(result);
    }

    aoc::registration r{2025, 9, part_1, part_2};

    //    TEST_SUITE("2025_day09") {
    //        TEST_CASE("2025_day09:example") {
    //            const std::vector<std::string> lines {
    //
    //            };
    //            const auto input = get_input(lines);
    //
    //        }
    //    }

} /* namespace <anon> */
