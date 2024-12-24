//
// Created by Dan on 12/24/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <utility>
#include <vector>
#include <bitset>
#include <iostream>
#include <bit>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/24
     */

    using int_t = unsigned long long;

    struct wire {
        std::string name;
        std::optional<bool> value;

        wire() = default;
        wire(std::string n) : name{std::move(n)} {}
        wire(std::string_view n, std::optional<bool> v) : name{n}, value{v} {}

        auto operator<=>(const wire& rhs) const { return name <=> rhs.name; }
        bool operator==(const wire& rhs) const { return name == rhs.name; }

        auto operator<=>(const std::string& rhs) const { return name <=> rhs; }
        bool operator==(const std::string& rhs) const { return name == rhs; }
    };

    enum class gate_type {
        AND,
        OR,
        XOR
    };

    using gate_func = std::optional<bool> (*)(std::optional<bool>, std::optional<bool>);

    std::optional<bool> and_func(std::optional<bool> a, std::optional<bool> b) { if (a && b) { return *a && *b; } else { return std::nullopt; } }
    std::optional<bool> or_func(std::optional<bool> a, std::optional<bool> b) { if (a && b) { return *a || *b; } else { return std::nullopt; } }
    std::optional<bool> xor_func(std::optional<bool> a, std::optional<bool> b) { if (a && b) { return *a != *b; } else { return std::nullopt; } }

    struct gate {
        std::string in1;
        std::string in2;
        std::string out;
        gate_type type = gate_type::AND;
        gate_func func = &and_func;

        gate() = default;
        gate(std::string_view i1, std::string_view i2, std::string_view o, std::string_view op) : in1{i1}, in2{i2}, out{o} {
            if (op == "AND") {
                type = gate_type::AND;
                func = &and_func;
            }
            else if (op == "OR") {
                type = gate_type::OR;
                func = &or_func;
            }
            else {
                type = gate_type::XOR;
                func = &xor_func;
            }
        }

        auto operator<=>(const gate& rhs) const { return std::tie(in1, in2, out, type) <=> std::tie(rhs.in1, rhs.in2, rhs.out, rhs.type); }
        bool operator==(const gate& rhs) const { return std::tie(in1, in2, out, type) == std::tie(rhs.in1, rhs.in2, rhs.out, rhs.type); }

        [[nodiscard]] bool has_in(const std::string& a) const {
            return in1 == a || in2 == a;
        }

        [[nodiscard]] bool matches_in(const std::string& a, const std::string& b) const {
            return (in1 == a && in2 == b) || (in1 == b && in2 == a);
        }
    };

    wire parse_wire(std::string_view s) {
        const auto parts = split(s, ": ");
        return {parts[0], parts[1].front() == '1'};
    }

    gate parse_gate(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parts[0], parts[2], parts[4], parts[1]};
    }

    struct input_data {
        std::vector<wire> wires;
        std::vector<gate> gates;
    };

    input_data get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_24_input.txt");
        const auto blank = std::find(lines.begin(), lines.end(), "");
        const auto num_wires = std::distance(lines.begin(), blank);
        return {lines | std::views::take(num_wires) | std::views::transform(&parse_wire) | std::ranges::to<std::vector>(),
                lines | std::views::drop(num_wires + 1) | std::views::transform(&parse_gate) | std::ranges::to<std::vector>()};
    }

    void add_uninit_wire(std::vector<wire>& wires, const std::string& name) {
        const auto found = std::find(wires.begin(), wires.end(), name);
        if (found == wires.end()) {
            wires.emplace_back(name);
        }
    }

    void add_uninit_wires(std::vector<wire>& wires, const std::vector<gate>& gates) {
        for (const auto& g : gates) {
            add_uninit_wire(wires, g.in1);
            add_uninit_wire(wires, g.in2);
            add_uninit_wire(wires, g.out);
        }
    }

    bool run_logic(std::vector<wire>& wires, const gate& g) {
        const auto found_in1 = std::find(wires.begin(), wires.end(), g.in1);
        const auto found_in2 = std::find(wires.begin(), wires.end(), g.in2);
        const auto found_out = std::find(wires.begin(), wires.end(), g.out);
        if (!found_out->value.has_value()) {
            //std::cout << fmt::format("{} + {} -> {} Test {} and {}", g.in1, g.in2, g.out, found_in1->value.has_value(), found_in2->value.has_value()) << std::endl;
            found_out->value = g.func(found_in1->value, found_in2->value);
            return found_out->value.has_value();
        }
        else {
            //std::cout << fmt::format("{} + {} -> {} OK", g.in1, g.in2, g.out) << std::endl;
            return true;
        }
    }

    bool run_logic(std::vector<wire>& wires, const std::vector<gate>& gates) {
        bool all_good = true;
        for (const auto& g : gates) {
            all_good = run_logic(wires, g) && all_good;
        }
        return all_good;
    }

    int_t get_value(const std::vector<wire>& wires, const char reg = 'z') {
        std::bitset<std::numeric_limits<int_t>::digits> bits{};
        for (const auto& w : wires) {
            if (w.name.front() == reg) {
                std::string_view n {w.name};
                n.remove_prefix(1);
                const auto bitnum = parse<int>(n);
                bits.set(bitnum, *w.value);
            }
        }
        return bits.to_ullong();
    }

    int_t calc_value(std::vector<wire> wires, const std::vector<gate>& gates) {
        std::sort(wires.begin(), wires.end());
        while (!run_logic(wires, gates)) {}
        const auto val = get_value(wires);
        return val;
    }

    struct inout {
        char type = '\0'; //x, y, or z
        int num = 0;
    };

    std::optional<inout> parse_inout(std::string_view wire) {
        const char c = wire.front();
        if (c == 'x' || c == 'y' || c == 'z') {
            wire.remove_prefix(1);
            return inout{c, parse<int>(wire)};
        }
        else {
            return std::nullopt;
        }
    }

    struct input_gate {
        int num = -1;
        std::string out;
        gate_type type = gate_type::OR;
        gate_func func = &or_func;
    };

    struct sorted_gates {
        std::vector<input_gate> input;
        std::vector<gate> and_gates;
        std::vector<gate> xor_gates;
        std::vector<gate> or_gates;
    };

    sorted_gates sort_gates(const std::vector<gate>& gates) {
        sorted_gates retval;
        for (const auto& g : gates) {
            const auto in1 = parse_inout(g.in1);
            const auto in2 = parse_inout(g.in2);
            if (in1 && in2) {
                retval.input.emplace_back(in1->num, g.out, g.type, g.func);
            }
            else if (g.type == gate_type::AND) {
                retval.and_gates.push_back(g);
            }
            else if (g.type == gate_type::XOR) {
                retval.xor_gates.push_back(g);
            }
            else {
                retval.or_gates.push_back(g);
            }
        }
        return retval;
    }

    int find_next_z(const std::vector<gate>& gates, const std::string& z, const int depth = 3) {
        const auto inout = parse_inout(z);
        if (inout) {
            return inout->num;
        }
        else if (depth < 0) {
            return 1000;
        }
        else {
            std::vector<gate> found;//There are two that will have z as input, and only one will hit our destination
            std::copy_if(gates.begin(), gates.end(), std::back_inserter(found), [&z](const gate& g){ return g.has_in(z); });
            std::vector<int> vals;
            std::transform(found.begin(), found.end(), std::back_inserter(vals), [depth, & gates](const gate& g){ return find_next_z(gates, g.out, depth - 1); });
            return *std::min_element(vals.begin(), vals.end());
        }
    }

    std::vector<gate> fix(std::vector<gate> gates, const std::vector<std::string>& not_zs, const std::vector<std::string>& bad_zs) {
        for (const auto& notz : not_zs) {
            const int next_z = find_next_z(gates, notz);
            const auto swap_z = fmt::format("z{:02}", (next_z - 1));
            if (std::find(bad_zs.begin(), bad_zs.end(), swap_z) == bad_zs.end()) {
                throw std::logic_error{fmt::format("Failed to find Z {} to swap with {} in list:  {}", swap_z, notz, fmt::join(bad_zs, ","))};
            }
            for (auto& g : gates) {
                if (g.out == swap_z) {
                    g.out = notz;
                }
                else if (g.out == notz) {
                    g.out = swap_z;
                }
            }
        }
        return gates;
    }

    std::vector<std::string> check_adders(const std::vector<gate>& gates, const std::vector<wire>& wires) {
        int max_z = 0;
        for (const auto& g : gates) {
            const auto zout = parse_inout(g.out);
            if (zout && zout->num > max_z) {
                max_z = zout->num;
            }
        }
        const auto max_z_str = fmt::format("z{:02}", max_z);

        const auto sorted = sort_gates(gates);

        bool found_last_z = false;
        std::vector<std::string> bad_zs;
        for (const auto& g : sorted.or_gates) {
            const auto outv = parse_inout(g.out);
            if (outv && outv->num != max_z) {
                bad_zs.push_back(g.out);
            }
            else if (outv && outv->num == max_z) {
                found_last_z = true;
            }
        }
        if (!found_last_z) {
            bad_zs.push_back(max_z_str);
        }

        for (const auto& g : sorted.and_gates) {
            const auto outv = parse_inout(g.out);
            if (outv) {
                //All zXX must be outputs of XOR except the last z.
                bad_zs.push_back(g.out);
            }
        }

        for (const auto& g : sorted.input) {
            const auto outv = parse_inout(g.out);
            if (outv && outv->num != 0) {
                //Only with N=0 can xN * yN => zN
                bad_zs.push_back(g.out);
            }
        }

        std::vector<std::string> not_zs;
        for (const auto& g : sorted.xor_gates) {
            const auto in1v = parse_inout(g.in1);
            const auto in2v = parse_inout(g.in2);
            const auto outv = parse_inout(g.out);

            if (((!in1v || !in2v) && !outv) || (in1v && in2v && outv && outv->num != 0)) {
                //Either both ins have to be x/y or the out has to be z, but only N = 0 is allowed for xN ^ yN = zN.
                not_zs.push_back(g.out);
            }
        }

        //At this point we have 3 values each in not_zs and bad_zs, which is 3 of the 4 swapped pairs.  So we fix them
        //so that we can find where the last swapped inputs are.
        auto fixed_gates = fix(gates, not_zs, bad_zs);
        const auto orig_val = calc_value(wires, gates);
        const auto fixed_val = calc_value(wires, fixed_gates);
        const auto xor_vals = orig_val ^ fixed_val;

        const auto x_val = get_value(wires, 'x');
        const auto y_val = get_value(wires, 'y');
        const auto expected = x_val + y_val;
        const auto xor_expected = expected ^ fixed_val;
        const auto failing_carry = std::countr_zero(xor_expected);
        //We now know where the last swap must be because it causes the xor of the actual answer and our calculated
        //answer to begin differing after failing_carry bits.  From inspection of the input the swap is that the output
        //or XORing and ANDing the x and y bits is swapped here.  This can be seen because the output of the AND should
        //only be the input to one gate and the output of the XOR should be the input for two, but this is not the case.

        const auto xin = fmt::format("x{}", failing_carry);
        const auto yin = fmt::format("y{}", failing_carry);

        std::vector<std::string> wrong;
        for (const auto& g : fixed_gates) {
            if (g.matches_in(xin, yin)) {
                wrong.push_back(g.out);
            }
        }

        //Fix it and sanity check.
        for (auto& g : fixed_gates) {
            if (g.matches_in(xin, yin)) {
                if (g.out == wrong[0]) {
                    g.out = wrong[1];
                }
                else if (g.out == wrong[1]) {
                    g.out = wrong[0];
                }
            }
        }
        const auto fixed2_val = calc_value(wires, fixed_gates);
        const auto xor2_expected = expected ^ fixed2_val;
        if (xor2_expected) {
            throw std::logic_error{fmt::format("Final fix didn't work.  XOR is:  {:x}", xor2_expected)};
        }

        wrong.insert(wrong.end(), not_zs.begin(), not_zs.end());
        wrong.insert(wrong.end(), bad_zs.begin(), bad_zs.end());
        return wrong;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        auto [wires, gates] = get_input(input_dir);
        add_uninit_wires(wires, gates);
        const auto val = calc_value(wires, gates);
        return std::to_string(val);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        auto [wires, gates] = get_input(input_dir);
        add_uninit_wires(wires, gates);
        auto result = check_adders(gates, wires);
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return fmt::format("{}", fmt::join(result, ","));
    }

    aoc::registration r{2024, 24, part_1, part_2};

//    TEST_SUITE("2024_day24") {
//        TEST_CASE("2024_day24:example") {
//
//        }
//    }

} /* namespace <anon> */