//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <vector>
#include <algorithm>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/6
     */

    using state = std::vector<int>;

    auto cmp_states(const state& lhs, const state& rhs) { return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
    bool les_states(const state& lhs, const state& rhs) { return cmp_states(lhs, rhs) == std::strong_ordering::less; }
    bool eql_states(const state& lhs, const state& rhs) { return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }

    state get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_6_input.txt");
        const auto parts = split_no_empty(lines.front(), '\t');
        return parts | std::views::transform([](std::string_view s){ return parse<int>(s); }) | std::ranges::to<std::vector>();
    }

    struct sighting {
        int step = 0;
        state s;

        auto operator<=>(const sighting& rhs) const { return cmp_states(s, rhs.s); }
        bool operator==(const sighting& rhs) const { return eql_states(s, rhs.s); }

        auto operator<=>(const state& rhs) const { return cmp_states(s, rhs); }
        bool operator==(const state& rhs) const { return eql_states(s, rhs); }
    };

    bool is_seen(const state& s, std::vector<sighting>& seen, const int step, int& last_seen) {
        const auto found = std::lower_bound(seen.begin(), seen.end(), s);
        if (found == seen.end() || *found != s) {
            seen.emplace(found, step, s);
            return false;
        }
        else {
            last_seen = found->step;
            return true;
        }
    }

    std::ptrdiff_t next_idx(const state& s, std::ptrdiff_t current) {
        return (++current) % static_cast<std::ptrdiff_t>(s.size());
    }

    void redistribute(state& s) {
        auto idx = std::distance(s.begin(), std::max_element(s.begin(), s.end()));
        int to_dist = s[idx];
        s[idx] = 0;
        idx = next_idx(s, idx);
        while (to_dist > 0) {
            ++s[idx];
            --to_dist;
            idx = next_idx(s, idx);
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        state s = input;
        std::vector<sighting> seen;
        int steps = 0, last_seen_step = 0;
        while (!is_seen(s, seen, steps, last_seen_step)) {
            redistribute(s);
            ++steps;
        }
        return std::to_string(steps);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        state s = input;
        std::vector<sighting> seen;
        int steps = 0, last_seen_step = 0;
        while (!is_seen(s, seen, steps, last_seen_step)) {
            redistribute(s);
            ++steps;
        }
        return std::to_string(steps - last_seen_step);
    }

    aoc::registration r{2017, 6, part_1, part_2};

    TEST_SUITE("2017_day06") {
        TEST_CASE("2017_day06:example") {
            state s {0, 2, 7, 0};
            std::vector<sighting> seen;
            int steps = 0, last_seen_step = 0;
            while (!is_seen(s, seen, steps, last_seen_step)) {
                redistribute(s);
                ++steps;
            }
            CHECK_EQ(steps, 5);
            CHECK_EQ(steps - last_seen_step, 4);
        }
    }

} /* namespace <anon> */