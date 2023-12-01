//
// Created by Dan on 12/1/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>
#include <array>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr std::string_view DIGITS[] = { "zero"sv, "one"sv, "two"sv, "three"sv, "four"sv, "five"sv, "six"sv, "seven"sv, "eight"sv, "nine"sv };

    bool is_digit(std::string_view s, std::string_view d) {
        const auto tmp = s.substr(0, d.size());
        return tmp == d;
    }

    std::pair<int, int> find_digit(std::string_view s) {
        const auto first = s.find_first_of("0123456789");
        const auto last = s.find_last_of("0123456789");
        return {first, last};
    }

    int parse_value(std::string_view s) {
        const auto [f, l] = find_digit(s);
        const auto firstDigit = static_cast<int>(s[f] - '0');
        const auto secondDigit = static_cast<int>(s[l] - '0');
        return firstDigit * 10 + secondDigit;
    }

    std::pair<int, int> find_abc_digit(std::string_view s) {
        int bestFirst = static_cast<int>(s.size()) + 1, bestLast = -1;
        for (const auto digit : DIGITS) {
            int off = static_cast<int>(s.find(digit));
            while (off != std::string_view::npos) {
                if (off < bestFirst) {
                    bestFirst = static_cast<int>(off);
                }
                if (off > bestLast) {
                    bestLast = static_cast<int>(off);
                }
                off = s.find(digit, off + 1);
            }
        }
        return {bestFirst, bestLast};
    }

    int parse_abc_value(std::string_view s) {
        const auto [firstDigit, secondDigit] = find_digit(s);
        const auto [firstAbcDigit, secondAbcDigit] = find_abc_digit(s);
        int retval = 0;
        if (firstDigit < firstAbcDigit) {
            retval += static_cast<int>(s[firstDigit] - '0');
        }
        else {
            for (const auto [val, str] : DIGITS | std::views::enumerate) {
                if (is_digit(s.substr(firstAbcDigit), str)) {
                    retval += static_cast<int>(val);
                    break;
                }
            }
        }
        retval *= 10;
        if (secondDigit > secondAbcDigit) {
            retval += static_cast<int>(s[secondDigit] - '0');
        }
        else {
            for (const auto [val, str] : DIGITS | std::views::enumerate) {
                if (is_digit(s.substr(secondAbcDigit), str)) {
                    retval += static_cast<int>(val);
                    break;
                }
            }
        }
        return retval;
    }

    std::vector<std::string> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_1_input.txt");
        return lines;
    }

    /*
    --- Day 1: Trebuchet?! ---
    Something is wrong with global snow production, and you've been selected to take a look. The Elves have even given you a map; on it, they've used stars to mark the top fifty locations that are likely to be having problems.

    You've been doing this long enough to know that to restore snow operations, you need to check all fifty stars by December 25th.

    Collect stars by solving puzzles. Two puzzles will be made available on each day in the Advent calendar; the second puzzle is unlocked when you complete the first. Each puzzle grants one star. Good luck!

    You try to ask why they can't just use a weather machine ("not powerful enough") and where they're even sending you ("the sky") and why your map looks mostly blank ("you sure ask a lot of questions") and hang on did you just say the sky ("of course, where do you think snow comes from") when you realize that the Elves are already loading you into a trebuchet ("please hold still, we need to strap you in").

    As they're making the final adjustments, they discover that their calibration document (your puzzle input) has been amended by a very young Elf who was apparently just excited to show off her art skills. Consequently, the Elves are having trouble reading the values on the document.

    The newly-improved calibration document consists of lines of text; each line originally contained a specific calibration value that the Elves now need to recover. On each line, the calibration value can be found by combining the first digit and the last digit (in that order) to form a single two-digit number.

    For example:

    1abc2
    pqr3stu8vwx
    a1b2c3d4e5f
    treb7uchet
    In this example, the calibration values of these four lines are 12, 38, 15, and 77. Adding these together produces 142.

    Consider your entire calibration document. What is the sum of all of the calibration values?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto vals = input | std::views::transform(&parse_value) | to<std::vector<int>>();
        const auto sum = std::accumulate(vals.begin(), vals.end(), 0);
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    Your calculation isn't quite right. It looks like some of the digits are actually spelled out with letters: one, two, three, four, five, six, seven, eight, and nine also count as valid "digits".

    Equipped with this new information, you now need to find the real first and last digit on each line. For example:

    two1nine
    eightwothree
    abcone2threexyz
    xtwone3four
    4nineeightseven2
    zoneight234
    7pqrstsixteen
    In this example, the calibration values are 29, 83, 13, 24, 42, 14, and 76. Adding these together produces 281.

    What is the sum of all of the calibration values?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto vals = input | std::views::transform(&parse_abc_value) | to<std::vector<int>>();
        const auto sum = std::accumulate(vals.begin(), vals.end(), 0);
        return std::to_string(sum);//30640, 43233
    }

    aoc::registration r{2023, 01, part_1, part_2};

//    TEST_SUITE("2023_day01") {
//        TEST_CASE("2023_day01:example") {
//
//        }
//    }

}