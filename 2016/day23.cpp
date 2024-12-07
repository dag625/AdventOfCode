//
// Created by Dan on 12/7/2024.
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

    /*
     See https://adventofcode.com/2016/day/23
     */

    //Borrowed and refactored from 2016 day 12.

    using state = std::array<int, 5>;
    using argument = std::variant<char, int>;

    constexpr int INS_PTR_INDEX = 0;

    constexpr int register_index(char c) {
        return c - 'a' + 1;
    }

    constexpr int register_index(const argument c) {
        return register_index(std::get<char>(c));
    }

    constexpr int get_value(const state& s, const argument src) {
        if (std::holds_alternative<char>(src)) {
            return s[register_index(std::get<char>(src))];
        }
        else {
            return std::get<int>(src);
        }
    }

    enum class ins_1arg_type : int {
        increment = 1,
        decrement = -1,
        toggle = 0
    };

    struct ins_1arg {
        ins_1arg_type type = ins_1arg_type::increment;
        argument arg;
    };

    enum class ins_2arg_type : int {
        copy = 0,
        jump = 1
    };

    struct ins_2arg {
        ins_2arg_type type = ins_2arg_type::copy;
        argument arg1;
        argument arg2;
    };

    using instruction = std::variant<ins_1arg, ins_2arg>;

    void toggle(ins_1arg& ins) {
        if (ins.type == ins_1arg_type::increment) {
            ins.type = ins_1arg_type::decrement;
        }
        else {
            ins.type = ins_1arg_type::increment;
        }
    }

    void toggle(ins_2arg& ins) {
        if (ins.type == ins_2arg_type::jump) {
            ins.type = ins_2arg_type::copy;
        }
        else {
            ins.type = ins_2arg_type::jump;
        }
    }

    void toggle(instruction& ins) {
        std::visit([](auto& i){ toggle(i); }, ins);
    }

    void run(const ins_1arg& ins, state& s, std::vector<instruction>& program) {
        if (ins.type == ins_1arg_type::toggle) {
            int dist = get_value(s, ins.arg);
            int idx = s[INS_PTR_INDEX] + dist;
            if (idx >= 0 && idx < program.size()) {
                toggle(program[idx]);
            }
            ++s[INS_PTR_INDEX];
        }
        else {
            if (std::holds_alternative<char>(ins.arg)) {
                s[register_index(ins.arg)] += std::to_underlying(ins.type);
            }
            ++s[INS_PTR_INDEX];
        }
    }

    void run(const ins_2arg& ins, state& s, std::vector<instruction>& program) {
        if (ins.type == ins_2arg_type::copy) {
            if (std::holds_alternative<char>(ins.arg2)) {
                s[register_index(ins.arg2)] = get_value(s, ins.arg1);
            }
            ++s[INS_PTR_INDEX];
        }
        else {
            const int check = get_value(s, ins.arg1);
            const int dist = get_value(s, ins.arg2);
            if (check != 0) {
                s[INS_PTR_INDEX] += dist;
            }
            else {
                ++s[INS_PTR_INDEX];
            }
        }
    }

    void run(const instruction& ins, state& s, std::vector<instruction>& program) {
        std::visit([&s, &program](const auto& i){ run(i, s, program); }, ins);
    }

    argument parse_arg(std::string_view a) {
        if (isalpha(a[0])) {
            return a[0];
        }
        else {
            return parse<int>(a);
        }
    }

    ins_1arg parse_1arg_ins(std::string_view s, ins_1arg_type type) {
        const auto parts = split_no_empty(s, ' ');
        return {type, parse_arg(parts[1])};
    }

    ins_2arg parse_2arg_ins(std::string_view s, ins_2arg_type type) {
        const auto parts = split_no_empty(s, ' ');
        return {type, parse_arg(parts[1]), parse_arg(parts[2])};
    }

    instruction parse_ins(std::string_view s) {
        using namespace std::string_view_literals;
        if (s.starts_with("cpy"sv)) {
            return parse_2arg_ins(s, ins_2arg_type::copy);
        }
        else if (s.starts_with("jnz"sv)) {
            return parse_2arg_ins(s, ins_2arg_type::jump);
        }
        else if (s.starts_with("inc"sv)) {
            return parse_1arg_ins(s, ins_1arg_type::increment);
        }
        else if (s.starts_with("dec"sv)) {
            return parse_1arg_ins(s, ins_1arg_type::decrement);
        }
        else {
            return parse_1arg_ins(s, ins_1arg_type::toggle);
        }
    }

    std::vector<instruction> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_23_input.txt");
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        state s{};
        s[1] = 7;
        while (s[INS_PTR_INDEX] >= 0 && s[INS_PTR_INDEX] < input.size()) {
            run(input[s[INS_PTR_INDEX]], s, input);
        }
        return std::to_string(s[1]);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        state s{};
        s[1] = 12;
        while (s[INS_PTR_INDEX] >= 0 && s[INS_PTR_INDEX] < input.size()) {
            run(input[s[INS_PTR_INDEX]], s, input);
        }
        return std::to_string(s[1]);
        //Release run takes ~10s.
    }

    aoc::registration r{2016, 23, part_1, part_2};

//    TEST_SUITE("2016_day23") {
//        TEST_CASE("2016_day23:example") {
//
//        }
//    }

} /* namespace <anon> */