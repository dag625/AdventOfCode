//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <utility>
#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/8
     */

    using check_f = bool(*)(int, int);

    struct instruction {
        std::string to_modify;
        int to_add = 0;
        std::string to_check;
        int check_val = 0;
        check_f func;

        [[nodiscard]] bool check(int chk) const {
            return func(chk, check_val);
        }
    };

    bool check_lt(int a, int b) { return a <  b; }
    bool check_le(int a, int b) { return a <= b; }
    bool check_gt(int a, int b) { return a >  b; }
    bool check_ge(int a, int b) { return a >= b; }
    bool check_eq(int a, int b) { return a == b; }
    bool check_ne(int a, int b) { return a != b; }

    instruction parse_ins(std::string_view s) {
        const auto parts = split(s, ' ');
        int mul = parts[1] == "inc" ? 1 : -1;
        check_f f = nullptr;
        if (parts[5] == "==") {
            f = &check_eq;
        }
        else if (parts[5] == "!=") {
            f = &check_ne;
        }
        else if (parts[5] == "<") {
            f = &check_lt;
        }
        else if (parts[5] == "<=") {
            f = &check_le;
        }
        else if (parts[5] == ">") {
            f = &check_gt;
        }
        else if (parts[5] == ">=") {
            f = &check_ge;
        }
        return {std::string{parts[0]}, parse<int>(parts[2]) * mul, std::string{parts[4]}, parse<int>(parts[6]), f};
    }

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    struct reg {
        std::string name;
        int val = 0;

        reg(std::string n) : name{std::move(n)} {}

        auto operator<=>(const reg& rhs) const { return name <=> rhs.name; }
        bool operator==(const reg& rhs) const { return name == rhs.name; }

        auto operator<=>(const std::string& rhs) const { return name <=> rhs; }
        bool operator==(const std::string& rhs) const { return name == rhs; }
    };

    void checked_insert(std::vector<reg>& registers, const std::string& name) {
        const auto found = std::lower_bound(registers.begin(), registers.end(), name);
        if (found == registers.end() || found->name != name) {
            registers.emplace(found, name);
        }
    }

    std::vector<reg> init_regs(const std::vector<instruction>& input) {
        std::vector<reg> retval;
        for (const auto& ins : input) {
            checked_insert(retval, ins.to_modify);
            checked_insert(retval, ins.to_check);
        }
        return retval;
    }

    int& get_register(std::vector<reg>& registers, const std::string& name) {
        const auto found = std::lower_bound(registers.begin(), registers.end(), name);
        return found->val;
    }

    void run(std::vector<reg>& registers, const instruction& ins) {
        int& to_chk = get_register(registers, ins.to_check);
        if (ins.check(to_chk)) {
            int& to_mod = get_register(registers, ins.to_modify);
            to_mod += ins.to_add;
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto registers = init_regs(input);
        int highest = 0;
        for (const auto& ins : input) {
            run(registers, ins);
        }
        const auto max_el = std::max_element(registers.begin(), registers.end(), [](const reg& lhs, const reg& rhs){ return lhs.val < rhs.val; });
        return std::to_string(max_el->val);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto registers = init_regs(input);
        int highest = 0;
        for (const auto& ins : input) {
            run(registers, ins);
            const auto max_el = std::max_element(registers.begin(), registers.end(), [](const reg& lhs, const reg& rhs){ return lhs.val < rhs.val; });
            if (max_el->val > highest) {
                highest = max_el->val;
            }
        }
        return std::to_string(highest);
    }

    aoc::registration r{2017, 8, part_1, part_2};

//    TEST_SUITE("2017_day08") {
//        TEST_CASE("2017_day08:example") {
//
//        }
//    }

} /* namespace <anon> */