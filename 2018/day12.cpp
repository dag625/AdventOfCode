//
// Created by Dan on 11/11/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <numeric>

#include "utilities.h"
#include "ranges.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/12
     */

    using index_t = std::ranges::range_difference_t<std::vector<int>>;
    using state = std::vector<index_t>;
    using spread = std::string;
    using spread_list = std::vector<spread>;

    constexpr index_t OFFSET = 4;
    constexpr int64_t P2_NUM_STEPS = 50000000000;
    constexpr std::string_view INIT_PREFIX {"initial state: "};

    std::optional<spread> parse_spread(std::string_view str) {
        if (str.back() == '.') {
            return std::nullopt;
        }
        else {
            return std::string{str.substr(0, 5)};
        }
    }

    std::pair<state, spread_list> get_input(const std::vector<std::string>& lines) {
        state s;
        for (const auto [idx, c] : lines.front() | std::views::drop(INIT_PREFIX.size()) | std::views::enumerate) {
            if (c == '#') {
                s.push_back(idx);
            }
        }
        spread_list l;
        for (const auto& it : lines | std::views::drop((2))) {
            auto sp = parse_spread(it);
            if (sp) {
                l.push_back(std::move(*sp));
            }
        }
        return {std::move(s), std::move(l)};
    }

    std::string build_window(const index_t idx, state::const_iterator current, const state::const_iterator& end) {
        std::string retval;
        for (auto i = idx; i < idx + 5; ++i) {
            if (current != end && i == *current) {
                retval.push_back('#');
                ++current;
            }
            else {
                retval.push_back('.');
            }
        }
        return retval;
    }

    state step(const state& s, const spread_list& spreads) {
        state retval;
        retval.reserve(s.size() + 2 * OFFSET);
        auto current = s.begin();
        const auto end = s.end();
        for (index_t idx = s.front() - OFFSET; idx <= s.back(); ++idx) {
            while (current != end && *current < idx) { ++current; }
            if (current == end) { break; }
            const auto win = build_window(idx, current, end);
            const auto found = std::find(spreads.begin(), spreads.end(), win);
            if (found != spreads.end()) {
                retval.push_back(idx + OFFSET / 2);
            }
        }
        return retval;
    }

    index_t calculate_score(const state& s) {
        return std::accumulate(s.begin(), s.end(), index_t{0});
    }

    std::string format_state(const state& s, const std::optional<std::pair<index_t, index_t>> range = std::nullopt) {
        if (s.empty()) {
            return "Empty state.";
        }
        std::string retval;
        if (!range) { retval = std::format("Starts at {:4}:  ", s.front()); };
        for (index_t i = s.front(); i <= s.back(); ++i) {
            if (std::ranges::find(s, i) != s.end()) {
                retval.push_back('#');
            }
            else {
                retval.push_back('.');
            }
        }
        if (range) {
            std::string prefix (static_cast<std::string::size_type>(s.front() - range->first), '.');
            std::string suffix (static_cast<std::string::size_type>(range->second - s.back()), '.');
            retval.insert(retval.begin(), prefix.begin(), prefix.end());
            retval.insert(retval.end(), suffix.begin(), suffix.end());
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [init_state, spreads] = get_input(lines);
        auto working = init_state;
        //fmt::print("Init:  {}\n", format_state(init_state));
        for (int s = 0; s < 20; ++s) {
            working = step(working, spreads);
            const auto score = calculate_score(working);
            //fmt::print("Step {:3} ({:5}):  {}\n", s+1, score, format_state(working));
        }
        return std::to_string(calculate_score(working));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [init_state, spreads] = get_input(lines);
        auto working = init_state;
        index_t last_score = 0, last_delta = 0;
        int64_t final_score = 0;
        for (int64_t s = 0; s < P2_NUM_STEPS; ++s) {
            working = step(working, spreads);
            const auto score = calculate_score(working);
            const auto delta = score - last_score;
            //fmt::print("Step {:3} ({:6}; Delta {:6}):  {}\n", s+1, score, delta, format_state(working, std::pair{-5, 1000}));
            if (delta == last_delta) {
                final_score = score + delta * (P2_NUM_STEPS - s - 1);
                break;
            }
            last_score = score;
            last_delta = delta;
        }
        return std::to_string(final_score);
    }

    aoc::registration r {2018, 12, part_1, part_2};

    TEST_SUITE("2018_day12") {
        TEST_CASE("2018_day12:example") {
            using namespace std::string_literals;
            const std::vector<std::string> lines {
                "initial state: #..#.#..##......###...###"s,
                ""s,
                "...## => #"s,
                "..#.. => #"s,
                ".#... => #"s,
                ".#.#. => #"s,
                ".#.## => #"s,
                ".##.. => #"s,
                ".#### => #"s,
                "#.#.# => #"s,
                "#.### => #"s,
                "##.#. => #"s,
                "##.## => #"s,
                "###.. => #"s,
                "###.# => #"s,
                "####. => #s"
            };
            const auto [init_state, spreads] = get_input(lines);
            auto working = init_state;
            //fmt::print("Init:      {}\n", format_state(init_state, std::pair{-5, 40}));
            for (int s = 0; s < 20; ++s) {
                working = step(working, spreads);
                //fmt::print("Step {:3}:  {}\n", s+1, format_state(working, std::pair{-5, 40}));
            }
            CHECK_EQ(calculate_score(working), 325);
        }
    }

} /* namespace <anon> */