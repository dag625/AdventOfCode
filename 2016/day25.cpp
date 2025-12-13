//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <vector>
#include <variant>
#include <array>
#include <algorithm>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/25
     */

    //Borrowed and refactored from 2016 day 12.

    using state = std::array<int, 5>;

    constexpr int INS_PTR_INDEX = 0;

    constexpr int register_index(char c) {
        return c - 'a' + 1;
    }

    constexpr int get_value(const state& s, const std::variant<char, int> src) {
        if (std::holds_alternative<char>(src)) {
            return s[register_index(std::get<char>(src))];
        }
        else {
            return std::get<int>(src);
        }
    }

    struct copy_ins {
        std::variant<char, int> src;
        char dst = '\0';

        void run(state& s, std::vector<int>& out) const {
            s[register_index(dst)] = get_value(s, src);
            ++s[INS_PTR_INDEX];
        }
    };

    struct incr_ins {
        char dst = '\0';
        int increment = 1;

        void run(state& s, std::vector<int>& out) const {
            s[register_index(dst)] += increment;
            ++s[INS_PTR_INDEX];
        }
    };

    struct jump_ins {
        std::variant<char, int> check;
        int dist = 0;

        void run(state& s, std::vector<int>& out) const {
            const int val = get_value(s, check);
            if (val != 0) {
                s[INS_PTR_INDEX] += dist;
            }
            else {
                ++s[INS_PTR_INDEX];
            }
        }
    };

    struct out_ins {
        char dst = '\0';

        void run(state& s, std::vector<int>& out) const {
            const int val = s[register_index(dst)];
            out.push_back(val);
            ++s[INS_PTR_INDEX];
        }
    };

    using instruction = std::variant<copy_ins, incr_ins, jump_ins, out_ins>;

    copy_ins parse_copy(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        if (isalpha(parts[1][0])) {
            return {parts[1][0], parts[2][0]};
        }
        else {
            return {parse<int>(parts[1]), parts[2][0]};
        }
    }

    incr_ins parse_incr(std::string_view s) {
        using namespace std::string_view_literals;
        const auto parts = split_no_empty(s, ' ');
        int num = 1;
        if (parts[0] == "dec"sv) {
            num = -1;
        }
        return {parts[1][0], num};
    }

    jump_ins parse_jump(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        if (isalpha(parts[1][0])) {
            return {parts[1][0], parse<int>(parts[2])};
        }
        else {
            return {parse<int>(parts[1]), parse<int>(parts[2])};
        }
    }

    out_ins parse_out(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        return {parts[1][0]};
    }

    instruction parse_ins(std::string_view s) {
        using namespace std::string_view_literals;
        if (s.starts_with("cpy"sv)) {
            return parse_copy(s);
        }
        else if (s.starts_with("jnz"sv)) {
            return parse_jump(s);
        }
        else if (s.starts_with("out"sv)) {
            return parse_out(s);
        }
        else {
            return parse_incr(s);
        }
    }

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    bool is_alternating(const std::vector<int>& out) {
        if (out.size() < 2) {
            return false;
        }
        const int odd = out.front();
        const int even = odd == 0 ? 1 : 0;
        return std::ranges::all_of(out | std::views::stride(2), [odd](const int c){ return c == odd; }) &&
                std::ranges::all_of(out | std::views::drop(1) | std::views::stride(2), [even](const int c){ return c == even; });
    }

    std::vector<int> generate(int i, const std::vector<instruction>& prog) {
        state s{};
        s[1] = i;
        std::vector<int> out;
        std::vector<state> seen;
        seen.push_back(s);
        while (s[INS_PTR_INDEX] >= 0 && s[INS_PTR_INDEX] < prog.size()) {
            std::visit([&s, &out](const auto& ins){ ins.run(s, out); }, prog[s[INS_PTR_INDEX]]);
            const auto found = std::lower_bound(seen.begin(), seen.end(), s);
            if (found == seen.end() || *found != s) {
                seen.insert(found, s);
            }
            else {
                //We're going to repeat now.
                break;
            }
        }
        return out;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int val = 0;
        for (;; ++val) {
            const auto out = generate(val, input);
            bool alternates = is_alternating(out);
            if (alternates) {
                break;
            }
        }
        return std::to_string(val);
    }

    aoc::registration r{2016, 25, part_1};

//    TEST_SUITE("2016_day25") {
//        TEST_CASE("2016_day25:example") {
//
//        }
//    }

} /* namespace <anon> */