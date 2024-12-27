//
// Created by Dan on 12/17/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <vector>
#include <array>
#include <iostream>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/17
     */

    struct state {
        std::array<int64_t, 3> regs{};
        int ip = 0;
        std::vector<int> output;

        state() = default;
        state(int64_t rA, int64_t rB, int64_t rC) : regs{rA, rB, rC} {}
    };

    std::pair<state, std::vector<int>> get_input(const std::vector<std::string>& lines) {
        const auto p1 = split_no_empty(lines[0], ' ');
        const auto p2 = split_no_empty(lines[1], ' ');
        const auto p3 = split_no_empty(lines[2], ' ');
        const auto prog_str_parts = split_no_empty(lines[4], ' ');
        const auto prog_strs = split(prog_str_parts[1], ',');
        return {{parse64(p1[2]), parse64(p2[2]), parse64(p3[2])},
                prog_strs | std::views::transform(&parse32) | std::ranges::to<std::vector>()};
    }

    int64_t get_combo(const state&s, const int operand) {
        if (operand < 4) {
            return static_cast<int64_t>(operand);
        }
        else {
            return s.regs[operand - 4];
        }
    }

    void run_adv(state& s, const int operand) {
        const auto val = get_combo(s, operand);
        const auto res = s.regs[0] >> val;
        s.regs[0] = res;
        s.ip += 2;
    }

    void run_bxl(state& s, const int operand) {
        const auto res = s.regs[1] ^ operand;
        s.regs[1] = res;
        s.ip += 2;
    }

    void run_bst(state& s, const int operand) {
        const auto res = get_combo(s, operand) & 0x07;
        s.regs[1] = res;
        s.ip += 2;
    }

    void run_jnz(state& s, const int operand) {
        if (s.regs[0] != 0) {
            s.ip = operand;
        }
        else {
            s.ip += 2;
        }
    }

    void run_bxc(state& s, const int operand) {
        const auto res = s.regs[1] ^ s.regs[2];
        s.regs[1] = res;
        s.ip += 2;
    }

    void run_out(state& s, const int operand) {
        const auto res = get_combo(s, operand) & 0x07;
        s.output.push_back(static_cast<int>(res));
        s.ip += 2;
    }

    void run_bdv(state& s, const int operand) {
        const auto val = get_combo(s, operand);
        const auto res = s.regs[0] >> val;
        s.regs[1] = res;
        s.ip += 2;
    }

    void run_cdv(state& s, const int operand) {
        const auto val = get_combo(s, operand);
        const auto res = s.regs[0] >> val;
        s.regs[2] = res;
        s.ip += 2;
    }

    using opcode_func = void (*)(state& s, int operand);

    constexpr std::array<opcode_func, 8> OPS {
        &run_adv,
        &run_bxl,
        &run_bst,
        &run_jnz,
        &run_bxc,
        &run_out,
        &run_bdv,
        &run_cdv
    };

    std::string print_combo(const int operand) {
        if (operand < 4) {
            return std::to_string(operand);
        }
        else {
            const char c = static_cast<char>('A' + (operand - 4));
            std::string retval;
            retval.push_back(c);
            return retval;
        }
    }

    void print_adv(state& s, const int operand) {
        fmt::println("{:2}:  adv {}", s.ip, print_combo(operand));
        s.ip += 2;
    }

    void print_bxl(state& s, const int operand) {
        fmt::println("{:2}:  bxl {}", s.ip, operand);
        s.ip += 2;
    }

    void print_bst(state& s, const int operand) {
        fmt::println("{:2}:  bst {}", s.ip, print_combo(operand));
        s.ip += 2;
    }

    void print_jnz(state& s, const int operand) {
        fmt::println("{:2}:  jnz {}", s.ip, operand);
        s.ip += 2;
    }

    void print_bxc(state& s, const int operand) {
        fmt::println("{:2}:  bxc", s.ip);
        s.ip += 2;
    }

    void print_out(state& s, const int operand) {
        fmt::println("{:2}:  out {}", s.ip, print_combo(operand));
        s.ip += 2;
    }

    void print_bdv(state& s, const int operand) {
        fmt::println("{:2}:  bdv {}", s.ip, print_combo(operand));
        s.ip += 2;
    }

    void print_cdv(state& s, const int operand) {
        fmt::println("{:2}:  cdv {}", s.ip, print_combo(operand));
        s.ip += 2;
    }

    using opcode_func = void (*)(state& s, int operand);

    constexpr std::array<opcode_func, 8> PRINT_OPS {
            &print_adv,
            &print_bxl,
            &print_bst,
            &print_jnz,
            &print_bxc,
            &print_out,
            &print_bdv,
            &print_cdv
    };

    void run(state& s, const std::vector<int>& program) {
        while (s.ip >= 0 && s.ip < program.size()) {
//            if (s.ip == 0) {
//                std::cout << fmt::format("{:2}:  {:16} -> {:2}", s.ip, fmt::join(s.regs, ", "), fmt::join(s.output, ", ")) << std::endl;
//            }
            OPS[program[s.ip]](s, program[s.ip+1]);
        }
    }

    void print(const state& init, const std::vector<int>& program) {
        state s = init;
        while (s.ip >= 0 && s.ip < program.size()) {
            PRINT_OPS[program[s.ip]](s, program[s.ip+1]);
        }
    }

    int64_t find_digit(const int64_t current, const int digit, const std::vector<int>& program, const std::vector<int>& expected) {
        const int64_t octet_unit = 1ll << (digit * 3);
        //To implement backtracking we just keep searching higher numbers, overwriting other previously found
        //digits in current.
        for (int i = 0; i < std::numeric_limits<int>::max(); ++i) {
            const auto candidate = current + octet_unit * i;
            if (candidate == 0) {
                continue;
            }
            state s{};
            s.regs[0] = static_cast<int64_t>(candidate);
            run(s, program);
            if (std::equal(expected.begin() + digit, expected.end(), s.output.begin() + digit, s.output.end())) {
                return candidate;
            }
        }
        return 0;//Shouldn't get here.
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto [s, program] = get_input(lines);
        //print(s, program);
        run(s, program);
        return fmt::format("{}", fmt::join(s.output, ","));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto [is, program] = get_input(lines);
        int64_t a = 0;
        for (int d = static_cast<int>(program.size()) - 1; d >= 0; --d) {
            a = find_digit(a, d, program, program);
        }
        return std::to_string(a);
    }

    aoc::registration r{2024, 17, part_1, part_2};

//    TEST_SUITE("2024_day17") {
//        TEST_CASE("2024_day17:example") {
//
//        }
//    }

} /* namespace <anon> */