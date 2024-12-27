//
// Created by Dan on 12/13/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/13
     */

    struct point {
        int64_t x = 0;
        int64_t y = 0;
    };

    struct machine {
        point claw_a;
        point claw_b;
        point prize;

        [[nodiscard]] std::optional<int64_t> calc_cost(int64_t offset = 0) const {
            const point p {prize.x + offset, prize.y + offset};
            const auto num_b = (p.y * claw_a.x - claw_a.y * p.x) / (claw_a.x * claw_b.y - claw_b.x * claw_a.y);
            const auto num_a = (p.x - claw_b.x * num_b) / claw_a.x;
            const auto check_x = claw_a.x * num_a + claw_b.x * num_b;
            const auto check_y = claw_a.y * num_a + claw_b.y * num_b;
            if (check_x == p.x && check_y == p.y) {
                return 3 * num_a + num_b;
            }
            else {
                return std::nullopt;
            }
        }
    };

    machine parse_machine(std::string_view a, std::string_view b, std::string_view p) {
        auto ap = split(a, ' ');
        auto bp = split(b, ' ');
        auto pp = split(p, ' ');
        ap[2].remove_prefix(2);
        ap[3].remove_prefix(2);
        bp[2].remove_prefix(2);
        bp[3].remove_prefix(2);
        pp[1].remove_prefix(2);
        pp[2].remove_prefix(2);
        return {{parse<int64_t>(ap[2]), parse<int64_t>(ap[3])},
                {parse<int64_t>(bp[2]), parse<int64_t>(bp[3])},
                {parse<int64_t>(pp[1]), parse<int64_t>(pp[2])}};
    }

    std::vector<machine> to_machines(const std::vector<std::string>& lines) {
        return lines |
            std::views::filter([](const std::string& s){ return !s.empty(); }) |
            std::views::chunk(3) |
            std::views::transform([](const auto& c){ return parse_machine(*c.begin(), *std::next(c.begin(), 1), *std::next(c.begin(), 2)); }) |
            std::ranges::to<std::vector>();
    }

    std::vector<machine> get_input(const std::vector<std::string>& lines) {
        return to_machines(lines);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (const auto& m : input) {
            const auto cost = m.calc_cost();
            sum += cost.value_or(0);
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (const auto& m : input) {
            const auto cost = m.calc_cost(10000000000000ll);
            sum += cost.value_or(0);
        }
        return std::to_string(sum);
    }

    aoc::registration r{2024, 13, part_1, part_2};

    TEST_SUITE("2024_day13") {
        TEST_CASE("2024_day13:example") {
            const std::vector<std::string> lines {
                    "Button A: X+94, Y+34",
                    "Button B: X+22, Y+67",
                    "Prize: X=8400, Y=5400",
                    "",
                    "Button A: X+26, Y+66",
                    "Button B: X+67, Y+21",
                    "Prize: X=12748, Y=12176",
                    "",
                    "Button A: X+17, Y+86",
                    "Button B: X+84, Y+37",
                    "Prize: X=7870, Y=6450",
                    "",
                    "Button A: X+69, Y+23",
                    "Button B: X+27, Y+71",
                    "Prize: X=18641, Y=10279"
            };
            const auto input = to_machines(lines);
            int sum = 0;
            for (const auto& m : input) {
                const auto cost = m.calc_cost();
                sum += cost.value_or(0);
            }
            CHECK_EQ(sum, 480);
        }
    }

} /* namespace <anon> */