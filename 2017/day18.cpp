//
// Created by Dan on 12/12/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <array>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/18
     */

    struct semi_state {
        std::array<int64_t, 26> registers{};
        std::vector<int64_t> sent;
        int64_t ip = 0;
        int64_t num_sent = 0;
    };

    struct state {
        std::array<semi_state, 2> progs;
        std::vector<int64_t> recovered;
        int64_t last_sound = 0;

        void init_part2();
    };

    using operand = std::variant<char, int64_t>;

    int64_t& get_register(state& s, int prog, char r) {
        return s.progs[prog].registers[static_cast<int>(r - 'a')];
    }

    void state::init_part2() {
        get_register(*this, 1, 'p') = 1;
    }

    int64_t get_value(state& s, int prog, operand op) {
        if (std::holds_alternative<char>(op)) {
            return get_register(s, prog, std::get<char>(op));
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

    struct sound_ins {
        operand op;

        void run1(state& s) const {
            s.last_sound = get_value(s, 0, op);
            ++s.progs[0].ip;
        }

        bool run2(state& s, const int prog) const {
            s.progs[prog].sent.push_back(get_value(s, prog, op));
            ++s.progs[prog].num_sent;
            ++s.progs[prog].ip;
            return false;
        }
    };

    sound_ins parse_snd(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parse_op(parts[1])};
    }

    struct set_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, 0, op1) = get_value(s, 0, op2);
            ++s.progs[0].ip;
        }

        bool run2(state& s, const int prog) const {
            get_register(s, prog, op1) = get_value(s, prog, op2);
            ++s.progs[prog].ip;
            return false;
        }
    };

    set_ins parse_set(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct add_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, 0, op1) += get_value(s, 0, op2);
            ++s.progs[0].ip;
        }

        bool run2(state& s, const int prog) const {
            get_register(s, prog, op1) += get_value(s, prog, op2);
            ++s.progs[prog].ip;
            return false;
        }
    };

    add_ins parse_add(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct mul_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, 0, op1) *= get_value(s, 0, op2);
            ++s.progs[0].ip;
        }

        bool run2(state& s, const int prog) const {
            get_register(s, prog, op1) *= get_value(s, prog, op2);
            ++s.progs[prog].ip;
            return false;
        }
    };

    mul_ins parse_mul(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct mod_ins {
        char op1 = '\0';
        operand op2;

        void run1(state& s) const {
            get_register(s, 0, op1) %= get_value(s, 0, op2);
            ++s.progs[0].ip;
        }

        bool run2(state& s, const int prog) const {
            get_register(s, prog, op1) %= get_value(s, prog, op2);
            ++s.progs[prog].ip;
            return false;
        }
    };

    mod_ins parse_mod(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0], parse_op(parts[2])};
    }

    struct recover_ins {
        char op = '\0';

        void run1(state& s) const {
            if (get_register(s, 0, op) != 0) {
                s.recovered.push_back(s.last_sound);
            }
            ++s.progs[0].ip;
        }

        bool run2(state& s, const int prog) const {
            const auto other = (prog + 1) % 2;
            if (s.progs[other].sent.empty()) {
                return true;
            }
            const auto val = s.progs[other].sent.front();
            s.progs[other].sent.erase(s.progs[other].sent.begin());
            get_register(s, prog, op) = val;
            ++s.progs[prog].ip;
            return false;
        }
    };

    recover_ins parse_rcv(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[1][0]};
    }

    struct jump_ins {
        operand op1;
        operand op2;

        void run1(state& s) const {
            if (get_value(s, 0, op1) > 0) {
                s.progs[0].ip += get_value(s, 0, op2);
            }
            else {
                ++s.progs[0].ip;
            }
        }

        bool run2(state& s, const int prog) const {
            if (get_value(s, prog, op1) > 0) {
                s.progs[prog].ip += get_value(s, prog, op2);
            }
            else {
                ++s.progs[prog].ip;
            }
            return false;
        }
    };

    jump_ins parse_jgz(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parse_op(parts[1]), parse_op(parts[2])};
    }

    using instruction = std::variant<sound_ins, set_ins, add_ins, mul_ins, mod_ins, recover_ins, jump_ins>;

    instruction parse_ins(std::string_view s) {
        if (s.starts_with("snd")) {
            return parse_snd(s);
        }
        else if (s.starts_with("set")) {
            return parse_set(s);
        }
        else if (s.starts_with("add")) {
            return parse_add(s);
        }
        else if (s.starts_with("mul")) {
            return parse_mul(s);
        }
        else if (s.starts_with("mod")) {
            return parse_mod(s);
        }
        else if (s.starts_with("rcv")) {
            return parse_rcv(s);
        }
        else {
            return parse_jgz(s);
        }
    }

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    int run_until_stopped(state& s, const int prog, const std::vector<instruction>& ins) {
        int num_steps = 0;
        while (s.progs[prog].ip >= 0 && s.progs[prog].ip < ins.size() &&
               !std::visit([&s, prog](const auto& i){ return i.run2(s, prog); }, ins[s.progs[prog].ip]))
        {
            ++num_steps;
        }
        return num_steps;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        state s{};
        while (s.progs[0].ip >= 0 && s.progs[0].ip < input.size() && s.recovered.empty()) {
            std::visit([&s](const auto& i){ i.run1(s); }, input[s.progs[0].ip]);
        }
        return std::to_string(s.recovered.front());
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        state s{};
        s.init_part2();
        int total_steps = 1;
        while (total_steps > 0) {
            total_steps = 0;
            total_steps += run_until_stopped(s, 0, input);
            total_steps += run_until_stopped(s, 1, input);
        }
        return std::to_string(s.progs[1].num_sent);
    }

    aoc::registration r{2017, 18, part_1, part_2};

    TEST_SUITE("2017_day18") {
        TEST_CASE("2017_day18:example") {
            const std::vector<std::string> lines {
                    "set a 1",
                    "add a 2",
                    "mul a a",
                    "mod a 5",
                    "snd a",
                    "set a 0",
                    "rcv a",
                    "jgz a -1",
                    "set a 1",
                    "jgz a -2"
            };
            const auto input = lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
            state s{};
            while (s.progs[0].ip >= 0 && s.progs[0].ip < input.size() && s.recovered.empty()) {
                std::visit([&s](const auto& i){ i.run1(s); }, input[s.progs[0].ip]);
            }
            CHECK(!s.recovered.empty());
            CHECK_EQ(s.recovered.front(), 4);
        }
    }

} /* namespace <anon> */