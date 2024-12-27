//
// Created by Dan on 12/6/2024.
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
     See https://adventofcode.com/2016/day/15
     */

    struct disk {
        int size = 0;
        int init_pos = 0;
    };

    disk parse_disk(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        return {parse<int>(parts[3]), parse<int>(parts[11].substr(0, parts[11].size() - 1))};
    }

    std::vector<disk> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_disk) | std::ranges::to<std::vector>();
    }

    bool falls_through(int time, const std::vector<disk>& disks) {
        for (int i = 0; i < disks.size(); ++i) {
            int pos = (disks[i].init_pos + time + i + 1) % disks[i].size;
            if (pos != 0) {
                return false;
            }
        }
        return true;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int time = 1;
        while (!falls_through(time, input)) {
            ++time;
        }
        return std::to_string(time);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        input.emplace_back(11, 0);
        int time = 1;
        while (!falls_through(time, input)) {
            ++time;
        }
        return std::to_string(time);
    }

    aoc::registration r{2016, 15, part_1, part_2};

//    TEST_SUITE("2016_day15") {
//        TEST_CASE("2016_day15:example") {
//
//        }
//    }

} /* namespace <anon> */