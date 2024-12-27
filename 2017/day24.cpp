//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/24
     */

    struct comp {
        int port1 = -1;
        int port2 = -1;
        int strength = 0;

        comp() = default;
        comp(int p1, int p2) : port1{std::min(p1, p2)}, port2{std::max(p1, p2)}, strength{p1 + p2} {}
        comp(int p1, int p2, int s) : port1{std::min(p1, p2)}, port2{std::max(p1, p2)}, strength{s} {}

        auto operator<=>(const comp& rhs) const { return std::tie(port1, port2) <=> std::tie(rhs.port1, rhs.port2); }
        bool operator==(const comp& rhs) const { return std::tie(port1, port2) == std::tie(rhs.port1, rhs.port2); }
    };

    comp parse_comp(std::string_view s) {
        const auto parts = split(s, '/');
        return {parse<int>(parts[0]), parse<int>(parts[1])};
    }

    std::vector<comp> get_input(const std::vector<std::string>& lines) {
        auto retval = lines | std::views::transform(&parse_comp) | std::ranges::to<std::vector>();
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    std::optional<int> can_combine(const int port, const comp& c) {
        if (port == c.port1) {
            return c.port2;
        }
        else if (port == c.port2) {
            return c.port1;
        }
        else {
            return std::nullopt;
        }
    }

    int max_strength(std::vector<comp>& current, const std::vector<comp>& all, const int last) {
        int retval = 0;
        for (const auto& c : all) {
            const auto found = std::find(current.begin(), current.end(), c);
            if (const auto p = can_combine(last, c); found == current.end() && p.has_value()) {
                current.push_back(c);
                const auto str = current.back().strength + max_strength(current, all, *p);
                if (str > retval) {
                    retval = str;
                }
                current.pop_back();
            }
        }
        return retval;
    }

    int max_strength(const std::vector<comp>& all, const int last) {
        std::vector<comp> current;
        current.reserve(all.size());
        return max_strength(current, all, last);
    }

    std::pair<int, int> max_length_strength(std::vector<comp>& current, const std::vector<comp>& all, const int last) {
        int length = 0, strength = 0;
        for (const auto& c : all) {
            const auto found = std::find(current.begin(), current.end(), c);
            if (const auto p = can_combine(last, c); found == current.end() && p.has_value()) {
                current.push_back(c);
                auto [len, str] = max_length_strength(current, all, *p);
                len += 1;
                str += current.back().strength;
                if (len > length) {
                    length = len;
                    strength = str;
                }
                else if (len == length && str > strength) {
                    strength = str;
                }
                current.pop_back();
            }
        }
        return {length, strength};
    }

    std::pair<int, int> max_length_strength(const std::vector<comp>& all, const int last) {
        std::vector<comp> current;
        current.reserve(all.size());
        return max_length_strength(current, all, last);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto str = max_strength(input, 0);
        return std::to_string(str);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [len, str] = max_length_strength(input, 0);
        return std::to_string(str);
    }

    aoc::registration r{2017, 24, part_1, part_2};

//    TEST_SUITE("2017_day24") {
//        TEST_CASE("2017_day24:example") {
//
//        }
//    }

} /* namespace <anon> */