//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/10
     */

    std::pair<std::vector<int>,std::string> get_input(const std::vector<std::string>& lines) {
        const auto parts = split(lines.front(), ',');
        return {parts | std::views::transform([](std::string_view s){ return parse<int>(s); }) | std::ranges::to<std::vector>(), lines.front()};
    }

    void do_reverse(std::vector<int>& list, const int pos, const int len) {
        const int size = static_cast<int>(list.size());
        const int end = (pos + len) % size;
        if (end >= pos) {
            std::reverse(list.begin() + pos, list.begin() + end);
        }
        else {
            std::vector<int> buf;
            buf.reserve(len);
            buf.insert(buf.end(), list.begin() + pos, list.end());
            buf.insert(buf.end(), list.begin(), list.begin() + end);
            std::reverse(buf.begin(), buf.end());
            int lp = pos;
            for (const int v : buf) {
                list[lp] = v;
                lp = (lp + 1) % size;
            }
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [input, ignored] = get_input(lines);
        auto list = std::views::iota(0, 256) | std::ranges::to<std::vector>();
        const auto size = static_cast<int>(list.size());
        int pos = 0, skip = 0;
        for (const auto len : input) {
            do_reverse(list, pos, len);
            pos = (pos + len + skip) % size;
            ++skip;
        }
        return std::to_string(list[0] * list[1]);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [ignored, raw_input] = get_input(lines);
        auto input = raw_input | std::views::transform([](const char c){ return static_cast<int>(c); }) | std::ranges::to<std::vector>();
        input.push_back(17);
        input.push_back(31);
        input.push_back(73);
        input.push_back(47);
        input.push_back(23);
        auto list = std::views::iota(0, 256) | std::ranges::to<std::vector>();
        const auto size = static_cast<int>(list.size());
        int pos = 0, skip = 0;
        for (int i = 0; i < 64; ++i) {
            for (const auto len : input) {
                do_reverse(list, pos, len);
                pos = (pos + len + skip) % size;
                ++skip;
            }
        }

        std::string hex;
        for (const auto grp : list | std::views::chunk(16)) {
            uint8_t v = 0;
            for (const auto i : grp) {
                v = v ^ static_cast<uint8_t>(i);
            }
            hex = fmt::format("{}{:02x}", hex, v);
        }
        return hex;
    }

    aoc::registration r{2017, 10, part_1, part_2};

//    TEST_SUITE("2017_day10") {
//        TEST_CASE("2017_day10:example") {
//
//        }
//    }

} /* namespace <anon> */