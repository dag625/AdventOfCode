//
// Created by Dan on 12/6/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/16
     */

    constexpr int P1_SIZE = 272;
    constexpr int P2_SIZE = 35651584;

    std::string get_input(const std::vector<std::string>& lines) {
        return lines.front();
    }

    std::string expand(std::string_view a) {
        std::string b {a.rbegin(), a.rend()};
        b = b | std::views::transform([](const char c){ return c == '0' ? '1' : '0'; }) | std::ranges::to<std::string>();
        return fmt::format("{}0{}", a, b);
    }

    std::string checksum_step(std::string_view s) {
        return s |
            std::views::chunk(2) |
            std::views::transform([](const auto& v){ return v[0] == v[1] ? '1' : '0'; }) |
            std::ranges::to<std::string>();
    }

    std::string checksum(std::string s) {
        do {
            s = checksum_step(s);
        } while (s.size() % 2 == 0);
        return s;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto data = get_input(lines);
        while (data.size() < P1_SIZE) {
            data = expand(data);
        }
        const auto cs = checksum(data.substr(0, P1_SIZE));
        return cs;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto data = get_input(lines);
        while (data.size() < P2_SIZE) {
            data = expand(data);
        }
        const auto cs = checksum(data.substr(0, P2_SIZE));
        return cs;
    }

    aoc::registration r{2016, 16, part_1, part_2};

//    TEST_SUITE("2016_day16") {
//        TEST_CASE("2016_day16:example") {
//
//        }
//    }

} /* namespace <anon> */