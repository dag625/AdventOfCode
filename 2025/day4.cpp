//
// Created by Dan on 12/4/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "grid.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/4
     */

    grid<char> get_input(const std::vector<std::string>& lines) {
        return to_grid(lines);
    }

    int count_accessible(const grid<char>& g) {
        int retval = 0;
        for (const auto p : g.list_positions()) {
            if (g[p] == '@') {
                int occupied = 0;
                for (const auto n : g.neighbors(p)) {
                    if (g.in(n) && g[n] == '@') {
                        ++occupied;
                    }
                }
                if (occupied < 4) {
                    ++retval;
                }
            }
        }
        return retval;
    }

    int remove_accessible(grid<char>& g) {
        int num_removed = 0;
        for (const auto p : g.list_positions()) {
            if (g[p] == '@') {
                int occupied = 0;
                for (const auto n : g.neighbors(p)) {
                    if (g.in(n) && g[n] == '@') {
                        ++occupied;
                    }
                }
                if (occupied < 4) {
                    ++num_removed;
                    g[p] = 'x';
                }
            }
        }
        return num_removed;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto num = count_accessible(input);
        return std::to_string(num);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        int total = 0;
        while (int num = remove_accessible(input)) {
            total += num;
        }
        return std::to_string(total);
    }

    aoc::registration r {2025, 4, part_1, part_2};

//    TEST_SUITE("2025_day04") {
//        TEST_CASE("2025_day04:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */