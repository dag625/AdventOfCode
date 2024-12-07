//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <vector>
#include <numeric>
#include <iostream>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/7
     */

    struct equation {
        int64_t result;
        std::vector<int64_t> values;
    };

    equation parse_eq(std::string_view s) {
        const auto p1 = split_no_empty(s, ':');
        const auto p2 = split_no_empty(p1[1], ' ');
        return {parse<int64_t>(p1[0]),
                p2 | std::views::transform([](std::string_view v){ return parse<int64_t>(v); }) | std::ranges::to<std::vector>()};
    }

    std::vector<equation> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_7_input.txt");
        return lines | std::views::transform(&parse_eq) | std::ranges::to<std::vector>();
    }

    constexpr int64_t add(const int64_t l, const int64_t r) noexcept { return l + r; }
    constexpr int64_t mul(const int64_t l, const int64_t r) noexcept { return l * r; }
    constexpr int64_t pow10_calc(const int64_t r) noexcept { int64_t ret = 10; while (ret <= r) { ret *= 10; } return ret; }
    constexpr int64_t concat(const int64_t l, const int64_t r) noexcept { return l * pow10_calc(r) + r; } //{ return l * static_cast<int64_t>(std::pow(10.0, std::ceil(std::log10(static_cast<double>(r))))) + r; }

    using func = int64_t(*)(int64_t, int64_t) noexcept;

    constexpr std::array<func, 2> OPS1 {&add, &mul};
    constexpr std::array<func, 3> OPS2 {&add, &mul, &concat};

    template <size_t N>
    int64_t calcN(const equation& e, uint32_t ops, const std::array<func, N>& funcs) {
        int64_t retval = e.values.front();
        for (const auto v : e.values | std::views::drop(1)) {
            uint32_t idx = ops % N;
            ops /= N;
            retval = funcs[idx](retval, v);
        }
        return retval;
    }

    template <size_t N>
    bool can_do_with_ops(const equation& e, const std::array<func, N>& funcs) {
        const auto max = static_cast<uint32_t>(std::pow(static_cast<double>(N), e.values.size()));
        for (uint32_t val = 0; val < max; ++val) {
            const auto res = calcN(e, val, funcs);
            if (res == e.result) {
                return true;
            }
        }
        return false;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto vals = input | std::views::filter([](const equation& e){ return can_do_with_ops(e, OPS1); }) |
                          std::views::transform([](const equation& e){ return e.result; }) |
                          std::ranges::to<std::vector>();
        const auto sum = std::accumulate(vals.begin(), vals.end(), 0ll);
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto vals = input | std::views::filter([](const equation& e){ return can_do_with_ops(e, OPS2); }) |
                          std::views::transform([](const equation& e){ return e.result; }) |
                          std::ranges::to<std::vector>();
        const auto sum = std::accumulate(vals.begin(), vals.end(), 0ll);
        return std::to_string(sum);
    }

    aoc::registration r{2024, 7, part_1, part_2};

    TEST_SUITE("2024_day7") {
        TEST_CASE("2024_day7:example") {
            const std::vector<equation> input {
                    {190, {10, 19}},
                    {3267, {81, 40, 27}},
                    {83, {17, 5}},
                    {156, {15, 6}},
                    {7290, {6, 8, 6, 15}},
                    {161011, {16, 10, 13}},
                    {192, {17, 8, 14}},
                    {21037, {9, 7, 18, 13}},
                    {292, {11, 6, 16, 20}}
            };
            const auto vals = input | std::views::filter([](const equation& e){ return can_do_with_ops(e, OPS2); }) |
                              std::views::transform([](const equation& e){ return e.result; }) |
                              std::ranges::to<std::vector>();
            const auto sum = std::accumulate(vals.begin(), vals.end(), 0ll);
            CHECK_EQ(sum, 11387);
            CHECK_EQ(concat(10, 20), 1020);
            CHECK_EQ(concat(12345678900ll, 987654321ll), 12345678900987654321ll);
        }
    }

} /* namespace <anon> */