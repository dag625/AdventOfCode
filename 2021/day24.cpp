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
    using state = std::array<int, 4>;

    struct result {
        int z;
        int input;

        bool operator==(const result& rhs) const {
            //To be used by std::unique, only care about state
            return z == rhs.z;
        }
        bool operator<(const result& rhs) const {
            //To be used by std::sort, want to group states together, with the highest input for a given state at the front
            return std::tie(z, input) < std::tie(z, rhs.input);
        }
    };

    struct multi_result {
        int z;
        std::array<int, 14> input;
        int num_inputs = 0;

        bool operator==(const multi_result& rhs) const {
            //To be used by std::unique, only care about state
            return z == rhs.z;
        }
        bool operator<(const multi_result& rhs) const {
            //To be used by std::sort, want to group states together, with the highest input for a given state at the front
            bool stateless = z < rhs.z;
            if (stateless) { return true; }
            else if (*this == rhs) {
                //We assume the lengths of the inputs are the same.
                const auto diff = std::mismatch(input.begin(), input.begin() + num_inputs, rhs.input.begin(), rhs.input.begin() + num_inputs);
                if (diff.first == input.end()) {
                    return false;
                }
                else {
                    return *diff.first < *diff.second;
                }
            }
            else {
                return false;
            }
        }
    };

    std::vector<program> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_24_input.txt");
        auto full = lines | std::views::transform([](std::string_view s){ return parse_instruction(s); }) | to<std::vector<instruction>>();
        std::vector<program> retval;
        retval.reserve(14);
        auto start = full.begin();
        for (auto current = start + 1; current != full.end(); ++current) {
            if (current->operation == op::input) {
                retval.emplace_back(start, current);
                start = current;
            }
        }
        retval.emplace_back(start, full.end());
        return retval;
    }

    int b_value(int b, bool is_b_lit, const state& s) { return is_b_lit ? b : s[b]; }

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

    std::array<int, 3> extract_constants(const program& p) {
        return {p[4].b, p[5].b, p[15].b};
    }

    int calc_next_z(int z, const int input, const std::array<int, 3>& constants) {
        int z2 = z * 26;
        z2 += input + constants[2];
        z /= constants[0];
        z *= 26;
        z += input + constants[2];
        return z;
    }

    template <typename T>
    void consolidate(std::vector<T>& states) {
        std::sort(states.begin(), states.end());
        states.erase(std::unique(states.begin(), states.end()), states.end());
    }

    std::vector<result> run_program_for_all_inputs(const program& p, const int prev_z) {
        constexpr int begin = 1, end = 9;
        std::vector<result> retval;
        retval.reserve(end - begin + 1);
        const auto constants = extract_constants(p);
        for (int i = begin; i <= end; ++i) {
            retval.push_back({calc_next_z(prev_z, i, constants), i});
        }
        consolidate(retval);
        return retval;
    }

    std::vector<multi_result> run_all_programs(const std::vector<program>& all) {
        std::vector<multi_result> results;
        results.push_back({});
        for (const auto& p : all) {
            std::vector<multi_result> new_results;
            for (const auto& s : results) {
                const auto mid = new_results.size();
                auto res = run_program_for_all_inputs(p, s.z);
                for (const auto& r : res) {
                    new_results.push_back({r.z, s.input, s.num_inputs + 1});
                    new_results.back().input[s.num_inputs] = r.input;
                }
                std::inplace_merge(new_results.begin(), new_results.begin() + static_cast<std::ptrdiff_t>(mid), new_results.end());
            }
            new_results.erase(std::unique(new_results.begin(), new_results.end()), new_results.end());
            results.swap(new_results);
        }
        return results;
    }

    void remove_invalid(std::vector<multi_result>& results) {
        results.erase(std::remove_if(results.begin(), results.end(), [](const multi_result& r){
            return r.z != 0;
        }), results.end());
    }

    std::string to_string(const multi_result& r) {
        std::stringstream retval;
        for (auto v : r.input) {
            retval << v;
        }
        return retval.str();
    }

    /*

    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto result = run_all_programs(input);
        remove_invalid(result);
        return to_string(result.front());
    }

    /*

    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    aoc::registration r{2021, 24, part_1, part_2};

//    TEST_SUITE("2021_day24") {
//        TEST_CASE("2021_day24:example") {
//
//        }
//    }

}