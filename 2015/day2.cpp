//
// Created by Daniel Garcia on 12/25/20.
//

#include "registration.h"
#include "utilities.h"

#include <doctest/doctest.h>

#include <vector>
#include <filesystem>
#include <string_view>
#include <algorithm>
#include <charconv>
#include <array>
#include <numeric>

namespace {

    struct size {
        int w = 0;
        int h = 0;
        int d = 0;
    };

    size parse_size(std::string_view line) {
        auto parts = aoc::split(line, 'x');
        size retval{};
        auto res = std::from_chars(parts[0].data(), parts[0].data()+ parts[0].size(), retval.w);
        if (res.ec != std::errc{}) {
            throw std::system_error{std::make_error_code(res.ec)};
        }
        res = std::from_chars(parts[1].data(), parts[1].data()+ parts[1].size(), retval.h);
        if (res.ec != std::errc{}) {
            throw std::system_error{std::make_error_code(res.ec)};
        }
        res = std::from_chars(parts[2].data(), parts[2].data()+ parts[2].size(), retval.d);
        if (res.ec != std::errc{}) {
            throw std::system_error{std::make_error_code(res.ec)};
        }
        return retval;
    }

    std::vector<size> get_input(const std::filesystem::path& input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2015" / "day_2_input.txt");
        std::vector<size> retval;
        retval.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_size);
        return retval;
    }

    int calculate_area(size s) {
        std::array<int, 3> sides { s.w * s.h, s.w * s.d, s.h * s.d };
        auto smallest = *std::min_element(sides.begin(), sides.end());
        return smallest + 2 * sides[0] + 2 * sides[1] + 2 * sides[2];
    }

    int calculate_length(size s) {
        int s1 = 0, s2 = 0;
        if (s.d >= s.h && s.d >= s.w) {
            s1 = s.h;
            s2 = s.w;
        }
        else if (s.h >= s.d && s.h >= s.w) {
            s1 = s.d;
            s2 = s.w;
        }
        else {
            s1 = s.h;
            s2 = s.d;
        }
        return 2 * s1 + 2 * s2 + s.w * s.h * s.d;
    }

    /*
    --- Day 2: I Was Told There Would Be No Math ---

    The elves are running low on wrapping paper, and so they need to submit an order for more. They have a list of the dimensions (length l, width w, and height h) of each present, and only want to order exactly as much as they need.

    Fortunately, every present is a box (a perfect right rectangular prism), which makes calculating the required wrapping paper for each gift a little easier: find the surface area of the box, which is 2*l*w + 2*w*h + 2*h*l. The elves also need a little extra paper for each present: the area of the smallest side.

    For example:

    A present with dimensions 2x3x4 requires 2*6 + 2*12 + 2*8 = 52 square feet of wrapping paper plus 6 square feet of slack, for a total of 58 square feet.
    A present with dimensions 1x1x10 requires 2*1 + 2*10 + 2*10 = 42 square feet of wrapping paper plus 1 square foot of slack, for a total of 43 square feet.
    All numbers in the elves' list are in feet. How many total square feet of wrapping paper should they order?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        auto sizes = get_input(input_dir);
        return std::to_string(std::accumulate(sizes.begin(), sizes.end(), 0, [](int acc, size s){ return acc + calculate_area(s); }));
    }

    /*
    The elves are also running low on ribbon. Ribbon is all the same width, so they only have to worry about the length they need to order, which they would again like to be exact.

    The ribbon required to wrap a present is the shortest distance around its sides, or the smallest perimeter of any one face. Each present also requires a bow made out of ribbon as well; the feet of ribbon required for the perfect bow is equal to the cubic feet of volume of the present. Don't ask how they tie the bow, though; they'll never tell.

    For example:

    A present with dimensions 2x3x4 requires 2+2+3+3 = 10 feet of ribbon to wrap the present plus 2*3*4 = 24 feet of ribbon for the bow, for a total of 34 feet.
    A present with dimensions 1x1x10 requires 1+1+1+1 = 4 feet of ribbon to wrap the present plus 1*1*10 = 10 feet of ribbon for the bow, for a total of 14 feet.
    How many total feet of ribbon should they order?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto sizes = get_input(input_dir);
        return std::to_string(std::accumulate(sizes.begin(), sizes.end(), 0, [](int acc, size s){ return acc + calculate_length(s); }));
    }

    aoc::registration r {2015, 2, part_1, part_2};

    TEST_SUITE("2015_day2") {
        TEST_CASE("2015_day2:example") {
            REQUIRE_EQ(calculate_area({2,3,4}), 58);
            REQUIRE_EQ(calculate_area({1,1,10}), 43);
            REQUIRE_EQ(calculate_length({2,3,4}), 34);
            REQUIRE_EQ(calculate_length({1,1,10}), 14);
        }
    }

}