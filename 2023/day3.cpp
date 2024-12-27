//
// Created by Dan on 12/3/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "point.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<std::string> get_input(const std::vector<std::string>& lines) {
        return lines;
    }

    bool is_symbol(char c) noexcept {
        return c != '.' && !isdigit(c);
    }

    void get_number(int c, const std::string& row, std::vector<int64_t>& found) {
        int start = c, end = c + 1;
        while (start >= 0 && isdigit(row[start])) {
            --start;
        }
        ++start;
        while (end < row.size() && isdigit(row[end])) {
            ++end;
        }
        const auto val = parse<int64_t>(std::string_view{row.begin() + start, row.begin() + end});
        found.push_back(val);
    }

    void get_vertical_neighboring_parts(int r, int c, const std::vector<std::string>& input, std::vector<int64_t>& found) {
        if (r < 0 || r >= input.size()) {
            return;
        }
        //Inspecting the input shows there are no stars on the edges, so we can neglect edge checking to start.
        bool pre = isdigit(input[r][c - 1]), at = isdigit(input[r][c]), post = isdigit(input[r][c + 1]);
        if (pre && post && !at) {//two at corners
            get_number(c - 1, input[r], found);
            get_number(c + 1, input[r], found);
        }
        else {//one only
            if (pre) {
                get_number(c - 1, input[r], found);
            }
            else if (post) {
                get_number(c + 1, input[r], found);
            }
            else if (at) { //Single digit number at c.
                found.push_back(parse<int64_t>(std::string_view{input[r].begin() + c, input[r].begin() + c + 1}));
            }
        }
    }

    /*
    --- Day 3: Gear Ratios ---
    You and the Elf eventually reach a gondola lift station; he says the gondola lift will take you up to the water source, but this is as far as he can bring you. You go inside.

    It doesn't take long to find the gondolas, but there seems to be a problem: they're not moving.

    "Aaah!"

    You turn around to see a slightly-greasy Elf with a wrench and a look of surprise. "Sorry, I wasn't expecting anyone! The gondola lift isn't working right now; it'll still be a while before I can fix it." You offer to help.

    The engineer explains that an engine part seems to be missing from the engine, but nobody can figure out which one. If you can add up all the part numbers in the engine schematic, it should be easy to work out which part is missing.

    The engine schematic (your puzzle input) consists of a visual representation of the engine. There are lots of numbers and symbols you don't really understand, but apparently any number adjacent to a symbol, even diagonally, is a "part number" and should be included in your sum. (Periods (.) do not count as a symbol.)

    Here is an example engine schematic:

    467..114..
    ...*......
    ..35..633.
    ......#...
    617*......
    .....+.58.
    ..592.....
    ......755.
    ...$.*....
    .664.598..
    In this schematic, two numbers are not part numbers because they are not adjacent to a symbol: 114 (top right) and 58 (middle right). Every other number is adjacent to a symbol and so is a part number; their sum is 4361.

    Of course, the actual engine schematic is much larger. What is the sum of all of the part numbers in the engine schematic?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (int r = 0; r < input.size(); ++r) {
            for (int c = 0; c < input[r].size(); ++c) {
                int start = c;
                bool has_sym = false;
                while (isdigit(input[r][c]) && c < input[r].size()) {
                    position p {r, c};
                    if (!has_sym) {
                        for (const auto &v: STANDARD_DIRECTIONS) {
                            const auto n = p + v;
                            if (n.x >= 0 && n.x < input.size() && n.y >= 0 && n.y < input[r].size() && is_symbol(input[n.x][n.y])) {
                                has_sym = true;
                                break;
                            }
                        }
                    }
                    ++c;
                }
                if (has_sym) {
                    const auto val = parse<int64_t>(std::string_view{input[r].begin() + start, input[r].begin() + c});
                    sum += val;
                }
            }
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    The engineer finds the missing part and installs it in the engine! As the engine springs to life, you jump in the closest gondola, finally ready to ascend to the water source.

    You don't seem to be going very fast, though. Maybe something is still wrong? Fortunately, the gondola has a phone labeled "help", so you pick it up and the engineer answers.

    Before you can explain the situation, she suggests that you look out the window. There stands the engineer, holding a phone in one hand and waving with the other. You're going so slowly that you haven't even left the station. You exit the gondola.

    The missing part wasn't the only issue - one of the gears in the engine is wrong. A gear is any * symbol that is adjacent to exactly two part numbers. Its gear ratio is the result of multiplying those two numbers together.

    This time, you need to find the gear ratio of every gear and add them all up so that the engineer can figure out which gear needs to be replaced.

    Consider the same engine schematic again:

    467..114..
    ...*......
    ..35..633.
    ......#...
    617*......
    .....+.58.
    ..592.....
    ......755.
    ...$.*....
    .664.598..
    In this schematic, there are two gears. The first is in the top left; it has part numbers 467 and 35, so its gear ratio is 16345. The second gear is in the lower right; its gear ratio is 451490. (The * adjacent to 617 is not a gear because it is only adjacent to one part number.) Adding up all of the gear ratios produces 467835.

    What is the sum of all of the gear ratios in your engine schematic?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (int r = 0; r < input.size(); ++r) {
            for (int c = 0; c < input[r].size(); ++c) {
                if (input[r][c] == '*') {
                    std::vector<int64_t> found;
                    get_vertical_neighboring_parts(r - 1, c, input, found);
                    get_vertical_neighboring_parts(r + 1, c, input, found);
                    if (c - 1 >= 0 && isdigit(input[r][c - 1])) {
                        int start = c - 2;
                        while (start >= 0 && isdigit(input[r][start])) {
                            --start;
                        }
                        const auto val = parse<int64_t>(std::string_view{input[r].begin() + (start + 1), input[r].begin() + c});
                        found.push_back(val);
                    }
                    if (c + 1 < input[r].size() && isdigit(input[r][c + 1])) {
                        int end = c + 2;
                        while (end < input[r].size() && isdigit(input[r][end])) {
                            ++end;
                        }
                        const auto val = parse<int64_t>(std::string_view{input[r].begin() + c + 1, input[r].begin() + end});
                        found.push_back(val);
                    }
                    if (found.size() == 2) {
                        sum += found.front() * found.back();
                    }
                }
            }
        }
        return std::to_string(sum);
    }

    aoc::registration r{2023, 3, part_1, part_2};

//    TEST_SUITE("2023_day03") {
//        TEST_CASE("2023_day03:example") {
//
//        }
//    }

}