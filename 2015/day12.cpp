//
// Created by Daniel Garcia on 6/8/21.
//

#include "registration.h"
#include "utilities.h"
#include "ranges.h"

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <string_view>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    int count_nums(const nlohmann::json& data) {
        if (data.is_number()) {
            return data.get<int>();
        }
        else if (data.is_array() || data.is_object()) {
            int val = 0;
            for (const auto& el : data) {
                val += count_nums(el);
            }
            return val;
        }
        else {
            return 0;
        }
    }

    int count_non_red_nums(const nlohmann::json& data) {
        if (data.is_number()) {
            return data.get<int>();
        }
        else if (data.is_array()) {
            int val = 0;
            for (const auto& el : data) {
                val += count_non_red_nums(el);
            }
            return val;
        }
        else if (data.is_object()) {
            int val = 0;
            bool any_red = false;
            for (const auto& el : data.items()) {
                if (el.key() == "red") {
                    any_red = true;
                    break;
                }
                else if (el.value().is_string() && el.value().get<std::string>() == "red") {
                    any_red = true;
                    break;
                }
                else {
                    val += count_non_red_nums(el.value());
                }
            }
            if (any_red) {
                return 0;
            }
            else {
                return val;
            }
        }
        else {
            return 0;
        }
    }

    nlohmann::json get_input(const std::vector<std::string>& lines) {
        return nlohmann::json::parse(lines | std::views::join | std::ranges::to<std::string>());
    }

    /*
    Santa's Accounting-Elves need help balancing the books after a recent order. Unfortunately, their accounting software uses a peculiar storage format. That's where you come in.

    They have a JSON document which contains a variety of things: arrays ([1,2,3]), objects ({"a":1, "b":2}), numbers, and strings. Your first job is to simply find all of the numbers throughout the document and add them together.

    For example:

    [1,2,3] and {"a":2,"b":4} both have a sum of 6.
    [[[3]]] and {"a":{"b":4},"c":-1} both have a sum of 3.
    {"a":[-1,1]} and [-1,{"a":1}] both have a sum of 0.
    [] and {} both have a sum of 0.
    You will not encounter any strings containing numbers.

    What is the sum of all numbers in the document?
     */
    std::string part_1(const std::vector<std::string>& lines) {
        return std::to_string(count_nums(get_input(lines)));
    }

    /*
    Uh oh - the Accounting-Elves have realized that they double-counted everything red.

    Ignore any object (and all of its children) which has any property with the value "red". Do this only for objects ({...}), not arrays ([...]).

    [1,2,3] still has a sum of 6.
    [1,{"c":"red","b":2},3] now has a sum of 4, because the middle object is ignored.
    {"d":"red","e":[1,2,3,4],"f":5} now has a sum of 0, because the entire structure is ignored.
    [1,"red",5] has a sum of 6, because "red" in an array has no effect.
     */
    std::string part_2(const std::vector<std::string>& lines) {
        return std::to_string(count_non_red_nums(get_input(lines)));
    }

    aoc::registration r{2015, 12, part_1, part_2};

    TEST_SUITE ("2015_day12") {
        TEST_CASE ("2015_day12:example") {
            using namespace nlohmann;
            REQUIRE_EQ(count_non_red_nums(R"([1,2,3])"_json), 6);
            REQUIRE_EQ(count_non_red_nums(R"([1,{"c":"red","b":2},3])"_json), 4);
            REQUIRE_EQ(count_non_red_nums(R"({"d":"red","e":[1,2,3,4],"f":5})"_json), 0);
            REQUIRE_EQ(count_non_red_nums(R"([1,"red",5])"_json), 6);
        }
    }

}