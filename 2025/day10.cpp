//
// Created by Dan on 12/10/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <bitset>
#include <bit>
#include <iostream>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace {
    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/10
     */

    using light_t = decltype(std::declval<std::bitset<16>>().to_ulong());

    struct indicator {
        light_t desired;
        std::vector<light_t> toggles;
        std::vector<int> joltages;

        indicator() = default;
        indicator(std::string_view l, std::vector<light_t> t, std::vector<int> j) :
            toggles{std::move(t)}, joltages{std::move(j)}
        {
            std::bitset<16> lights;
            for (int i = 0; i < lights.size() && i < l.size(); ++i) {
                if (l[i] == '#') {
                    lights.set(i);
                }
            }
            desired = lights.to_ulong();
        }
    };

    std::string_view next_between(std::string_view s, int& offset, const char begin, const char end) {
        auto retval = s;
        const auto start = retval.find(begin, offset);
        const auto finish = retval.find(end, offset);
        if (start == std::string_view::npos || finish == std::string_view::npos) {
            offset = static_cast<int>(s.size());
            return {};
        }
        retval = retval.substr(start + 1, finish - start - 1);
        offset = static_cast<int>(finish + 1);
        return retval;
    }

    std::string_view next_between(std::string_view s, const char begin, const char end) {
        int offset = 0;
        return next_between(s, offset, begin, end);
    }

    std::vector<int> comma_sep_to_vec(std::string_view s) {
        const auto parts = split(s, ',');
        return parts | std::views::transform(&parse32) | std::ranges::to<std::vector>();
    }

    indicator parse_indicator(std::string_view str) {
        const auto lights_str = next_between(str, '[', ']');
        const auto jolts_str = next_between(str, '{', '}');

        int offset = 0;
        std::vector<light_t> toggles;
        while (offset < str.size()) {
            const auto toggle_str = next_between(str, offset, '(', ')');
            if (offset < str.size()) {
                const auto idxs = comma_sep_to_vec(toggle_str);
                std::bitset<16> lights;
                for (const int i : idxs) {
                    lights.set(i);
                }
                toggles.push_back(lights.to_ulong());
            }
        }

        return {std::string{lights_str}, std::move(toggles), comma_sep_to_vec(jolts_str)};
    }

    std::vector<indicator> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_indicator) | std::ranges::to<std::vector>();
    }

    int find_match_steps(const indicator& ind) {
        static constexpr std::array<light_t, 2> MASKS {0x0u, 0xffffffffu};
        const auto max = 1u << ind.toggles.size();
        int min = std::numeric_limits<int>::max();
        for (unsigned t = 1; t < max; ++t) {
            std::bitset<16> lights {t};
            light_t val = 0u;
            for (int i = 0; i < ind.toggles.size(); ++i) {
                const auto& a = MASKS[(int)lights[i]];
                const auto& b = ind.toggles[i];
                val ^= a & b;
            }
            const auto num = std::popcount(t);
            if (val == ind.desired && num < min) {
                min = num;
            }
        }
        return min;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int sum = 0;
        for (const auto& ind : input) {
            const auto steps = find_match_steps(ind);
            sum += steps;
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);

        return std::to_string(-1);
    }

    aoc::registration r {2025, 10, part_1, part_2};

    TEST_SUITE("2025_day10") {
        TEST_CASE("2025_day10:example") {
            const std::vector<std::string> lines {
                "[.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}",
                "[...#.] (0,2,3,4) (2,3) (0,4) (0,1,2) (1,2,3,4) {7,5,12,7,2}",
                "[.###.#] (0,1,2,3,4) (0,3,4) (0,1,2,4,5) (1,2) {10,11,11,5,10,5}"
            };
            const auto input = get_input(lines);
            const auto steps0 = find_match_steps(input[0]);
            CHECK_EQ(steps0, 2);
        }
    }

} /* namespace <anon> */