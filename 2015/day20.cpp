//
// Created by Dan on 7/2/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <cmath>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr uint64_t MIN_NUM_PRESENTS = 29000000;
    constexpr uint64_t PRESENTS_PER_ELF = 10;
    constexpr uint64_t MIN_SUM_OF_ALL_FACTORS = MIN_NUM_PRESENTS / PRESENTS_PER_ELF;

    uint64_t sum_of_all_factors(uint64_t v) {
        uint64_t retval = 1;
        //Combination of:
        //https://planetmath.org/formulaforsumofdivisors
        //https://www.geeksforgeeks.org/print-all-prime-factors-of-a-given-number/

        //2 is the only even prime
        int num_2s = 0;
        while (v % 2 == 0) {
            ++num_2s;
            v /= 2;
        }
        if (num_2s > 0) {
            retval *= ((2 << num_2s) - 1);
        }

        //All the odd primes
        for (uint64_t p = 3; static_cast<double>(p) <= std::sqrt(v); p += 2) {
            int num_ps = 0;
            while (v % p == 0) {
                ++num_ps;
                v /= p;
            }
            if (num_ps > 0) {
                retval *= (static_cast<uint64_t>(std::pow(p, num_ps + 1)) - 1) / (p - 1);
            }
        }

        //If the original v was prime
        if (v > 2) {
            retval *= (v * v - 1) / (v - 1);
        }
        return retval;
    }

    uint64_t total_elf_num_count(uint64_t n, const uint64_t max_factor = std::numeric_limits<uint64_t>::max()) {
        uint64_t retval = 0;
        const auto sqrt_val = std::sqrt(n);
        for (uint64_t elf = 1; static_cast<double>(elf) <= sqrt_val; ++elf) {
            if (n % elf == 0) {
                const auto factor = n / elf;
                if (factor <= max_factor) {
                    retval += elf;
                }
                if (elf <= max_factor) {
                    retval += factor;
                }
            }
        }
        return retval;
    }

    /*
    --- Day 20: Infinite Elves and Infinite Houses ---
    To keep the Elves busy, Santa has them deliver some presents by hand, door-to-door. He sends them down a street with infinite houses numbered sequentially: 1, 2, 3, 4, 5, and so on.

    Each Elf is assigned a number, too, and delivers presents to houses based on that number:

    The first Elf (number 1) delivers presents to every house: 1, 2, 3, 4, 5, ....
    The second Elf (number 2) delivers presents to every second house: 2, 4, 6, 8, 10, ....
    Elf number 3 delivers presents to every third house: 3, 6, 9, 12, 15, ....
    There are infinitely many Elves, numbered starting with 1. Each Elf delivers presents equal to ten times his or her number at each house.

    So, the first nine houses on the street end up like this:

    House 1 got 10 presents.
    House 2 got 30 presents.
    House 3 got 40 presents.
    House 4 got 70 presents.
    House 5 got 60 presents.
    House 6 got 120 presents.
    House 7 got 80 presents.
    House 8 got 150 presents.
    House 9 got 130 presents.
    The first house gets 10 presents: it is visited only by Elf 1, which delivers 1 * 10 = 10 presents. The fourth house gets 70 presents, because it is visited by Elves 1, 2, and 4, for a total of 10 + 20 + 40 = 70 presents.

    What is the lowest house number of the house to get at least as many presents as the number in your puzzle input?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        uint64_t val = 1;
        for (; val < MIN_SUM_OF_ALL_FACTORS; ++val) {
            uint64_t sum = sum_of_all_factors(val);
            if (sum >= MIN_SUM_OF_ALL_FACTORS) {
                break;
            }
        }
        return std::to_string(val);
    }

    /*
    --- Part Two ---
    The Elves decide they don't want to visit an infinite number of houses. Instead, each Elf will stop after delivering presents to 50 houses. To make up for it, they decide to deliver presents equal to eleven times their number at each house.

    With these changes, what is the new lowest house number of the house to get at least as many presents as the number in your puzzle input?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        uint64_t val = 1;
        //Do part 1 because part 2 should be larger (we're removing presents from earlier elves).
        for (; val < MIN_SUM_OF_ALL_FACTORS; ++val) {
            uint64_t sum = sum_of_all_factors(val);
            if (sum >= MIN_SUM_OF_ALL_FACTORS) {
                break;
            }
        }
        for (; val < MIN_SUM_OF_ALL_FACTORS; ++val) {
            uint64_t sum = total_elf_num_count(val, 50) * 11;
            if (sum >= MIN_NUM_PRESENTS) {
                break;
            }
        }
        return std::to_string(val);
    }

    aoc::registration r{2015, 20, part_1, part_2};

//    TEST_SUITE("2015_day20") {
//        TEST_CASE("2015_day20:example") {
//
//        }
//    }

}