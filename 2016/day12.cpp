//
// Created by Dan on 12/5/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <array>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

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

        void run(state& s) const {
            s[register_index(dst)] = get_value(s, src);
            ++s[INS_PTR_INDEX];
        }
    };

    struct incr_ins {
        char dst = '\0';
        int increment = 1;

        void run(state& s) const {
            s[register_index(dst)] += increment;
            ++s[INS_PTR_INDEX];
        }
    };

    struct jump_ins {
        std::variant<char, int> check;
        int dist = 0;

        void run(state& s) const {
            const int val = get_value(s, check);
            if (val != 0) {
                s[INS_PTR_INDEX] += dist;
            }
            else {
                ++s[INS_PTR_INDEX];
            }
        }
    };

    using instruction = std::variant<copy_ins, incr_ins, jump_ins>;

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

    instruction parse_ins(std::string_view s) {
        using namespace std::string_view_literals;
        if (s.starts_with("cpy"sv)) {
            return parse_copy(s);
        }
        else if (s.starts_with("jnz"sv)) {
            return parse_jump(s);
        }
        else {
            return parse_incr(s);
        }
    }


    /*
     See https://adventofcode.com/2016/day/12
     */

    std::vector<instruction> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_12_input.txt");
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        state s{};
        while (s[INS_PTR_INDEX] >= 0 && s[INS_PTR_INDEX] < input.size()) {
            std::visit([&s](const auto& ins){ ins.run(s); }, input[s[INS_PTR_INDEX]]);
        }
        return std::to_string(s[1]);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        state s{};
        s[3] = 1;
        while (s[INS_PTR_INDEX] >= 0 && s[INS_PTR_INDEX] < input.size()) {
            std::visit([&s](const auto& ins){ ins.run(s); }, input[s[INS_PTR_INDEX]]);
        }
        return std::to_string(s[1]);
    }

    aoc::registration r{2016, 12, part_1, part_2};

//    TEST_SUITE("2016_day12") {
//        TEST_CASE("2016_day12:example") {
//
//        }
//    }

} /* namespace <anon> */