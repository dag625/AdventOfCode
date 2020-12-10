//
// Created by Daniel Garcia on 12/9/20.
//

#include "day9.h"
#include "utilities.h"

#include <cstdint>
#include <charconv>
#include <numeric>
#include <iostream>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        int64_t parse_int64(std::string_view s) noexcept {
            int64_t retval = -1;
            std::from_chars(s.data(), s.data() + s.size(), retval);
            //No error checking, like a boss!
            return retval;
        }

        std::vector<int64_t> get_input(const fs::path &input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_9_input.txt");
            std::vector<int64_t> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_int64);
            return retval;
        }

        constexpr std::size_t WINDOW = 25;
        using iter = std::vector<int64_t>::const_iterator;

        bool is_valid(iter i, const iter end) {
            for (; i != end; ++i) {
                for (auto j = i + 1; j != end; ++j) {
                    if (*i + *j == *end) {
                        return true;
                    }
                }
            }
            return false;
        }

        int64_t find_first_invalid(const std::vector<int64_t> &data) noexcept {
            if (data.size() < WINDOW + 1) {
                return -1;
            }
            const auto end = data.end();
            for (auto curr = data.begin(); curr + WINDOW != end; ++curr) {
                auto val = curr + WINDOW;
                if (!is_valid(curr, val)) {
                    return *val;
                }
            }
            return -1;
        }

        std::pair<int64_t, int64_t> min_max(iter begin, const iter end) noexcept {
            int64_t min = std::numeric_limits<int64_t>::max(),
                    max = std::numeric_limits<int64_t>::min();
            for (; begin != end; ++begin) {
                min = std::min(*begin, min);
                max = std::max(*begin, max);
            }
            return {min, max};
        }

        int64_t find_sum_range(const std::vector<int64_t> &data, int64_t sum) {
            const auto end = data.end();
            for (auto start = data.begin(); start != end; ++start) {
                for (auto stop = start + 2; stop != end; ++stop) {
                    if (sum == std::accumulate(start, stop, 0LL)) {
                        auto[min, max] = min_max(start, stop);
                        return min + max;
                    }
                }
            }
            return -1;
        }

    }

    /*
    With your neighbor happily enjoying their video game, you turn your attention to an open data port on the little screen in the seat in front of you.

    Though the port is non-standard, you manage to connect it to your computer through the clever use of several paperclips. Upon connection, the port outputs a series of numbers (your puzzle input).

    The data appears to be encrypted with the eXchange-Masking Addition System (XMAS) which, conveniently for you, is an old cypher with an important weakness.

    XMAS starts by transmitting a preamble of 25 numbers. After that, each number you receive should be the sum of any two of the 25 immediately previous numbers. The two numbers will have different values, and there might be more than one such pair.

    For example, suppose your preamble consists of the numbers 1 through 25 in a random order. To be valid, the next number must be the sum of two of those numbers:

    26 would be a valid next number, as it could be 1 plus 25 (or many other pairs, like 2 and 24).
    49 would be a valid next number, as it is the sum of 24 and 25.
    100 would not be valid; no two of the previous 25 numbers sum to 100.
    50 would also not be valid; although 25 appears in the previous 25 numbers, the two numbers in the pair must be different.
    Suppose the 26th number is 45, and the first number (no longer an option, as it is more than 25 numbers ago) was 20. Now, for the next number to be valid, there needs to be some pair of numbers among 1-19, 21-25, or 45 that add up to it:

    26 would still be a valid next number, as 1 and 25 are still within the previous 25 numbers.
    65 would not be valid, as no two of the available numbers sum to it.
    64 and 66 would both be valid, as they are the result of 19+45 and 21+45 respectively.
    Here is a larger example which only considers the previous 5 numbers (and has a preamble of length 5):

    35
    20
    15
    25
    47
    40
    62
    55
    65
    95
    102
    117
    150
    182
    127
    219
    299
    277
    309
    576
    In this example, after the 5-number preamble, almost every number is the sum of two of the previous 5 numbers; the only number that does not follow this rule is 127.

    The first step of attacking the weakness in the XMAS data is to find the first number in the list (after the preamble) which is not the sum of two of the 25 numbers before it. What is the first number that does not have this property?
    */
    void solve_day_9_1(const std::filesystem::path& input_dir) {
        std::cout << '\t' << find_first_invalid(get_input(input_dir)) << '\n';
    }

    /*
    The final step in breaking the XMAS encryption relies on the invalid number you just found: you must find a contiguous set of at least two numbers in your list which sum to the invalid number from step 1.

    Again consider the above example:

    35
    20
    15
    25
    47
    40
    62
    55
    65
    95
    102
    117
    150
    182
    127
    219
    299
    277
    309
    576
    In this list, adding up all of the numbers from 15 through 40 produces the invalid number from step 1, 127. (Of course, the contiguous set of numbers in your actual list might be much longer.)

    To find the encryption weakness, add together the smallest and largest number in this contiguous range; in this example, these are 15 and 47, producing 62.

    What is the encryption weakness in your XMAS-encrypted list of numbers?
    */
    void solve_day_9_2(const std::filesystem::path& input_dir) {
        auto data = get_input(input_dir);
        std::cout << '\t' << find_sum_range(data, find_first_invalid(data)) << '\n';
    }

} /* namespace aoc2020 */