//
// Created by Dan on 11/12/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"
#include "parse.h"
#include "letters.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct rect {
        int width = 0;
        int height = 0;
    };

    struct rotate {
        bool row = true;
        int index = 0;
        int amount = 0;
    };

    using instruction = std::variant<rect, rotate>;

    instruction parse_instruction(std::string_view s) {
        if (s.starts_with("rect ")) {
            s.remove_prefix(5);
            const auto p = split(s, 'x');
            return rect{parse<int>(p[0]), parse<int>(p[1])};
        }
        else {
            s.remove_prefix(7);
            bool row = s.starts_with("row");
            s.remove_prefix(row ? 4 : 7);
            const auto p = split(s, ' ');
            const auto q = split(p[0], '=');
            return rotate{row, parse<int>(q[1]), parse<int>(p[2])};
        }
    }

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_instruction) | to<std::vector<instruction>>();
    }

    struct do_instruction {
        grid<char>& screen;

        do_instruction(grid<char>& s) : screen{s} {}

        void operator()(const rect& r) {
            for (int x = 0; x < r.height && x < screen.num_rows(); ++x) {
                for (int y = 0; y < r.width && y < screen.num_cols(); ++y) {
                    screen.at(x, y) = '#';
                }
            }
        }

        void operator()(const rotate& r) {
            if (r.row) {
                auto row = screen.row(r.index);
                std::rotate(row.begin(), row.begin() + row.size() - r.amount, row.end());
            }
            else {
                auto col = screen.column(r.index);
                std::rotate(col.begin(), col.begin() + col.size() - r.amount, col.end());
            }
        }
    };

    /*
    --- Day 8: Two-Factor Authentication ---
    You come across a door implementing what you can only assume is an implementation of two-factor authentication after a long game of requirements telephone.

    To get past the door, you first swipe a keycard (no problem; there was one on a nearby desk). Then, it displays a code on a little screen, and you type that code on a keypad. Then, presumably, the door unlocks.

    Unfortunately, the screen has been smashed. After a few minutes, you've taken everything apart and figured out how it works. Now you just have to work out what the screen would have displayed.

    The magnetic strip on the card you swiped encodes a series of instructions for the screen; these instructions are your puzzle input. The screen is 50 pixels wide and 6 pixels tall, all of which start off, and is capable of three somewhat peculiar operations:

    rect AxB turns on all of the pixels in a rectangle at the top-left of the screen which is A wide and B tall.
    rotate row y=A by B shifts all of the pixels in row A (0 is the top row) right by B pixels. Pixels that would fall off the right end appear at the left end of the row.
    rotate column x=A by B shifts all of the pixels in column A (0 is the left column) down by B pixels. Pixels that would fall off the bottom appear at the top of the column.
    For example, here is a simple sequence on a smaller screen:

    rect 3x2 creates a small rectangle in the top-left corner:

    ###....
    ###....
    .......
    rotate column x=1 by 1 rotates the second column down by one pixel:

    #.#....
    ###....
    .#.....
    rotate row y=0 by 4 rotates the top row right by four pixels:

    ....#.#
    ###....
    .#.....
    rotate column x=1 by 1 again rotates the second column down by one pixel, causing the bottom pixel to wrap back to the top:

    .#..#.#
    #.#....
    .#.....
    As you can see, this display technology is extremely powerful, and will soon dominate the tiny-code-displaying-screen market. That's what the advertisement on the back of the display tries to convince you, anyway.

    There seems to be an intermediate check of the voltage used by the display: after you swipe your card, if the screen did work, how many pixels should be lit?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        grid<char> screen {6, 50};
        std::fill(screen.begin(), screen.end(), '.');
        for (const auto& ins : input) {
            std::visit(do_instruction{screen}, ins);
        }
        const auto num = std::count(screen.begin(), screen.end(), '#');
        return std::to_string(num);
    }

    /*
    --- Part Two ---
    You notice that the screen is only capable of displaying capital letters; in the font it uses, each letter is 5 pixels wide and 6 tall.

    After you swipe your card, what code is the screen trying to display?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        grid<char> screen {6, 50};
        std::fill(screen.begin(), screen.end(), '.');
        for (const auto& ins : input) {
            std::visit(do_instruction{screen}, ins);
        }
        return letters_from_grid(screen);
    }

    aoc::registration r{2016, 8, part_1, part_2};

    TEST_SUITE("2016_day8") {
        TEST_CASE("2016_day8:example") {
            const std::vector<instruction> input {
                rect{3, 2},
                rotate{false, 1, 1},
                rotate{true, 0, 4},
                rotate{false, 1, 1}
            };
            grid<char> screen {3, 7};
            std::fill(screen.begin(), screen.end(), '.');
            screen.display(std::cout);
            for (const auto& ins : input) {
                std::visit(do_instruction{screen}, ins);
                screen.display(std::cout);
            }
            const auto num = std::count(screen.begin(), screen.end(), '#');
            CHECK_EQ(num, 6);
        }
    }

}