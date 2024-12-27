//
// Created by Daniel Garcia on 4/27/21.
//

#include "registration.h"
#include "utilities.h"

#include <doctest/doctest.h>

#include <string_view>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    bool is_nice_part1(const std::string& s) {
        constexpr auto VOWELS = "aeiou"sv;
        const auto num_vowels = std::count_if(s.begin(), s.end(),
                                              [&VOWELS](char c){ return VOWELS.find(c) != std::string_view::npos; });
        const bool has_bad_subs = s.find("ab") != std::string::npos ||
                s.find("cd") != std::string::npos ||
                s.find("pq") != std::string::npos ||
                s.find("xy") != std::string::npos;
        bool has_double_char = false;
        char last = '\0';
        for (char c : s) {
            if (c == last) {
                has_double_char = true;
                break;
            }
            last = c;
        }
        return num_vowels >= 3 && has_double_char && !has_bad_subs;
    }

    bool is_nice_part2(const std::string& s) {
        bool has_double_char = false;
        char last = '\0', mid = '\0';
        for (char c : s) {
            if (c == last) {
                has_double_char = true;
                break;
            }
            last = mid;
            mid = c;
        }
        bool has_repeated = false;
        std::string_view view {s};
        while (view.size() > 1) {
            auto candidate = view.substr(0, 2);
            auto rest = view.substr(2);
            if (rest.find(candidate) != std::string_view::npos) {
                has_repeated = true;
                break;
            }
            view.remove_prefix(1);
        }
        return has_repeated && has_double_char;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        return std::to_string(std::count_if(lines.begin(), lines.end(), is_nice_part1));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        return std::to_string(std::count_if(lines.begin(), lines.end(), is_nice_part2));
    }

    aoc::registration r {2015, 5, part_1, part_2};

    TEST_SUITE("2015_day5") {
        TEST_CASE("2015_day5:example") {
//        REQUIRE_EQ(count_visits(calculate_visits(">")), 2);
//        REQUIRE_EQ(count_visits(calculate_visits("^>v<")), 4);
//        REQUIRE_EQ(count_visits(calculate_visits("^v^v^v^v^v")), 2);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^v")), 3);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^>v<")), 3);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^v^v^v^v^v")), 11);
    }

}

}

