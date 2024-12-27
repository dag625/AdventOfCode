//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/25
     */

    char parse_1st_state(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        return parts[3][0];
    }

    int parse_num_steps(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        return parse<int>(parts[5]);
    }

    struct state_action {
        int set_value = 0;
        int move_by = 0;
        char next_state = '\0';
    };

    struct state {
        char name = '\0';
        std::array<state_action, 2> actions{};
    };

    state_action parse_action(std::string_view set_str, std::string_view move_str, std::string_view next_str) {
        const auto set_ps = split_no_empty(set_str, ' ');
        const auto move_ps = split_no_empty(move_str, ' ');
        const auto next_ps = split_no_empty(next_str, ' ');
        return {parse<int>(set_ps[4]), move_ps[6] == "right." ? 1 : -1, next_ps[4][0]};
    }

    state parse_state(auto& str_range) {
        std::string_view name_str {*std::next(str_range.begin(), 0)};
        std::string_view set0_str {*std::next(str_range.begin(), 2)};
        std::string_view move0_str {*std::next(str_range.begin(), 3)};
        std::string_view next0_str {*std::next(str_range.begin(), 4)};
        std::string_view set1_str {*std::next(str_range.begin(), 6)};
        std::string_view move1_str {*std::next(str_range.begin(), 7)};
        std::string_view next1_str {*std::next(str_range.begin(), 8)};
        const auto parts = split_no_empty(name_str, ' ');
        return {parts[2][0], {parse_action(set0_str, move0_str, next0_str), parse_action(set1_str, move1_str, next1_str)}};
    }

    struct machine {
        char init_state = '\0';
        int num_steps = 0;
        std::vector<state> states;
    };

    machine get_input(const std::vector<std::string>& lines) {
        const auto init_state = parse_1st_state(lines[0]);
        const auto num_steps = parse_num_steps(lines[1]);
        return {init_state, num_steps,
                lines |
                std::views::drop(3) |
                std::views::filter([](std::string_view s){ return !s.empty(); }) |
                std::views::chunk(9) |
                std::views::transform([](const auto& v){ return parse_state(v); }) |
                std::ranges::to<std::vector>()};
    }

    void step(const machine& m, int& cursor, int& sp, std::vector<int>& on) {
        const auto found = std::lower_bound(on.begin(), on.end(), cursor);
        bool is_on = found != on.end() && *found == cursor;
        const auto& action = m.states[sp].actions[static_cast<int>(is_on)];
        if (is_on && action.set_value == 0) {
            on.erase(found);
        }
        else if (!is_on && action.set_value == 1) {
            on.insert(found, cursor);
        }
        cursor += action.move_by;
        sp = static_cast<int>(action.next_state - 'A');
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<int> on;
        int cursor = 0, sp = static_cast<int>(input.init_state - 'A');
        for (int i = 0; i < input.num_steps; ++i) {
            step(input, cursor, sp, on);
        }
        return std::to_string(on.size());
    }

    aoc::registration r{2017, 25, part_1};

//    TEST_SUITE("2017_day25") {
//        TEST_CASE("2017_day25:example") {
//
//        }
//    }

} /* namespace <anon> */