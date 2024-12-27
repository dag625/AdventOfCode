//
// Created by Dan on 12/12/2024.
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
     See https://adventofcode.com/2017/day/19
     */

    grid<char> get_input(const std::vector<std::string>& lines) {
        return to_grid(lines);
    }

    position find_start(const grid<char>& g) {
        position retval {};
        for (; retval.y < g.num_cols(); ++retval.y) {
            if (g[retval] == '|') {
                return retval;
            }
        }
        return retval;
    }

    void step(const grid<char>& g, position& current, position& last, std::string& letters) {
        const velocity delta {current.x - last.x, current.y - last.y};
        auto next = current + delta;
        //Technically, the text never states that we only turn at '+'s, but that seems to be true and is semi-implied...
        if (g[next] == '+') {
            for (const auto n : CARDINAL_DIRECTIONS) {
                const auto nn = next + n;
                if (nn != current) {
                    if (g[nn] == '|' || g[nn] == '-') {
                        //Our next next is nn, so to get there by going strait we set 'last' to be 'nn-2*n' or 'next-n'.
                        last = next - n;
                        current = next;
                        break;
                    }
                }
            }
        }
        else if (g[next] == ' ') {
            //Stop, end of line
            last = current;
        }
        else {
            if (isalpha(g[next])) {
                letters.push_back(g[next]);
            }
            last = current;
            current = next;
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto current = find_start(input);
        position last {-1, current.y};
        std::string letters;
        while (current != last) {
            step(input, current, last, letters);
        }
        return letters;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto current = find_start(input);
        position last {-1, current.y};
        std::string letters;
        int steps = 0;
        while (current != last) {
            step(input, current, last, letters);
            ++steps;
        }
        return std::to_string(steps);
    }

    aoc::registration r{2017, 19, part_1, part_2};

//    TEST_SUITE("2017_day19") {
//        TEST_CASE("2017_day19:example") {
//
//        }
//    }

} /* namespace <anon> */