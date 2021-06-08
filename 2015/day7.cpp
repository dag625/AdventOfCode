//
// Created by Daniel Garcia on 4/27/21.
//

#include "registration.h"
#include "utilities.h"
#include "parse.h"

#include <doctest/doctest.h>

#include <string_view>
#include <regex>
#include <numeric>
#include <variant>

#include <iostream>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    struct input {
        uint16_t value;
    };

    struct and_or {
        std::string first;
        std::string second;
        bool is_and;
    };

    struct and_or_val {
        uint16_t first;
        std::string second;
        bool is_and;
    };

    struct shift {
        std::string src;
        uint16_t size;
        bool is_right;
    };

    struct pass {
        std::string src;
    };

    struct not_op {
        std::string src;
    };

    using gate_op = std::variant<input, and_or, and_or_val, shift, pass, not_op>;

    struct gate {
        std::string dest;
        std::optional<uint16_t> prev_value;
        std::optional<uint16_t> value;
        gate_op op;
        gate(std::string_view d, gate_op o) : dest{d}, prev_value{}, value{}, op{std::move(o)} {}
        void step() {
            prev_value = value;
            value = std::nullopt;
        }
        [[nodiscard]] bool unchanged() const {
            return prev_value.has_value() == value.has_value() && (!prev_value.has_value() || *prev_value == *value);
        }
    };

    std::regex regex_input {R"(^(\d+) -> ([a-z]+)$)"};
    std::regex regex_andor {R"(^([a-z]+) (AND|OR) ([a-z]+) -> ([a-z]+)$)"};
    std::regex regex_andor_val {R"(^(\d+) (AND|OR) ([a-z]+) -> ([a-z]+)$)"};
    std::regex regex_shift {R"(^([a-z]+) (R|L)SHIFT (\d+) -> ([a-z]+)$)"};
    std::regex regex_pass {R"(^([a-z]+) -> ([a-z]+)$)"};
    std::regex regex_not {R"(^NOT ([a-z]+) -> ([a-z]+)$)"};

    gate parse(std::string_view s) {
        std::match_results<std::string_view::iterator> match;
        if (std::regex_match(s.begin(), s.end(), match, regex_input) && match.size() == 3) {
            return gate{match[2].str(), input{aoc::parse<uint16_t>(match[1].str())}};
        }
        else if (std::regex_match(s.begin(), s.end(), match, regex_andor) && match.size() == 5) {
            return gate{match[4].str(), and_or{match[1].str(),
                                               match[3].str(),
                                               match[2].str() == "AND"sv}};
        }
        else if (std::regex_match(s.begin(), s.end(), match, regex_andor_val) && match.size() == 5) {
            return gate{match[4].str(), and_or_val{aoc::parse<uint16_t>(match[1].str()),
                                                   match[3].str(),
                                                   match[2].str() == "AND"sv}};
        }
        else if (std::regex_match(s.begin(), s.end(), match, regex_shift) && match.size() == 5) {
            return gate{match[4].str(), shift{match[1].str(),
                                              aoc::parse<uint16_t>(match[3].str()),
                                              match[2].str() == "R"sv}};
        }
        else if (std::regex_match(s.begin(), s.end(), match, regex_pass) && match.size() == 3) {
            return gate{match[2].str(), pass{match[1].str()}};
        }
        else if (std::regex_match(s.begin(), s.end(), match, regex_not) && match.size() == 3) {
            return gate{match[2].str(), not_op{match[1].str()}};
        }
        else {
            throw std::runtime_error{"Failed to parse gate entry."};
        }
    }

    std::optional<uint16_t> get_last(const std::vector<gate>& gates, const std::string& src) {
        auto found = std::find_if(gates.begin(), gates.end(), [&src](const gate& g){ return g.dest == src; });
        if (found != gates.end()) {
            return found->prev_value;
        }
        else {
            return std::nullopt;
        }
    }

    std::optional<uint16_t> evaluate(std::vector<gate>& gates, const input& op) {
        return op.value;
    }

    std::optional<uint16_t> evaluate(std::vector<gate>& gates, const and_or& op) {
        auto v1 = get_last(gates, op.first), v2 = get_last(gates, op.second);
        if (!v1 || !v2) {
            return std::nullopt;
        }
        else if (op.is_and) {
            return *v1 & *v2;
        }
        else {
            return *v1 | *v2;
        }
    }

    std::optional<uint16_t> evaluate(std::vector<gate>& gates, const and_or_val& op) {
        auto v2 = get_last(gates, op.second);
        if (!v2) {
            return std::nullopt;
        }
        else if (op.is_and) {
            return op.first & *v2;
        }
        else {
            return op.first | *v2;
        }
    }

    std::optional<uint16_t> evaluate(std::vector<gate>& gates, const shift& op) {
        auto v = get_last(gates, op.src);
        if (!v) {
            return std::nullopt;
        }
        else if (op.is_right) {
            auto ret = *v >> op.size;
            return ret;
        }
        else {
            auto ret = *v << op.size;
            return ret;
        }
    }

    std::optional<uint16_t> evaluate(std::vector<gate>& gates, const pass& op) {
        return get_last(gates, op.src);
    }

    std::optional<uint16_t> evaluate(std::vector<gate>& gates, const not_op& op) {
        auto v = get_last(gates, op.src);
        if (v) {
            return ~*v;
        }
        else {
            return std::nullopt;
        }
    }

    bool iterate(std::vector<gate>& gates, int iteration) {
        for (auto& g : gates) {
            g.step();
        }
        for (auto& g : gates) {
            g.value = std::visit([&gates](const auto& op){ return evaluate(gates, op); }, g.op);
        }
        return std::all_of(gates.begin(), gates.end(), [](const gate& g){ return g.unchanged(); });
    }

    uint16_t iterate_until_static(std::vector<gate>& gates, std::string_view ret_gate) {
        int iter = 1;
        while (!iterate(gates, iter++)) {}
        auto found = std::find_if(gates.begin(), gates.end(), [ret_gate](const gate& g){ return g.dest == ret_gate; });
        if (found != gates.end() && found->value) {
            return *found->value;
        }
        else {
            return 0;
        }
    }

    std::vector<gate> get_input(const std::filesystem::path& input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2015" / "day_7_input.txt");
        std::vector<gate> retval;
        retval.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse);
        return retval;
    }

    /*
    This year, Santa brought little Bobby Tables a set of wires and bitwise logic gates! Unfortunately, little Bobby is a little under the recommended age range, and he needs help assembling the circuit.

    Each wire has an identifier (some lowercase letters) and can carry a 16-bit signal (a number from 0 to 65535). A signal is provided to each wire by a gate, another wire, or some specific value. Each wire can only get a signal from one source, but can provide its signal to multiple destinations. A gate provides no signal until all of its inputs have a signal.

    The included instructions booklet describes how to connect the parts together: x AND y -> z means to connect wires x and y to an AND gate, and then connect its output to wire z.

    For example:

    123 -> x means that the signal 123 is provided to wire x.
    x AND y -> z means that the bitwise AND of wire x and wire y is provided to wire z.
    p LSHIFT 2 -> q means that the value from wire p is left-shifted by 2 and then provided to wire q.
    NOT e -> f means that the bitwise complement of the value from wire e is provided to wire f.
    Other possible gates include OR (bitwise OR) and RSHIFT (right-shift). If, for some reason, you'd like to emulate the circuit instead, almost all programming languages (for example, C, JavaScript, or Python) provide operators for these gates.

    For example, here is a simple circuit:

    123 -> x
    456 -> y
    x AND y -> d
    x OR y -> e
    x LSHIFT 2 -> f
    y RSHIFT 2 -> g
    NOT x -> h
    NOT y -> i
    After it is run, these are the signals on the wires:

    d: 72
    e: 507
    f: 492
    g: 114
    h: 65412
    i: 65079
    x: 123
    y: 456
    In little Bobby's kit's instructions booklet (provided as your puzzle input), what signal is ultimately provided to wire a?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        auto gates = get_input(input_dir);
        std::sort(gates.begin(), gates.end(), [](const gate& a, const gate& b){ return a.dest < b.dest; });
        return std::to_string(iterate_until_static(gates, "a"));
    }

    /*
    --- Part Two ---

    Now, take the signal you got on wire a, override wire b to that signal, and reset the other wires (including wire a). What new signal is ultimately provided to wire a?
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto gates = get_input(input_dir);
        std::sort(gates.begin(), gates.end(), [](const gate& a, const gate& b){ return a.dest < b.dest; });
        auto first_run = iterate_until_static(gates, "a");
        auto found = std::find_if(gates.begin(), gates.end(), [](const gate& g){ return g.dest == "b"sv; });
        if (found != gates.end()) {
            std::get<input>(found->op).value = first_run;
        }
        return std::to_string(iterate_until_static(gates, "a"));
    }

    aoc::registration r {2015, 7, part_1, part_2};

    TEST_SUITE("2015_day7") {
        TEST_CASE("2015_day7:example") {
            std::vector<gate> gates;
            gates.emplace_back("x", input{123});
            gates.emplace_back("y", input{456});
            gates.emplace_back("d", and_or{"x", "y", true});
            gates.emplace_back("e", and_or{"x", "y", false});
            gates.emplace_back("f", shift{"x", 2, false});
            gates.emplace_back("g", shift{"y", 2, true});
            gates.emplace_back("h", not_op{"x"});
            gates.emplace_back("i", not_op{"y"});
            std::for_each(gates.begin(), gates.end(), [&gates](auto& g){ iterate_until_static(gates, g.dest); });
            REQUIRE(gates[0].value.has_value());
            REQUIRE_EQ(gates[0].value.value(), 123);
            REQUIRE(gates[1].value.has_value());
            REQUIRE_EQ(gates[1].value.value(), 456);
            REQUIRE(gates[2].value.has_value());
            REQUIRE_EQ(gates[2].value.value(), 72);
            REQUIRE(gates[3].value.has_value());
            REQUIRE_EQ(gates[3].value.value(), 507);
            REQUIRE(gates[4].value.has_value());
            REQUIRE_EQ(gates[4].value.value(), 492);
            REQUIRE(gates[5].value.has_value());
            REQUIRE_EQ(gates[5].value.value(), 114);
            REQUIRE(gates[6].value.has_value());
            REQUIRE_EQ(gates[6].value.value(), 65412);
            REQUIRE(gates[7].value.has_value());
            REQUIRE_EQ(gates[7].value.value(), 65079);
        }
        TEST_CASE("2015_day7:test1") {
            std::vector<gate> gates;
            gates.emplace_back("a", input{123});
            gates.emplace_back("b", input{456});
            gates.emplace_back("c", and_or{"a", "b", true});//72
            gates.emplace_back("d", and_or{"a", "b", false});//507
            gates.emplace_back("e", shift{"a", 2, false});//492
            gates.emplace_back("f", shift{"b", 2, true});//114
            gates.emplace_back("g", and_or{"c", "f", true});//64
            gates.emplace_back("h", and_or{"d", "e", false});//511
            gates.emplace_back("i", and_or_val{24, "g", false});//88
            gates.emplace_back("j", and_or_val{264, "h", true});//264
            gates.emplace_back("k", and_or{"i", "j", true});//8
            auto val = iterate_until_static(gates, "k");
            REQUIRE_EQ(val, 8);
        }
        TEST_CASE("2015_day7:test_parse_in") {
            REQUIRE_THROWS(parse("fsfs -> 12"));
            REQUIRE_THROWS(parse("12 -> 54"));

            auto in1 = parse("43 -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<input>(in1.op));
            REQUIRE_EQ(std::get<input>(in1.op).value, 43u);

            auto in2 = parse("8 -> z");
            REQUIRE_EQ(in2.dest, "z"sv);
            REQUIRE(std::holds_alternative<input>(in2.op));
            REQUIRE_EQ(std::get<input>(in2.op).value, 8u);
        }
        TEST_CASE("2015_day7:test_parse_and_or") {
            REQUIRE_THROWS(parse("fsfs AND 65 -> 12"));
            REQUIRE_THROWS(parse("12 OR hfdj -> 54"));
            REQUIRE_THROWS(parse("fsfs AND 65 -> jiu"));
            REQUIRE_THROWS(parse("5 NOT gfd -> jiu"));
            REQUIRE_THROWS(parse("eww BUT gfd -> jiu"));

            auto in1 = parse("43 AND rtew -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<and_or_val>(in1.op));
            REQUIRE_EQ(std::get<and_or_val>(in1.op).first, 43u);
            REQUIRE_EQ(std::get<and_or_val>(in1.op).second, "rtew"sv);
            REQUIRE(std::get<and_or_val>(in1.op).is_and);

            auto in2 = parse("43 OR rtew -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<and_or_val>(in2.op));
            REQUIRE_EQ(std::get<and_or_val>(in2.op).first, 43u);
            REQUIRE_EQ(std::get<and_or_val>(in2.op).second, "rtew"sv);
            REQUIRE(!std::get<and_or_val>(in2.op).is_and);

            auto in3 = parse("mnb AND rtew -> fhj");
            REQUIRE_EQ(in3.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<and_or>(in3.op));
            REQUIRE_EQ(std::get<and_or>(in3.op).first, "mnb"sv);
            REQUIRE_EQ(std::get<and_or>(in3.op).second, "rtew"sv);
            REQUIRE(std::get<and_or>(in3.op).is_and);

            auto in4 = parse("mnb OR rtew -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<and_or>(in4.op));
            REQUIRE_EQ(std::get<and_or>(in4.op).first, "mnb"sv);
            REQUIRE_EQ(std::get<and_or>(in4.op).second, "rtew"sv);
            REQUIRE(!std::get<and_or>(in4.op).is_and);
        }
        TEST_CASE("2015_day7:test_parse_shift") {
            auto in1 = parse("ytr LSHIFT 54 -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<shift>(in1.op));
            REQUIRE_EQ(std::get<shift>(in1.op).size, 54u);
            REQUIRE_EQ(std::get<shift>(in1.op).src, "ytr"sv);
            REQUIRE(!std::get<shift>(in1.op).is_right);

            auto in2 = parse("y RSHIFT 8 -> z");
            REQUIRE_EQ(in2.dest, "z"sv);
            REQUIRE(std::holds_alternative<shift>(in2.op));
            REQUIRE_EQ(std::get<shift>(in2.op).size, 8u);
            REQUIRE_EQ(std::get<shift>(in2.op).src, "y"sv);
            REQUIRE(std::get<shift>(in2.op).is_right);
        }
        TEST_CASE("2015_day7:test_parse_not") {
            auto in1 = parse("NOT ytr -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<not_op>(in1.op));
            REQUIRE_EQ(std::get<not_op>(in1.op).src, "ytr"sv);

            auto in2 = parse("NOT y -> z");
            REQUIRE_EQ(in2.dest, "z"sv);
            REQUIRE(std::holds_alternative<not_op>(in2.op));
            REQUIRE_EQ(std::get<not_op>(in2.op).src, "y"sv);
        }
        TEST_CASE("2015_day7:test_parse_pass") {
            auto in1 = parse("ytr -> fhj");
            REQUIRE_EQ(in1.dest, "fhj"sv);
            REQUIRE(std::holds_alternative<pass>(in1.op));
            REQUIRE_EQ(std::get<pass>(in1.op).src, "ytr"sv);

            auto in2 = parse("y -> z");
            REQUIRE_EQ(in2.dest, "z"sv);
            REQUIRE(std::holds_alternative<pass>(in2.op));
            REQUIRE_EQ(std::get<pass>(in2.op).src, "y"sv);
        }
    }

}

