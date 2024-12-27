//
// Created by Dan on 12/6/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <set>

#include "utilities.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    std::string get_input(const std::vector<std::string>& lines) {
        return lines | std::views::join | std::ranges::to<std::string>();
    }

    bool is_start_of_packet(std::string_view s, const int start_size) {
        std::string t {s.begin(), s.begin() + start_size};
        std::sort(t.begin(), t.end());
        t.erase(std::unique(t.begin(), t.end()), t.end());
        return t.size() == start_size;
    }

    int find_first_start_of_packet(std::string_view s, const int start_size) {
        for (int idx = 0; idx < s.size() - start_size; ++idx) {
            if (is_start_of_packet(s.substr(idx), start_size)) {
                return idx + start_size;
            }
        }
        return -1;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        return std::to_string(find_first_start_of_packet(input, 4));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        return std::to_string(find_first_start_of_packet(input, 14));
    }

    aoc::registration r{2022, 6, part_1, part_2};

    TEST_SUITE("2022_day06") {
        TEST_CASE("2022_day06:example") {
            const std::string input = "mjqjpqmgbljsphdztnvjfqwrcgsmlb";
            CHECK_EQ(find_first_start_of_packet(input, 4), 7);
            CHECK_EQ(find_first_start_of_packet(input, 14), 19);
        }
    }

}