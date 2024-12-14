//
// Created by Dan on 12/14/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <vector>
#include <variant>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/23
     */

    struct state {
        std::array<int64_t, 8> registers{};
        int64_t ip = 0;
        int num_mul = 0;
    };

    using operand = std::variant<char, int64_t>;

    int64_t& get_register(state& s, char r) {
        return s.registers[static_cast<int>(r - 'a')];
    }

    int64_t get_value(state& s, operand op) {
        if (std::holds_alternative<char>(op)) {
            return get_register(s, std::get<char>(op));
        }
        else {
            return std::get<int64_t>(op);
        }
    }

    operand parse_op(std::string_view s) {
        s = trim(s);
        if (isalpha(s.front())) {
            return s.front();
        }
        else {
            return parse<int64_t>(s);
        }
    }

    struct set_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, op1) = get_value(s, op2);
            ++s.ip;
        }
    };

    set_ins parse_set(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct sub_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, op1) -= get_value(s, op2);
            ++s.ip;
        }
    };

    sub_ins parse_sub(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct mul_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, op1) *= get_value(s, op2);
            ++s.ip;
            ++s.num_mul;
        }
    };

    mul_ins parse_mul(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct jump_ins {
        operand op1;
        operand op2;

        void run1(state& s) const {
            if (get_value(s, op1) != 0) {
                s.ip += get_value(s,  op2);
            }
            else {
                ++s.ip;
            }
        }
    };

    jump_ins parse_jnz(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parse_op(parts[1]), parse_op(parts[2])};
    }

    using instruction = std::variant<set_ins, sub_ins, mul_ins, jump_ins>;

    instruction parse_ins(std::string_view s) {
        if (s.starts_with("set")) {
            return parse_set(s);
        }
        else if (s.starts_with("sub")) {
            return parse_sub(s);
        }
        else if (s.starts_with("mul")) {
            return parse_mul(s);
        }
        else {
            return parse_jnz(s);
        }
    }

    std::vector<instruction> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_23_input.txt");
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    bool is_non_prime(int64_t val) {
        const auto max = static_cast<int64_t>(std::sqrt(val)) + 1;
        for (int i = 2; i <= max; ++i) {
            if (val % i == 0) {
                return true;
            }
        }
        return false;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        state s{};
        while (s.ip >= 0 && s.ip < input.size()) {
            std::visit([&s](const auto& ins){ ins.run1(s); }, input[s.ip]);
        }
        return std::to_string(s.num_mul);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        /*
         * ANALYSIS OF INPUT PROGRAM:
         * Note:  INCR is the increment of the 2nd to last instruction from the input
         * e is incremented until it equals b, at which point d is incremented
         * when d and e are equal to b, b is incremented by INCR and f is set to 1
         * program stops when b, c, d, and e are equal
         * f is set to 0 when d * e == b
         * f is set to 1 when b is incremented (after all other increment operations)
         * g is set to 0 when b is incremented (b == d == e)
         * h is incremented when f and g are 0
         * -> h is incremented when b is non-prime and incremented
         * --> h = # of non-prime numbers between the inital b and c, incrementing by INCR, inclusive of c
         */
        const auto incr = -std::get<int64_t>(std::get<sub_ins>(input[input.size() - 2]).op2);
        state s{};
        s.registers[0] = 1;
        while (s.ip >= 0 && s.ip < input.size() && s.ip < 8) {
            std::visit([&s](const auto& ins){ ins.run1(s); }, input[s.ip]);
        }
        int h = 0;
        for (int64_t val = s.registers[1]; val <= s.registers[2]; val += incr) {
            h += static_cast<int>(is_non_prime(val));
        }
        return std::to_string(h);
    }

    aoc::registration r{2017, 23, part_1, part_2};

//    TEST_SUITE("2017_day23") {
//        TEST_CASE("2017_day23:example") {
//
//        }
//    }

} /* namespace <anon> */