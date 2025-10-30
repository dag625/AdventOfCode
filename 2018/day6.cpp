//
// Created by Dan on 10/30/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "point.h"
#include "ranges.h"
#include "parse.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/6
     */

    struct coordinate {
        position pos;
        bool is_infinite = false;
        int area = 0;

        auto operator<=>(const coordinate& other) const { return pos <=> other.pos; }
        bool operator==(const coordinate& other) const { return pos == other.pos; }

        auto operator<=>(const position& other) const { return pos <=> other; }
        bool operator==(const position& other) const { return pos == other; }
    };

    coordinate parse_coord(std::string_view line)
    {
        const auto parts = split(line, ", ");
        return {parse<int>(parts[0]), parse<int>(parts[1])};
    }

    std::vector<coordinate> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_coord) | std::ranges::to<std::vector>();
    }

    struct bounding_box {
        int min_x = std::numeric_limits<int>::max();
        int min_y = std::numeric_limits<int>::max();
        int max_x = std::numeric_limits<int>::min();
        int max_y = std::numeric_limits<int>::min();

        [[nodiscard]] int area() const { return (max_x - min_x + 1) * (max_y - min_y + 1); }
    };

    int manhattan_distance(position a, position b) { return std::abs(a.x - b.x) + std::abs(a.y - b.y); }

    bounding_box get_bounding_box(const std::vector<coordinate>& points) {
        bounding_box retval{};
        for (const auto& p : points) {
            retval.min_x = std::min(retval.min_x, p.pos.x);
            retval.min_y = std::min(retval.min_y, p.pos.y);
            retval.max_x = std::max(retval.max_x, p.pos.x);
            retval.max_y = std::max(retval.max_y, p.pos.y);
        }
        return retval;
    }

    std::vector<position> get_box_edge_points(const bounding_box& b) {
        std::vector<position> retval;
        for (int x = b.min_x; x <= b.max_x; ++x) {
            retval.emplace_back(x, b.min_y);
            retval.emplace_back(x, b.max_y);
        }
        for (int y = b.min_y + 1; y < b.max_y; ++y) {
            retval.emplace_back(b.min_x, y);
            retval.emplace_back(b.max_x, y);
        }
        return retval;
    }

    void find_infinite(std::vector<coordinate>& coords, const std::vector<position>& borders) {
        std::vector<position> retval;

        for (const auto& b : borders) {
            const auto min = std::min_element(coords.begin(), coords.end(),
                [&b](const coordinate& p1, const coordinate& p2) { return manhattan_distance(p1.pos,b) < manhattan_distance(p2.pos,b); });
            min->is_infinite = true;
        }
    }

    std::vector<position> get_finite(const std::vector<position>& all, const std::vector<position>& infinite) {
        std::vector<position> retval;
        std::set_difference(all.begin(), all.end(), infinite.begin(), infinite.end(), std::back_inserter(retval));
        return retval;
    }

    void find_areas(const bounding_box& b, std::vector<coordinate>& coords) {
        for (int x = b.min_x; x <= b.max_x; ++x) {
            for (int y = b.min_y; y <= b.max_y; ++y) {
                const position bc {x, y};
                int min = std::numeric_limits<int>::max();
                int64_t min_idx = -1;
                for (const auto [idx, p] : coords | std::views::enumerate) {
                    const auto dist = manhattan_distance(bc, p.pos);
                    if (dist < min) {
                        min_idx = idx;
                        min = dist;
                    }
                }
                ++coords[min_idx].area;
            }
        }
    }

    int calculate_distances(const position& p, const std::vector<coordinate>& coords) {
        return std::accumulate(coords.begin(), coords.end(), 0,
            [&p](int current, const auto& c){ return current + manhattan_distance(p, c.pos); });
    }

    int find_region_size(const bounding_box& b, const std::vector<coordinate>& coords, const int max_dist) {
        int retval = 0;
        for (int x = b.min_x; x <= b.max_x; ++x) {
            for (int y = b.min_y; y <= b.max_y; ++y) {
                const position bc {x, y};
                const auto dist = calculate_distances(bc, coords);
                if (dist < max_dist) {
                    ++retval;
                }
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        const auto bb = get_bounding_box(input);
        const auto bb_edges = get_box_edge_points(bb);
        find_infinite(input, bb_edges);
        find_areas(bb, input);
        input.erase(std::remove_if(input.begin(), input.end(), [](const auto& c){ return c.is_infinite; }), input.end());
        const auto max = std::max_element(input.begin(), input.end(), [](const auto& c1, const auto& c2){ return c1.area < c2.area; });
        return std::to_string(max->area);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        constexpr int MAX_DIST = 10000;
        const auto input = get_input(lines);
        auto bb = get_bounding_box(input);
        const auto leeway = (MAX_DIST / static_cast<int>(input.size())) + 2;
        bb.min_x -= leeway;
        bb.min_y -= leeway;
        bb.max_x += leeway;
        bb.max_y += leeway;
        const auto region_size = find_region_size(bb, input, MAX_DIST);
        return std::to_string(region_size);
    }

    aoc::registration r {2018, 6, part_1, part_2};

//    TEST_SUITE("2018_day6") {
//        TEST_CASE("2018_day6:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */