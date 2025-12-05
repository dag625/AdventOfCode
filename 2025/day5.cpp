//
// Created by Dan on 12/5/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2025/day/5
     */

    std::pair<int64_t, int64_t> parse_range(std::string_view str) {
        const auto parts = split(str, '-');
        return {parse64(parts[0]), parse64(parts[1])};
    }

    std::pair<std::vector<std::pair<int64_t, int64_t>>, std::vector<int64_t>> get_input(const std::vector<std::string>& lines) {
        const auto split = std::find(lines.begin(), lines.end(), "");
        const auto num_ranges = std::distance(lines.begin(), split);
        return {
            lines | std::views::take(num_ranges) | std::views::transform(&parse_range) | std::ranges::to<std::vector>(),
            lines | std::views::drop(num_ranges + 1) | std::views::transform(&parse64) | std::ranges::to<std::vector>()
        };
    }

    bool is_fresh(const int64_t id, const std::vector<std::pair<int64_t, int64_t>>& valid) {
        return std::any_of(valid.begin(), valid.end(),
            [id](const std::pair<int64_t, int64_t>& v) { return id >= v.first && id <= v.second; });
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [valid, ids] = get_input(lines);
        const auto num_fresh = std::count_if(ids.begin(), ids.end(), [&valid](const int64_t id) { return is_fresh(id, valid); });
        return std::to_string(num_fresh);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto [valid, ignore_me] = get_input(lines);
        std::sort(valid.begin(), valid.end());
        for (int idx = 1; idx < valid.size();) {
            int target = idx - 1;
            auto& t = valid[target];
            const auto& i = valid[idx];
            if (t.second + 1 >= i.first) {
                if (i.second > t.second) {
                    t.second = i.second;
                }
                valid.erase(valid.begin() + idx);
            }
            else {
                ++idx;
            }
        }
        const auto possible_fresh = std::accumulate(valid.begin(), valid.end(), 0ll,
            [](int64_t total, const std::pair<int64_t, int64_t>& v) { return total + (v.second - v.first + 1); });
        return std::to_string(possible_fresh);
    }

    aoc::registration r{2025, 5, part_1, part_2};

    //    TEST_SUITE("2025_day05") {
    //        TEST_CASE("2025_day05:example") {
    //            const std::vector<std::string> lines {
    //
    //            };
    //            const auto input = get_input(lines);
    //
    //        }
    //    }

} /* namespace <anon> */
