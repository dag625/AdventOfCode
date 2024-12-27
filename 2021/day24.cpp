//
// Created by Dan on 12/24/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    enum class op {
        input,
        add,
        multiply,
        divide,
        modulo,
        equals
    };

    struct instruction {
        op operation;
        int a;
        int b = 0;
        bool is_b_literal = false;
    };

    instruction parse_instruction(std::string_view s) {
        const auto parts = split(s, ' ');

        op oper = op::equals;
        if (parts[0] == "inp") {
            oper = op::input;
        }
        else if (parts[0] == "add") {
            oper = op::add;
        }
        else if (parts[0] == "mul") {
            oper = op::multiply;
        }
        else if (parts[0] == "div") {
            oper = op::divide;
        }
        else if (parts[0] == "mod") {
            oper = op::modulo;
        }

        int a = parts[1][0] - 'w';
        int b = 0;
        bool is_b_literal = false;
        if (oper != op::input) {
            if (parts[2][0] <= 'z' && parts[2][0] >= 'w') {
                b = parts[2][0] - 'w';
            }
            else {
                b = parse<int>(parts[2]);
                is_b_literal = true;
            }
        }
        return {oper, a, b, is_b_literal};
    }

    using program = std::vector<instruction>;
    using constants = std::array<int, 3>;

    /*
     * Solution derived from explanation at:  https://www.reddit.com/r/adventofcode/comments/rnejv5/comment/hps5hgw/?utm_source=share&utm_medium=web2x&context=3
     *
     * The solution revolves around the fact that each program is of the form:
     *
     * inp w
     * mul x 0
     * add x z
     * mod x 26
     * div z {A}
     * add x {B}
     * eql x w
     * eql x 0
     * mul y 0
     * add y 25
     * mul y x
     * add y 1
     * mul z y
     * mul y 0
     * add y w
     * add y {C}
     * mul y x
     * add z y
     *
     * The values in {} are variable constants and the differences between each program and each input more
     * generally.  Constant A is always 1 or 26.  When the constant A is 1, B is always in the range [11,15];
     * when A is 26, B is always negative.  Constant C is always a small positive number.
     *
     * The w values are always the program input (e.g. the latest digit).  Otherwise the following operations
     * are performed (p = previous z value from last program, ==0 for first run):
     *
     * x = w != (B + (p % 26)) ? 1 : 0
     * z = p / A
     * z *= 25*x + 1
     * z += w + C
     *
     * If A==1, then B is in [11,15] and because w is the input which is always [1,9], when A==1 then x==1
     * (B itself is always not equal to w, so it doesn't matter what (p%26) is).  For A==26, remember our
     * goal is to make z==0 at the end of everything.  Note that if when A==26 we allow x==1, then z always
     * grows (w and C are always >0).  However, if we force x==0 when A==26 the middle two equations form a
     * push/pop mechanism for a "stack" of base 26 values stored in z.  When A==26 and x==0 the middle equations
     * are "z = p / 26; z *= 1"; when A==1 and x==1 the middle equations are "z = p; z *= 26".  A scan of the
     * input shows that for all C, (w+C) < 26.  Moreover, the input contains as many programs with A==1 as it
     * does A==26, so everything we push (A==1) will be popped (A==26), and the first program does a push and
     * the last does a pop.
     *
     * So to get the inputs, we see that we need x==0 when A==26, which means w == (B + p % 26).  "p % 26" is
     * the value to be popped off the stack, and this must be equal to "w-B".  If the input value which produced
     * the value being popped off the stack is w_o and the C constant at that time is C_o, then we have
     * "w=w_o + C_o + B".  So we can walk the stack, try all valid w_o, keep the w and w_o values which are both
     * valid, keep the maximum w_o and w (the maximum w_o and w will always correspond because the relationship
     * is linear with a slope of +1).
     */

    constexpr int A_POP = 26;

    constants extract_constants(const program& p) {
        return {p[4].b, p[5].b, p[15].b};
    }

    std::pair<std::vector<program>, std::vector<constants>> get_input(const std::vector<std::string>& lines) {
        auto full = lines | std::views::transform([](std::string_view s){ return parse_instruction(s); }) | to<std::vector<instruction>>();
        std::vector<program> programs;
        programs.reserve(14);
        auto start = full.begin();
        for (auto current = start + 1; current != full.end(); ++current) {
            if (current->operation == op::input) {
                programs.emplace_back(start, current);
                start = current;
            }
        }
        programs.emplace_back(start, full.end());
        return {programs, programs | std::views::transform([](const program& p){ return extract_constants(p); }) | to<std::vector<constants>>()};
    }

    struct input_data {
        int min = -1;
        int max = -1;
    };

    void calculate(std::array<input_data, 14>& inputs, const std::vector<constants>& programs, int push, int pop) {
//        fmt::print("Push = ({:>3}, {:>3}, {:>3}) @ {:>2}; Pop = ({:>3}, {:>3}, {:>3}) @ {:>2}\n",
//                   programs[push][0], programs[push][1], programs[push][2], push,
//                   programs[pop][0], programs[pop][1], programs[pop][2], pop);
        const int diff = programs[pop][1] + programs[push][2];
        constexpr int min_in = 1, max_in = 9;
        for (int push_in = max_in; push_in >= min_in; --push_in) {
            int pop_in = push_in + diff;
            if (pop_in >= min_in && pop_in <= max_in) {
                if (inputs[push].max == -1) {
                    inputs[push].max = push_in;
                    inputs[pop].max = pop_in;
                }
                inputs[push].min = push_in;
                inputs[pop].min = pop_in;
            }
        }
    }

    int walk(std::array<input_data, 14>& inputs, const std::vector<constants>& programs, int next = 0, int last_push = -1) {
        while (next < programs.size()) {
            if (programs[next][0] == A_POP) {
                calculate(inputs, programs, last_push, next);
                return next + 1;
            }
            else {
                next = walk(inputs, programs, next + 1, next);
            }
        }
        return next;
    }

    std::pair<std::string, std::string> inputs_to_string(const std::array<input_data, 14>& inputs) {
        std::pair<std::string, std::string> retval;
        for (const auto& d : inputs) {
            retval.first += static_cast<char>(d.min + '0');
            retval.second += static_cast<char>(d.max + '0');
        }
        return retval;
    }

    using state = std::array<int64_t, 4>;

    int64_t b_value(int b, bool is_b_lit, const state& s) { return is_b_lit ? b : s[b]; }

    void evaluate(const instruction& ins, state& s, int input) {
        switch (ins.operation) {
            case op::input:    s[ins.a] = input; break;
            case op::add:      s[ins.a] = s[ins.a] + b_value(ins.b, ins.is_b_literal, s); break;
            case op::multiply: s[ins.a] = s[ins.a] * b_value(ins.b, ins.is_b_literal, s); break;
            case op::divide:   s[ins.a] = s[ins.a] / b_value(ins.b, ins.is_b_literal, s); break;
            case op::modulo:   s[ins.a] = s[ins.a] % b_value(ins.b, ins.is_b_literal, s); break;
            case op::equals:   s[ins.a] = s[ins.a] == b_value(ins.b, ins.is_b_literal, s) ? 1 : 0; break;
            default: break;
        }
    }

    state run_program(const program& p, const state& start, int input) {
        state s = start;
        for (const auto& ins : p) {
            evaluate(ins, s, input);
        }
        return s;
    }

    state run_all_programs(const std::vector<program>& programs, std::string_view input) {
        state current{};
        if (input.size() != 14) {
            current[3] = -1;
            return current;
        }
        int idx = 0;
        for (const auto& p : programs) {
            current = run_program(p, current, input[idx] - '0');
            ++idx;
        }
        return current;
    }

    /*
    --- Day 24: Arithmetic Logic Unit ---
    Magic smoke starts leaking from the submarine's arithmetic logic unit (ALU). Without the ability to perform basic arithmetic and logic functions, the submarine can't produce cool patterns with its Christmas lights!

    It also can't navigate. Or run the oxygen system.

    Don't worry, though - you probably have enough oxygen left to give you enough time to build a new ALU.

    The ALU is a four-dimensional processing unit: it has integer variables w, x, y, and z. These variables all start with the value 0. The ALU also supports six instructions:

    inp a - Read an input value and write it to variable a.
    add a b - Add the value of a to the value of b, then store the result in variable a.
    mul a b - Multiply the value of a by the value of b, then store the result in variable a.
    div a b - Divide the value of a by the value of b, truncate the result to an integer, then store the result in variable a. (Here, "truncate" means to round the value toward zero.)
    mod a b - Divide the value of a by the value of b, then store the remainder in variable a. (This is also called the modulo operation.)
    eql a b - If the value of a and b are equal, then store the value 1 in variable a. Otherwise, store the value 0 in variable a.
    In all of these instructions, a and b are placeholders; a will always be the variable where the result of the operation is stored (one of w, x, y, or z), while b can be either a variable or a number. Numbers can be positive or negative, but will always be integers.

    The ALU has no jump instructions; in an ALU program, every instruction is run exactly once in order from top to bottom. The program halts after the last instruction has finished executing.

    (Program authors should be especially cautious; attempting to execute div with b=0 or attempting to execute mod with a<0 or b<=0 will cause the program to crash and might even damage the ALU. These operations are never intended in any serious ALU program.)

    For example, here is an ALU program which takes an input number, negates it, and stores it in x:

    inp x
    mul x -1
    Here is an ALU program which takes two input numbers, then sets z to 1 if the second input number is three times larger than the first input number, or sets z to 0 otherwise:

    inp z
    inp x
    mul z 3
    eql z x
    Here is an ALU program which takes a non-negative integer as input, converts it into binary, and stores the lowest (1's) bit in z, the second-lowest (2's) bit in y, the third-lowest (4's) bit in x, and the fourth-lowest (8's) bit in w:

    inp w
    add z w
    mod z 2
    div w 2
    add y w
    mod y 2
    div w 2
    add x w
    mod x 2
    div w 2
    mod w 2
    Once you have built a replacement ALU, you can install it in the submarine, which will immediately resume what it was doing when the ALU failed: validating the submarine's model number. To do this, the ALU will run the MOdel Number Automatic Detector program (MONAD, your puzzle input).

    Submarine model numbers are always fourteen-digit numbers consisting only of digits 1 through 9. The digit 0 cannot appear in a model number.

    When MONAD checks a hypothetical fourteen-digit model number, it uses fourteen separate inp instructions, each expecting a single digit of the model number in order of most to least significant. (So, to check the model number 13579246899999, you would give 1 to the first inp instruction, 3 to the second inp instruction, 5 to the third inp instruction, and so on.) This means that when operating MONAD, each input instruction should only ever be given an integer value of at least 1 and at most 9.

    Then, after MONAD has finished running all of its instructions, it will indicate that the model number was valid by leaving a 0 in variable z. However, if the model number was invalid, it will leave some other non-zero value in z.

    MONAD imposes additional, mysterious restrictions on model numbers, and legend says the last copy of the MONAD documentation was eaten by a tanuki. You'll need to figure out what MONAD does some other way.

    To enable as many submarine features as possible, find the largest valid fourteen-digit model number that contains no 0 digits. What is the largest model number accepted by MONAD?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [programs, input] = get_input(lines);
        std::array<input_data, 14> inputs{};
        walk(inputs, input);
        const auto strs = inputs_to_string(inputs);
        const auto res = run_all_programs(programs, strs.second);
        if (res[3] != 0) {
            throw std::runtime_error{fmt::format("Invalid input '{}' had result:  {}", strs.second, res[3])};
        }
        return strs.second;
    }

    /*
    --- Part Two ---
    As the submarine starts booting up things like the Retro Encabulator, you realize that maybe you don't need all these submarine features after all.

    What is the smallest model number accepted by MONAD?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [programs, input] = get_input(lines);
        std::array<input_data, 14> inputs{};
        walk(inputs, input);
        const auto strs = inputs_to_string(inputs);
        const auto res = run_all_programs(programs, strs.first);
        if (res[3] != 0) {
            throw std::runtime_error{fmt::format("Invalid input '{}' had result:  {}", strs.first, res[3])};
        }
        return strs.first;
    }

    aoc::registration r{2021, 24, part_1, part_2};

//    TEST_SUITE("2021_day24") {
//        TEST_CASE("2021_day24:example") {
//
//        }
//    }

}