//
// Created by Dan on 12/3/2021.
//

#include "registration.h"

#include <vector>
#include <iostream>
#include <string_view>
#include <array>
#include <algorithm>
#include <ranges>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int INPUT_LEN = 12;

    std::vector<std::string> get_input(const fs::path& input_dir) {
        return read_file_lines(input_dir / "2021" / "day_3_input.txt");
    }

    using iter = std::vector<std::string>::const_iterator;

    template <typename P>
    void find_bit_criteria(const std::vector<std::string>& data, std::string& prefix, const char tie, P use_ones) {
        auto range = data |
                     std::views::filter([&prefix](const std::string& s){ return s.starts_with(prefix); });
        const auto num = std::ranges::distance(range);
        if (num == 1) {
            prefix = range.front();
            return;
        }

        const auto one_count = std::ranges::count_if(range,
                                                     [idx = prefix.size()](const std::string& s){ return s[idx] == '1'; });
        if (use_ones(one_count, num)) {
            prefix += '1';
        }
        else if (num % 2 == 0 && one_count == num / 2) {
            prefix += tie;
        }
        else {
            prefix += '0';
        }

        find_bit_criteria(data, prefix, tie, use_ones);
    }

    /*
    --- Day 3: Binary Diagnostic ---
    The submarine has been making some odd creaking noises, so you ask it to produce a diagnostic report just in case.

    The diagnostic report (your puzzle input) consists of a list of binary numbers which, when decoded properly, can tell you many useful things about the conditions of the submarine. The first parameter to check is the power consumption.

    You need to use the binary numbers in the diagnostic report to generate two new binary numbers (called the gamma rate and the epsilon rate). The power consumption can then be found by multiplying the gamma rate by the epsilon rate.

    Each bit in the gamma rate can be determined by finding the most common bit in the corresponding position of all numbers in the diagnostic report. For example, given the following diagnostic report:

    00100
    11110
    10110
    10111
    10101
    01111
    00111
    11100
    10000
    11001
    00010
    01010
    Considering only the first bit of each number, there are five 0 bits and seven 1 bits. Since the most common bit is 1, the first bit of the gamma rate is 1.

    The most common second bit of the numbers in the diagnostic report is 0, so the second bit of the gamma rate is 0.

    The most common value of the third, fourth, and fifth bits are 1, 1, and 0, respectively, and so the final three bits of the gamma rate are 110.

    So, the gamma rate is the binary number 10110, or 22 in decimal.

    The epsilon rate is calculated in a similar way; rather than use the most common bit, the least common bit from each position is used. So, the epsilon rate is 01001, or 9 in decimal. Multiplying the gamma rate (22) by the epsilon rate (9) produces the power consumption, 198.

    Use the binary numbers in your diagnostic report to calculate the gamma rate and epsilon rate, then multiply them together. What is the power consumption of the submarine? (Be sure to represent your answer in decimal, not binary.)
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        std::array<int, INPUT_LEN> counts{};
        for (const auto& s : input) {
            for (int i = 0; i < INPUT_LEN; ++i) {
                counts[i] += s[i] == '1' ? 1 : 0;
            }
        }
        const int num = static_cast<int>(input.size()) / 2;
        uint32_t gamma = 0, epsilon = 0, mask = 0;
        for (auto c : counts) {
            gamma <<= 1u;
            mask <<= 1u;
            gamma |= (c > num ? 1u : 0u);
            mask |= 1u;
        }
        epsilon = (~gamma) & mask;
        return std::to_string(gamma * epsilon);
    }

    /*
    --- Part Two ---
    Next, you should verify the life support rating, which can be determined by multiplying the oxygen generator rating by the CO2 scrubber rating.

    Both the oxygen generator rating and the CO2 scrubber rating are values that can be found in your diagnostic report - finding them is the tricky part. Both values are located using a similar process that involves filtering out values until only one remains. Before searching for either rating value, start with the full list of binary numbers from your diagnostic report and consider just the first bit of those numbers. Then:

    Keep only numbers selected by the bit criteria for the type of rating value for which you are searching. Discard numbers which do not match the bit criteria.
    If you only have one number left, stop; this is the rating value for which you are searching.
    Otherwise, repeat the process, considering the next bit to the right.
    The bit criteria depends on which type of rating value you want to find:

    To find oxygen generator rating, determine the most common value (0 or 1) in the current bit position, and keep only numbers with that bit in that position. If 0 and 1 are equally common, keep values with a 1 in the position being considered.
    To find CO2 scrubber rating, determine the least common value (0 or 1) in the current bit position, and keep only numbers with that bit in that position. If 0 and 1 are equally common, keep values with a 0 in the position being considered.
    For example, to determine the oxygen generator rating value using the same example diagnostic report from above:

    Start with all 12 numbers and consider only the first bit of each number. There are more 1 bits (7) than 0 bits (5), so keep only the 7 numbers with a 1 in the first position: 11110, 10110, 10111, 10101, 11100, 10000, and 11001.
    Then, consider the second bit of the 7 remaining numbers: there are more 0 bits (4) than 1 bits (3), so keep only the 4 numbers with a 0 in the second position: 10110, 10111, 10101, and 10000.
    In the third position, three of the four numbers have a 1, so keep those three: 10110, 10111, and 10101.
    In the fourth position, two of the three numbers have a 1, so keep those two: 10110 and 10111.
    In the fifth position, there are an equal number of 0 bits and 1 bits (one each). So, to find the oxygen generator rating, keep the number with a 1 in that position: 10111.
    As there is only one number left, stop; the oxygen generator rating is 10111, or 23 in decimal.
    Then, to determine the CO2 scrubber rating value from the same example above:

    Start again with all 12 numbers and consider only the first bit of each number. There are fewer 0 bits (5) than 1 bits (7), so keep only the 5 numbers with a 0 in the first position: 00100, 01111, 00111, 00010, and 01010.
    Then, consider the second bit of the 5 remaining numbers: there are fewer 1 bits (2) than 0 bits (3), so keep only the 2 numbers with a 1 in the second position: 01111 and 01010.
    In the third position, there are an equal number of 0 bits and 1 bits (one each). So, to find the CO2 scrubber rating, keep the number with a 0 in that position: 01010.
    As there is only one number left, stop; the CO2 scrubber rating is 01010, or 10 in decimal.
    Finally, to find the life support rating, multiply the oxygen generator rating (23) by the CO2 scrubber rating (10) to get 230.

    Use the binary numbers in your diagnostic report to calculate the oxygen generator rating and CO2 scrubber rating, then multiply them together. What is the life support rating of the submarine? (Be sure to represent your answer in decimal, not binary.)
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        std::string ox_gen_rating_str, co2_scrub_rating_str;
        find_bit_criteria(input, ox_gen_rating_str, '1', [](int64_t num_ones, int64_t total){ return num_ones > total / 2; });
        find_bit_criteria(input, co2_scrub_rating_str, '0', [](int64_t num_ones, int64_t total){ return num_ones < total / 2 || (total % 2 == 1 && num_ones == total / 2); });
        int ox_gen_rating = 0, co2_scrub_rating = 0;
        auto res1 = std::from_chars(ox_gen_rating_str.data(), ox_gen_rating_str.data() + ox_gen_rating_str.size(), ox_gen_rating, 2);
        auto res2 = std::from_chars(co2_scrub_rating_str.data(), co2_scrub_rating_str.data() + co2_scrub_rating_str.size(), co2_scrub_rating, 2);
        const std::errc no_err{};
        if (res1.ec != no_err) {
            throw std::system_error{std::make_error_code(res1.ec)};
        }
        else if (res2.ec != no_err) {
            throw std::system_error{std::make_error_code(res2.ec)};
        }
        return std::to_string(ox_gen_rating * co2_scrub_rating);
    }

    aoc::registration r {2021, 3, part_1, part_2};

}