//
// Created by Daniel Garcia on 6/8/21.
//

#include "registration.h"
#include "parse.h"

#include <doctest/doctest.h>

#include <string_view>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr auto INPUT = "3113322113"sv;

    std::string iterate(std::string_view prev) {
        char last = '\0';
        int num_last = 0;
        std::string retval;
        retval.reserve(prev.size() * 1.5);
        for (char c : prev) {
            if (c != last) {
                if (num_last > 0) {
                    retval += std::to_string(num_last);
                    retval += last;
                }
                last = c;
                num_last = 1;
            }
            else {
                ++num_last;
            }
        }
        if (num_last > 0) {
            retval += std::to_string(num_last);
            retval += last;
        }
        return retval;
    }

    /*
    Today, the Elves are playing a game called look-and-say. They take turns making sequences by reading aloud the previous sequence and using that reading as the next sequence. For example, 211 is read as "one two, two ones", which becomes 1221 (1 2, 2 1s).

    Look-and-say sequences are generated iteratively, using the previous value as input for the next step. For each step, take the previous value, and replace each run of digits (like 111) with the number of digits (3) followed by the digit itself (1).

    For example:

    1 becomes 11 (1 copy of digit 1).
    11 becomes 21 (2 copies of digit 1).
    21 becomes 1211 (one 2 followed by one 1).
    1211 becomes 111221 (one 1, one 2, and two 1s).
    111221 becomes 312211 (three 1s, two 2s, and one 1).
    Starting with the digits in your puzzle input, apply this process 40 times. What is the length of the result?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        std::string puzzle {INPUT};
        for (int i = 0; i < 40; ++i) {
            puzzle = iterate(puzzle);
        }
        return std::to_string(puzzle.size());
    }

    /*
    Neat, right? You might also enjoy hearing John Conway talking about this sequence (that's Conway of Conway's Game of Life fame).

    Now, starting again with the digits in your puzzle input, apply this process 50 times. What is the length of the new result?
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        std::string puzzle {INPUT};
        for (int i = 0; i < 50; ++i) {
            puzzle = iterate(puzzle);
        }
        return std::to_string(puzzle.size());
    }

    aoc::registration r {2015, 10, part_1, part_2};

    TEST_SUITE("2015_day10") {
        TEST_CASE("2015_day10:example") {
            std::string puzzle = "1";

            puzzle = iterate(puzzle);
            REQUIRE_EQ(puzzle, "11");

            puzzle = iterate(puzzle);
            REQUIRE_EQ(puzzle, "21");

            puzzle = iterate(puzzle);
            REQUIRE_EQ(puzzle, "1211");

            puzzle = iterate(puzzle);
            REQUIRE_EQ(puzzle, "111221");

            puzzle = iterate(puzzle);
            REQUIRE_EQ(puzzle, "312211");
        }
    }

}
