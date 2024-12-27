//
// Created by Dan on 12/18/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2018/day/5
     */

    std::string get_input(const std::vector<std::string>& lines) {
        return lines.front();
    }

    bool collapse_one(std::string& s) {
        for (int i = 1; i < s.size(); ++i) {
            const auto diff = std::abs(s[i] - s[i-1]);
            if (diff == 32) { //Upper and lower case ASCII are 32/0x20 apart.
                s.erase(i - 1, 2);
                return true;
            }
        }
        return false;
    }

    std::string collapse(std::string s) {
        while (collapse_one(s)) {}
        return s;
    }

    int remove_and_collapse(std::string s, const char rem) {
        s.erase(std::remove_if(s.begin(), s.end(), [rem](const char c){ return tolower(c) == rem; }), s.end());
        const auto res = collapse(s);
        return static_cast<int>(res.size());
    }

    int find_best(const std::string& s) {
        int best = std::numeric_limits<int>::max();
        for (char rem = 'a'; rem <= 'z'; ++rem) {
            const auto len = remove_and_collapse(s, rem);
            if (len < best) {
                best = len;
            }
        }
        return best;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto reacted = collapse(input);
        return std::to_string(reacted.size());
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto best_len = find_best(input);
        return std::to_string(best_len);
    }

    aoc::registration r{2018, 5, part_1, part_2};

    TEST_SUITE("2018_day05") {
        TEST_CASE("2018_day05:example") {
            const std::string input = "dabAcCaCBAcCcaDA";
            const auto reacted = collapse(input);
            CHECK_EQ(reacted.size(), 10);
            CHECK_EQ(reacted, "dabCBAcaDA");
        }
    }

} /* namespace <anon> */