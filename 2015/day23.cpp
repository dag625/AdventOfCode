//
// Created by Dan on 11/27/2022.
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

    enum class instruction_code : int {
        half,
        triple,
        increment,
        jump,
        jump_if_even,
        jump_if_one
    };

    struct instruction {
        instruction_code op;
        std::optional<int> reg;
        std::optional<int> offset;
    };

    instruction parse_instr(std::string_view s) {
        instruction_code op = instruction_code::half;
        std::optional<int> reg;
        std::optional<int> offset;
        if (s.starts_with("hlf ")) {
            op = instruction_code::half;
            s.remove_prefix(4);
            reg = s.front() - 'a';
        }
        else if (s.starts_with("tpl ")) {
            op = instruction_code::triple;
            s.remove_prefix(4);
            reg = s.front() - 'a';
        }
        else if (s.starts_with("inc ")) {
            op = instruction_code::increment;
            s.remove_prefix(4);
            reg = s.front() - 'a';
        }
        else if (s.starts_with("jmp ")) {
            op = instruction_code::jump;
            s.remove_prefix(4);
            offset = parse_allow_plus<int>(s);
        }
        else if (s.starts_with("jie ")) {
            op = instruction_code::jump_if_even;
            s.remove_prefix(4);
            reg = s.front() - 'a';
            s.remove_prefix(3);
            offset = parse_allow_plus<int>(s);
        }
        else if (s.starts_with("jio ")) {
            op = instruction_code::jump_if_one;
            s.remove_prefix(4);
            reg = s.front() - 'a';
            s.remove_prefix(3);
            offset = parse_allow_plus<int>(s);
        }
        else {
            throw std::runtime_error{fmt::format("Invalid instruction code:  {}", s)};
        }
        return {op, reg, offset};
    }

    std::vector<instruction> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2015" / "day_23_input.txt");
        return lines | std::views::transform(parse_instr) | to<std::vector<instruction>>();
    }

    struct computer {
        std::array<uint64_t, 2> registers{};
        int ip = 0;

        bool run(const std::vector<instruction>& program) {
            if (ip < 0 || ip >= program.size()) {
                return false;
            }
            switch (program[ip].op) {
            case instruction_code::half:
                registers[*program[ip].reg] /= 2;
                break;
            case instruction_code::triple:
                registers[*program[ip].reg] *= 3;
                break;
            case instruction_code::increment:
                registers[*program[ip].reg] += 1;
                break;
            case instruction_code::jump:
                ip += *program[ip].offset - 1;
                break;
            case instruction_code::jump_if_even:
                ip += registers[*program[ip].reg] % 2 == 0 ? *program[ip].offset - 1 : 0;
                break;
            case instruction_code::jump_if_one:
                ip += registers[*program[ip].reg] == 1 ? *program[ip].offset - 1 : 0;
                break;
            }
            ++ip;
            return true;
        }
    };

    /*
    --- Day 23: Opening the Turing Lock ---
    Little Jane Marie just got her very first computer for Christmas from some unknown benefactor. It comes with instructions and an example program, but the computer itself seems to be malfunctioning. She's curious what the program does, and would like you to help her run it.

    The manual explains that the computer supports two registers and six instructions (truly, it goes on to remind the reader, a state-of-the-art technology). The registers are named a and b, can hold any non-negative integer, and begin with a value of 0. The instructions are as follows:

    hlf r sets register r to half its current value, then continues with the next instruction.
    tpl r sets register r to triple its current value, then continues with the next instruction.
    inc r increments register r, adding 1 to it, then continues with the next instruction.
    jmp offset is a jump; it continues with the instruction offset away relative to itself.
    jie r, offset is like jmp, but only jumps if register r is even ("jump if even").
    jio r, offset is like jmp, but only jumps if register r is 1 ("jump if one", not odd).
    All three jump instructions work with an offset relative to that instruction. The offset is always written with a prefix + or - to indicate the direction of the jump (forward or backward, respectively). For example, jmp +1 would simply continue with the next instruction, while jmp +0 would continuously jump back to itself forever.

    The program exits when it tries to run an instruction beyond the ones defined.

    For example, this program sets a to 2, because the jio instruction causes it to skip the tpl instruction:

    inc a
    jio a, +2
    tpl a
    inc a
    What is the value in register b when the program in your puzzle input is finished executing?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        computer c;
        while (c.run(input)) {}
        return std::to_string(c.registers[1]);
    }

    /*
    --- Part Two ---
    The unknown benefactor is very thankful for releasi-- er, helping little Jane Marie with her computer. Definitely not to distract you, what is the value in register b after the program is finished executing if register a starts as 1 instead?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        computer c;
        c.registers[0] = 1;
        while (c.run(input)) {}
        return std::to_string(c.registers[1]);
    }

    aoc::registration r{2015, 23, part_1, part_2};

//    TEST_SUITE("2015_day23") {
//        TEST_CASE("2015_day23:example") {
//
//        }
//    }

}