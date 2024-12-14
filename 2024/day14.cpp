//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "point.h"
#include "parse.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/14
     */

    struct robot {
        position pos;
        velocity vel;
    };

    robot parse_robot(std::string_view s) {
        auto parts = split(s, ' ');
        parts[0].remove_prefix(2);
        parts[1].remove_prefix(2);
        const auto pp = split(parts[0], ',');
        const auto vp = split(parts[1], ',');
        return {{parse<int>(pp[0]), parse<int>(pp[1])}, {parse<int>(vp[0]), parse<int>(vp[1])}};
    }

    std::vector<robot> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_14_input.txt");
        return lines | std::views::transform(&parse_robot) | std::ranges::to<std::vector>();
    }

    constexpr int X_SIZE = 101;
    constexpr int Y_SIZE = 103;

    void wrap(int& pos, const int size) {
        if (pos >= 0) {
            pos %= size;
        }
        else {
            while (pos < 0) {
                pos += size;
            }
        }
    }

    void wrap(robot& r, const int xsize, const int ysize) {
        wrap(r.pos.x, xsize);
        wrap(r.pos.y, ysize);
    }

    void move(robot& r, const int times, const int xsize, const int ysize) {
        r.pos += (r.vel * times);
        wrap(r, xsize, ysize);
    }

    int safety_factor(const std::vector<robot>& robots, const int xsize, const int ysize) {
        int q1 = 0, q2 = 0, q3 = 0, q4 = 0;
        const int midx = xsize / 2, midy = ysize / 2;
        for (const auto& r : robots) {
            if (r.pos.x < midx && r.pos.y < midy) {
                ++q1;
            }
            else if (r.pos.x > midx && r.pos.y < midy) {
                ++q2;
            }
            else if (r.pos.x < midx && r.pos.y > midy) {
                ++q3;
            }
            else if (r.pos.x > midx && r.pos.y > midy) {
                ++q4;
            }
            //else is on the mid-point lines
        }
        return q1 * q2 * q3 * q4;
    }

    grid<char> create_grid(const std::vector<robot>& robots, const int xsize, const int ysize) {
        grid<char> retval {static_cast<std::size_t>(xsize), static_cast<std::size_t>(ysize)};
        for (auto& p : retval) {
            p = '.';
        }
        for (const auto& r : robots) {
            retval[r.pos] = '#';
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        for (auto& r : input) {
            move(r, 100, X_SIZE, Y_SIZE);
        }
        const auto sf = safety_factor(input, X_SIZE, Y_SIZE);
        return std::to_string(sf);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        int min_val = std::numeric_limits<int>::max(), min_i = -1;
        for (int i = 0; i < 10000; ++i) {
            for (auto& r : input) {
                move(r, 1, X_SIZE, Y_SIZE);
            }
            const auto sf = safety_factor(input, X_SIZE, Y_SIZE);
            if (sf < min_val) {
                min_val = sf;
                min_i = i;
            }
            if ((i+1) == -1) {
                /*
                 * It turns out that the "Easter egg" is actually an easter egg of a Christmas tree and not
                 * a literal Easter egg...
                 */
                std::cout << "\nAfter " << (i + 1) << " moves:\n";
                create_grid(input, X_SIZE, Y_SIZE).display(std::cout);
                std::cout << '\n' << std::endl;
            }
        }
        return std::to_string(min_i + 1);
    }

    aoc::registration r{2024, 14, part_1, part_2};

    TEST_SUITE("2024_day14") {
        TEST_CASE("2024_day14:example") {
            const int EX_XSIZE = 11;
            const int EX_YSIZE = 7;
            const std::vector<std::string> lines {
                    "p=0,4 v=3,-3",
                    "p=6,3 v=-1,-3",
                    "p=10,3 v=-1,2",
                    "p=2,0 v=2,-1",
                    "p=0,0 v=1,3",
                    "p=3,0 v=-2,-2",
                    "p=7,6 v=-1,-3",
                    "p=3,0 v=-1,-2",
                    "p=9,3 v=2,3",
                    "p=7,3 v=-1,2",
                    "p=2,4 v=2,-3",
                    "p=9,5 v=-3,-3"
            };
            auto input = lines | std::views::transform(&parse_robot) | std::ranges::to<std::vector>();
            for (auto& r : input) {
                move(r, 100, EX_XSIZE, EX_YSIZE);
            }
            const auto sf = safety_factor(input, EX_XSIZE, EX_YSIZE);
            CHECK_EQ(sf, 12);
        }
    }

} /* namespace <anon> */