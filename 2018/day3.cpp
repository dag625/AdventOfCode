//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2018/day/3
     */

    struct claim {
        int id = 0;
        position offset;
        position size;
    };

    claim parse_claim(std::string_view s) {
        const auto parts = split_by_all_no_empty(s, "# @,x:");
        return {parse32(parts[0]), {parse32(parts[1]), parse32(parts[2])}, {parse32(parts[3]), parse32(parts[4])}};
    }

    std::vector<claim> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2018" / "day_3_input.txt");
        return lines | std::views::transform(&parse_claim) | std::ranges::to<std::vector>();
    }

    std::pair<grid<int>, int> create_grid(const std::vector<claim>& claims) {
        std::size_t max_x = 0, max_y = 0;
        for (const auto& c : claims) {
            if (c.offset.x + c.size.x > max_x) {
                max_x = c.offset.x + c.size.x;
            }
            if (c.offset.y + c.size.y > max_y) {
                max_y = c.offset.y + c.size.y;
            }
        }
        grid<int> retval {max_x + 1, max_y + 1};
        std::vector<int> non_overlapping;
        for (const auto& c : claims) {
            bool overlapped = false;
            for (int x = 0; x < c.size.x; ++x) {
                for (int y = 0; y < c.size.y; ++y) {
                    int& val = retval[c.offset.x + x][c.offset.y + y];
                    if (val == 0) {
                        val = c.id;
                    }
                    else {
                        overlapped = true;
                        const auto found = std::find(non_overlapping.begin(), non_overlapping.end(), val);
                        if (found != non_overlapping.end()) {
                            non_overlapping.erase(found);
                        }
                        val = -1;
                    }
                }
            }
            if (!overlapped) {
                non_overlapping.push_back(c.id);
            }
        }
        return {std::move(retval), non_overlapping.front()};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto [g, id] = create_grid(input);
        const auto num_overlaps = std::count(g.begin(), g.end(), -1);
        return std::to_string(num_overlaps);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto [g, id] = create_grid(input);
        return std::to_string(id);
    }

    aoc::registration r{2018, 3, part_1, part_2};

//    TEST_SUITE("2018_day03") {
//        TEST_CASE("2018_day03:example") {
//
//        }
//    }

} /* namespace <anon> */