//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/3
     */

    int get_input(const std::vector<std::string>& lines) {
        return parse<int>(lines.front());
    }

    int get_ring(const int num) {
        return (static_cast<int>(std::ceil(std::sqrt(num)))) / 2;
    }

    std::pair<int, int> ring_start_end(const int ring) noexcept {
        if (ring == 0) { return {1, 1}; }
        const int a = (2 * ring + 1), b = (2 * ring - 1);
        return {b * b + 1, a * a};
    }

    std::pair<int, int> ring_and_dist_to_cardinal(int val) {
        const auto ring = get_ring(val);
        if (ring == 0) {
            return {ring, 0};
        }
        const auto [rstart, rend] = ring_start_end(ring);
        const auto dist_from_end = rend - val;
        const auto ring_side_len = 2 * ring;
        const auto dist_from_cross = std::abs((dist_from_end % ring_side_len) - ring);
        return {ring, dist_from_cross};
    }

    position to_spiral_pos(int idx) {
        const auto ring = get_ring(idx);
        if (ring == 0) {
            return {ring, 0};
        }
        const auto [rstart, rend] = ring_start_end(ring);
        const auto ring_side_len = 2 * ring;
        const auto dist_from_end = rend - idx;
        const auto dist_from_cross = (dist_from_end % ring_side_len) - ring;
        const auto cross_num = dist_from_end / ring_side_len;
        if (cross_num == 0) { //on bottom
            return {-dist_from_cross, -ring};
        }
        else if (cross_num == 1) {//on left
            return {-ring, dist_from_cross};
        }
        else if (cross_num == 2) {//on top
            return {dist_from_cross, ring};
        }
        else {//on right
            return {ring, -dist_from_cross};
        }
    }

    position to_grid_pos(const int rings, position p) {
        return {p.x + rings, p.y + rings};
    }

    int set_grid_val(grid<int>& g, const int idx) {
        const auto spiral_pos = to_spiral_pos(idx);
        const auto pos = to_grid_pos(g.num_rows() / 2, spiral_pos);
        const auto neighbors = g.neighbors(pos);
        int val = 0;
        for (const auto np : neighbors) {
            const auto nval = g[np];
            val += nval;
        }
        g[pos] = val;
        return val;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [ring, xdist] = ring_and_dist_to_cardinal(input);
        return std::to_string(ring + xdist);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [ring, xdist] = ring_and_dist_to_cardinal(input);
        const auto side_len = 2 * static_cast<std::size_t>(ring) + 1;
        grid<int> g {side_len, side_len};
        int last_val = 1;
        g[to_grid_pos(ring, {0, 0})] = last_val;
        for (int i = 2; i < input; ++i) {
            last_val = set_grid_val(g, i);
            if (last_val > input) {
                break;
            }
        }
        return std::to_string(last_val);
    }

    aoc::registration r{2017, 3, part_1, part_2};

//    TEST_SUITE("2017_day03") {
//        TEST_CASE("2017_day03:example") {
//
//        }
//    }

} /* namespace <anon> */