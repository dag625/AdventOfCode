//
// Created by Dan on 11/2/2025.
//

#include <iostream>
#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "point.h"
#include "parse.h"
#include "grid.h"
#include "letters.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/19
     */

    struct star {
        position pos;
        velocity vel;
    };

    star parse_star(std::string_view line) {
        const auto parts = split_by_all_no_empty(line, " <>,");
        return{{parse32(parts[1]), parse32(parts[2])}, {parse32(parts[4]), parse32(parts[5])}};
    }

    std::vector<star> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_star) | std::ranges::to<std::vector>();
    }

    std::tuple<int, int, int, int> step(std::vector<star>& stars) {
        int min_x = std::numeric_limits<int>::max(),
            max_x = std::numeric_limits<int>::min(),
            min_y = std::numeric_limits<int>::max(),
            max_y = std::numeric_limits<int>::min();
        for (auto& s : stars) {
            s.pos += s.vel;
            if (s.pos.x < min_x) { min_x = s.pos.x; }
            else if (s.pos.x > max_x) { max_x = s.pos.x; }
            if (s.pos.y < min_y) { min_y = s.pos.y; }
            else if (s.pos.y > max_y) { max_y = s.pos.y; }
        }
        return std::make_tuple(min_x, max_x, min_y, max_y);
    }

    void step_back(std::vector<star>& stars) {
        for (auto& s : stars) {
            s.pos -= s.vel;
        }
    }

    int64_t variance(const std::vector<star>& stars) {
        //Assume message around 0,0:
        const auto avg_x = std::accumulate(stars.begin(), stars.end(), 0ll,
            [](int64_t total, const star& s){ return total + s.pos.x; }) / static_cast<int64_t>(stars.size());
        const auto avg_y = std::accumulate(stars.begin(), stars.end(), 0ll,
            [](int64_t total, const star& s){ return total + s.pos.y; }) / static_cast<int64_t>(stars.size());
        return std::accumulate(stars.begin(), stars.end(), 0ll,
            [avg_x, avg_y](int64_t total, const star& s) {
                const auto x = s.pos.x - avg_x;
                const auto y = s.pos.y - avg_y;
                return total + x * x + y * y;
            });
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        auto var = variance(input), last_var = std::numeric_limits<int64_t>::max();
        //fmt::print("Starting:     {}\n", var);
        int last_min_x = 0, last_min_y = 0, last_max_x = 0, last_max_y = 0;
        std::string msg;
        for (int i = 0; i < 100000; ++i) {
            const auto [min_x, max_x, min_y, max_y] = step(input);
            var = variance(input);
            //fmt::print("Step {:6}:  {}\n", i+1, var);
            if (var > last_var) {
                step_back(input);
                grid<char> g {static_cast<size_t>(last_max_x - last_min_x + 1 + 2), static_cast<size_t>(last_max_y - last_min_y + 1)};
                position offset {-last_min_x, -last_min_y};
                for (auto& c : g) { c = '.'; }
                for (const auto& s : input) {
                    g[s.pos + offset] = '#';
                }
                g = g.transpose();
                //g.display(std::cout);
                // fmt::print("Step {:6} has the message.\n", i);
                msg = letters_from_grid(g, 8, 10);//By inspecting output.
                break;
            }
            last_var = var;
            last_min_x = min_x;
            last_min_y = min_y;
            last_max_x = max_x;
            last_max_y = max_y;
        }

        return msg;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        auto var = variance(input), last_var = std::numeric_limits<int64_t>::max();
        //fmt::print("Starting:     {}\n", var);
        int message_step = -1;
        for (int i = 0; i < 100000; ++i) {
            const auto [min_x, max_x, min_y, max_y] = step(input);
            var = variance(input);
            //fmt::print("Step {:6}:  {}\n", i+1, var);
            if (var > last_var) {
                message_step = i;
                break;
            }
            last_var = var;
        }
        return std::to_string(message_step);
    }

    aoc::registration r {2018, 10, part_1, part_2};

//    TEST_SUITE("2018_day10") {
//        TEST_CASE("2018_day10:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */