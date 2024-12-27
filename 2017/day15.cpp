//
// Created by Dan on 12/11/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/15
     */

    using state = std::array<uint64_t, 2>;

    uint64_t parse_line(std::string_view s) {
        const auto parts = split(s, ' ');
        return parse<uint64_t>(parts[4]);
    }

    state get_input(const std::vector<std::string>& lines) {
        return {parse_line(lines[0]), parse_line(lines[1])};
    }

    constexpr state FACTORS {16807u, 48271u};
    constexpr state PICKY {0xffffffff'fffffffcu, 0xffffffff'fffffff8u};
    constexpr uint64_t MODULUS = 2147483647ull;

    uint64_t judge(const state& s) {
        return static_cast<uint64_t>((s[0] & 0x0ffffu) == (s[1] & 0x0ffffu));
    }

    bool equals_mask(uint64_t v, uint64_t m) {
        return ((v & m) == v);
    }

    void increment_picky(state& s) {
        while (!equals_mask(s[0] = (s[0] * FACTORS[0]) % MODULUS, PICKY[0])) {}
        while (!equals_mask(s[1] = (s[1] * FACTORS[1]) % MODULUS, PICKY[1])) {}
    }

    void increment(state& s) {
        s[0] = (s[0] * FACTORS[0]) % MODULUS;
        s[1] = (s[1] * FACTORS[1]) % MODULUS;
    }

    uint64_t count_judgments(state s, const uint64_t steps) {
        uint64_t sum = 0;
        for (uint64_t i = 0; i < steps; ++i) {
            increment(s);
            sum += judge(s);
        }
        return sum;
    }

    uint64_t count_picky_judgments(state s, const uint64_t steps) {
        uint64_t sum = 0;
        for (uint64_t i = 0; i < steps; ++i) {
            increment_picky(s);
            sum += judge(s);
        }
        return sum;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto res = count_judgments(input, 40'000'000);
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto res = count_picky_judgments(input, 5'000'000);
        return std::to_string(res);
    }

    aoc::registration r{2017, 15, part_1, part_2};

//    TEST_SUITE("2017_day15") {
//        TEST_CASE("2017_day15:example") {
//
//        }
//    }

} /* namespace <anon> */