//
// Created by Dan on 12/2/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <regex>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/2
     */

    std::pair<int64_t, int64_t> parse_range(std::string_view str) {
        const auto parts = split(str, '-');
        return {parse64(parts[0]), parse64(parts[1])};
    }

    std::vector<std::pair<int64_t, int64_t>> get_input(const std::vector<std::string>& lines) {
        return split(lines.front(), ',') | std::views::transform(&parse_range) | std::ranges::to<std::vector>();
    }

    int64_t check_invalid_p1(int64_t val) {
        const auto str = fmt::format("{}", val);
        const auto sz = str.size() / 2;
        if (str.size() % 2 == 0 && str.substr(0, sz) == str.substr(sz)) {
            return val;
        }
        else {
            return 0;
        }
    }

    int64_t check_invalid_p2(int64_t val) {
        static const std::regex RE {"([0-9]+)\\1+", std::regex::ECMAScript|std::regex::optimize};
        const auto str = fmt::format("{}", val);
        if (std::regex_match(str, RE)) {
            return val;
        }
        else {
            return 0;
        }
    }

    int64_t check_invalid_p2_alt(int64_t val) {
        //This is substantially faster than a regex, only slightly slower than p1.
        const auto str = fmt::format("{}", val);
        const auto str_sz = str.size();
        for (std::size_t num = 2; num <= str_sz; ++num) {
            if (str_sz % num == 0) {
                const std::size_t sz = str.size() / num;
                std::string_view part {str.begin(), str.begin() + static_cast<std::ptrdiff_t>(sz)};
                bool any_diff = false;
                for (std::size_t s = 1; s < num && !any_diff; ++s) {
                    std::string_view sv {str.begin() + static_cast<std::ptrdiff_t>(s * sz), str.begin() + static_cast<std::ptrdiff_t>((s + 1) * sz)};
                    any_diff = part != sv;
                }
                if (!any_diff) {
                    return val;
                }
            }
        }
        return 0;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (const auto [first, last] : input) {
            for (auto val = first; val <= last; ++val) {
                sum += check_invalid_p1(val);
            }
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (const auto [first, last] : input) {
            for (auto val = first; val <= last; ++val) {
                sum += check_invalid_p2_alt(val);
            }
        }
        return std::to_string(sum);
    }

    aoc::registration r {2025, 2, part_1, part_2};

    TEST_SUITE("2025_day02") {
        TEST_CASE("2025_day02:example") {
            const std::vector<std::string> lines {
                "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,1698522-1698528,446443-446449,38593856-38593862,565653-565659,824824821-824824827,2121212118-2121212124"
            };
            const auto input = get_input(lines);
            int64_t sum1 = 0;
            for (const auto [first, last] : input) {
                for (auto val = first; val <= last; ++val) {
                    sum1 += check_invalid_p1(val);
                }
            }
            CHECK_EQ(sum1, 1227775554ll);

            CHECK_EQ(check_invalid_p2(121212), 121212);
            CHECK_EQ(check_invalid_p2(121211), 0);

            CHECK_EQ(check_invalid_p2_alt(1111111), 1111111);
            CHECK_EQ(check_invalid_p2_alt(121212), 121212);
            CHECK_EQ(check_invalid_p2_alt(121211), 0);

            int64_t sum2 = 0;
            for (const auto [first, last] : input) {
                for (auto val = first; val <= last; ++val) {
                    sum2 += check_invalid_p2_alt(val);
                }
            }
            CHECK_EQ(sum2, 4174379265ll);
        }
    }

} /* namespace <anon> */