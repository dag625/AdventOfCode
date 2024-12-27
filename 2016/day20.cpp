//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/20
     */

    using range = std::pair<uint32_t, uint32_t>;

    range parse_range(std::string_view s) {
        const auto parts = split(s, '-');
        return {parse<uint32_t>(parts[0]), parse<uint32_t>(parts[1])};
    }

    std::vector<range> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_range) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        std::sort(input.begin(), input.end());
        uint32_t addr = 0;
        for (const auto [first, last] : input) {
            if (addr >= first && addr <= last) {
                addr = last + 1;
            }
            else if (addr < first) {
                break;
            }
        }
        return std::to_string(addr);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        std::sort(input.begin(), input.end());
        uint32_t addr = 0, total = 0;
        for (const auto [first, last] : input) {
            if (addr >= first && addr <= last) {
                addr = last + 1;
            }
            else if (addr == 0) {
                break;
            }
            else if (addr < first) {
                const auto del = first - addr;
                total += del;
                addr = last + 1;
            }
        }
        if (addr != 0) {
            total += 0xffffffffu - addr + 1;
        }
        return std::to_string(total);
    }

    aoc::registration r{2016, 20, part_1, part_2};

//    TEST_SUITE("2016_day20") {
//        TEST_CASE("2016_day20:example") {
//
//        }
//    }

} /* namespace <anon> */