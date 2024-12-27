//
// Created by Dan on 11/12/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::string get_input(const std::vector<std::string>& lines) {
        return lines.front();
    }

    std::string expand(std::string_view s) {
        std::string retval;
        retval.reserve(s.size() * 2);
        while (!s.empty()) {
            const auto start = s.find_first_of('(');
            if (start == std::string_view::npos) {
                retval.insert(retval.end(), s.begin(), s.end());
                s.remove_prefix(s.size());
            }
            else {
                retval.insert(retval.end(), s.begin(), s.begin() + start);
                s.remove_prefix(start + 1);
                const auto finish = s.find_first_of(')');
                const auto marker = s.substr(0, finish);
                s.remove_prefix(finish + 1);
                const auto parts = split(marker, 'x');
                const auto num_chars = parse<int>(parts[0]);
                const auto num_times = parse<int>(parts[1]);
                const auto repeated = s.substr(0, num_chars);
                s.remove_prefix(num_chars);
                for (int i = 0; i < num_times; ++i) {
                    retval.insert(retval.end(), repeated.begin(), repeated.end());
                }
            }
        }
        return retval;
    }

    std::size_t calculate_length_non_recursive(std::string_view s) {
        std::size_t retval = 0;
        while (!s.empty()) {
            const auto start = s.find_first_of('(');
            if (start == std::string_view::npos) {
                retval += s.size();
                s.remove_prefix(s.size());
            }
            else {
                retval += start;
                s.remove_prefix(start + 1);
                const auto finish = s.find_first_of(')');
                const auto marker = s.substr(0, finish);
                s.remove_prefix(finish + 1);
                const auto parts = split(marker, 'x');
                const auto num_chars = parse<int>(parts[0]);
                const auto num_times = parse<int>(parts[1]);
                const auto repeated = s.substr(0, num_chars);
                retval += num_times * num_chars;
                s.remove_prefix(num_chars);
            }
        }
        return retval;
    }

    std::size_t calculate_length(std::string_view s) {
        std::size_t retval = 0;
        while (!s.empty()) {
            const auto start = s.find_first_of('(');
            if (start == std::string_view::npos) {
                retval += s.size();
                s.remove_prefix(s.size());
            }
            else {
                retval += start;
                s.remove_prefix(start + 1);
                const auto finish = s.find_first_of(')');
                const auto marker = s.substr(0, finish);
                s.remove_prefix(finish + 1);
                const auto parts = split(marker, 'x');
                const auto num_chars = parse<int>(parts[0]);
                const auto num_times = parse<int>(parts[1]);
                const auto repeated = s.substr(0, num_chars);
                retval += num_times * calculate_length(repeated);
                s.remove_prefix(num_chars);
            }
        }
        return retval;
    }

    /*
    --- Day 9: Explosives in Cyberspace ---
    Wandering around a secure area, you come across a datalink port to a new part of the network. After briefly scanning it for interesting files, you find one file in particular that catches your attention. It's compressed with an experimental format, but fortunately, the documentation for the format is nearby.

    The format compresses a sequence of characters. Whitespace is ignored. To indicate that some sequence should be repeated, a marker is added to the file, like (10x2). To decompress this marker, take the subsequent 10 characters and repeat them 2 times. Then, continue reading the file after the repeated data. The marker itself is not included in the decompressed output.

    If parentheses or other characters appear within the data referenced by a marker, that's okay - treat it like normal data, not a marker, and then resume looking for markers after the decompressed section.

    For example:

    ADVENT contains no markers and decompresses to itself with no changes, resulting in a decompressed length of 6.
    A(1x5)BC repeats only the B a total of 5 times, becoming ABBBBBC for a decompressed length of 7.
    (3x3)XYZ becomes XYZXYZXYZ for a decompressed length of 9.
    A(2x2)BCD(2x2)EFG doubles the BC and EF, becoming ABCBCDEFEFG for a decompressed length of 11.
    (6x1)(1x3)A simply becomes (1x3)A - the (1x3) looks like a marker, but because it's within a data section of another marker, it is not treated any differently from the A that comes after it. It has a decompressed length of 6.
    X(8x2)(3x3)ABCY becomes X(3x3)ABC(3x3)ABCY (for a decompressed length of 18), because the decompressed data from the (8x2) marker (the (3x3)ABC) is skipped and not processed further.
    What is the decompressed length of the file (your puzzle input)? Don't count whitespace.
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto len = calculate_length_non_recursive(input);
        return std::to_string(len);
    }

    /*
    --- Part Two ---
    Apparently, the file actually uses version two of the format.

    In version two, the only difference is that markers within decompressed data are decompressed. This, the documentation explains, provides much more substantial compression capabilities, allowing many-gigabyte files to be stored in only a few kilobytes.

    For example:

    (3x3)XYZ still becomes XYZXYZXYZ, as the decompressed section contains no markers.
    X(8x2)(3x3)ABCY becomes XABCABCABCABCABCABCY, because the decompressed data from the (8x2) marker is then further decompressed, thus triggering the (3x3) marker twice for a total of six ABC sequences.
    (27x12)(20x12)(13x14)(7x10)(1x12)A decompresses into a string of A repeated 241920 times.
    (25x3)(3x3)ABC(2x3)XY(5x2)PQRSTX(18x9)(3x2)TWO(5x7)SEVEN becomes 445 characters long.
    Unfortunately, the computer you brought probably doesn't have enough memory to actually decompress the file; you'll have to come up with another way to get its decompressed length.

    What is the decompressed length of the file using this improved format?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto len = calculate_length(input);
        return std::to_string(len);
    }

    aoc::registration r{2016, 9, part_1, part_2};

    TEST_SUITE("2016_day9") {
        TEST_CASE("2016_day9:example_p1") {
            CHECK_EQ(expand("A(1x5)BC"), std::string{"ABBBBBC"});
            CHECK_EQ(expand("(3x3)XYZ"), std::string{"XYZXYZXYZ"});
            CHECK_EQ(expand("A(2x2)BCD(2x2)EFG"), std::string{"ABCBCDEFEFG"});
            CHECK_EQ(expand("(6x1)(1x3)A"), std::string{"(1x3)A"});
            CHECK_EQ(expand("X(8x2)(3x3)ABCY"), std::string{"X(3x3)ABC(3x3)ABCY"});
        }
        TEST_CASE("2016_day9:example_p2") {
            CHECK_EQ(calculate_length("(3x3)XYZ"), 9);
            CHECK_EQ(calculate_length("X(8x2)(3x3)ABCY"), 20);
            CHECK_EQ(calculate_length("(27x12)(20x12)(13x14)(7x10)(1x12)A"), 241920);
            CHECK_EQ(calculate_length("(25x3)(3x3)ABC(2x3)XY(5x2)PQRSTX(18x9)(3x2)TWO(5x7)SEVEN"), 445);
        }
    }

}