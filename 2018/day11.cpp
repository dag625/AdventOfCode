//
// Created by Dan on 11/9/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "parse.h"
#include "grid.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/11
     */

    int get_input(const std::vector<std::string>& lines) {
        return parse32(lines.front());
    }

    constexpr std::size_t GRID_SIZE = 300;

    grid<int> create_grid(const int serial_num) {
        grid<int> retval{GRID_SIZE, GRID_SIZE};
        //Cordinates actually start at 1, so we need to be mindful of that.
        for (const auto p : retval.list_positions()) {
            const int rid = (p.x + 1) + 10;
            int power = rid * (p.y + 1) + serial_num;
            power *= rid;
            power /= 100;
            power %= 10;
            retval[p] = power - 5;
        }
        return retval;
    }

    position find_highest_3x3(const grid<int>& levels) {
        position retval{};
        int max = std::numeric_limits<int>::min();
        for (int x = 0; x < levels.num_rows() - 2; ++x) {
            for (int y = 0; y < levels.num_cols() - 2; ++y) {
                int total = levels[{x + 0, y + 0}] +
                        levels[{x + 0, y + 1}] +
                        levels[{x + 0, y + 2}] +
                        levels[{x + 1, y + 0}] +
                        levels[{x + 1, y + 1}] +
                        levels[{x + 1, y + 2}] +
                        levels[{x + 2, y + 0}] +
                        levels[{x + 2, y + 1}] +
                        levels[{x + 2, y + 2}];
                if (total > max) {
                    max = total;
                    retval = {x, y};
                }
            }
        }
        return retval;
    }

    std::pair<position, int> find_highest_NxX(const grid<int>& levels, grid<int>& prev, const int sz) {
        const int last_sz = sz - 1;
        position retval{};
        int max = std::numeric_limits<int>::min();
        for (int x = 0; x < levels.num_rows() - last_sz; ++x) {
            for (int y = 0; y < levels.num_cols() - last_sz; ++y) {
                int a = prev[{x, y}], b = levels[{x + last_sz, y + last_sz}], e, f;
                int total = prev[{x, y}] + levels[{x + last_sz, y + last_sz}];
                for (int d = 0; d < last_sz; ++d) {
                    position ep {x + last_sz, y + d};
                    position fp {x + d, y + last_sz};
                    e = levels[ep];
                    f = levels[fp];
                    total += levels[{x + last_sz, y + d}] + levels[{x + d, y + last_sz}];
                }
                // int total_l = 0;
                // for (int dx = 0; dx < sz; ++dx) {
                //     for (int dy = 0; dy < sz; ++dy) {
                //         total_l += levels[{x + dx, y + dy}];
                //     }
                // }
                // if (sz > 1) {
                //     fmt::print("Size {:3} @ {:3},{:3} = {:5} vs {:5}\n", sz, x, y, total, total_l);
                // }
                prev[{x, y}] = total;
                if (total > max) {
                    max = total;
                    retval = {x, y};
                }
            }
        }
        return {retval, max};
    }

    std::string p2_from_serial_num(const int serial_num) {
        const auto levels = create_grid(serial_num);
        position max_p{};
        int max_tot = std::numeric_limits<int>::min(), max_sz = 0;
        //levels.display(std::cout);
        grid<int> prev {levels.num_rows(), levels.num_cols()};
        for (int sz = 1; sz <= GRID_SIZE; ++sz) {
            auto [p, val] = find_highest_NxX(levels, prev, sz);
            //fmt::print("Size {:3} has max {:4} at {:3},{:3}.\n", sz, val, p.x+1, p.y+1);
            std::cout << std::flush;
            if (val > max_tot) {
                max_tot = val;
                max_p = p;
                max_sz = sz;
            }
        }
        return fmt::format("{},{},{}", max_p.x+1, max_p.y+1, max_sz);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto levels = create_grid(input);
        const auto max = find_highest_3x3(levels);
        return fmt::format("{},{}", max.x+1, max.y+1);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        return p2_from_serial_num(input);
        //239,210,61 not right
    }

    aoc::registration r {2018, 11, part_1, part_2};

    TEST_SUITE("2018_day11") {
        TEST_CASE("2018_day11:example") {
            using namespace std::string_literals;
            const auto g1 = create_grid(57);
            const auto g2 = create_grid(39);
            const auto g3 = create_grid(71);

            CHECK_EQ(g1[{122-1,79-1}], -5);
            CHECK_EQ(g2[{217-1,196-1}], 0);
            CHECK_EQ(g3[{101-1,153-1}], 4);

            CHECK_EQ(p2_from_serial_num(18), "90,269,16"s);
            CHECK_EQ(p2_from_serial_num(42), "232,251,12"s);
        }
    }

} /* namespace <anon> */