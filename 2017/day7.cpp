//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <tuple>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/7
     */

    struct program {
        std::string name;
        int weight = 0;
        std::vector<std::string> subs;
    };

    program parse_prog(std::string_view s) {
        auto parts = split(s, ' ');
        auto subs = parts |
                std::views::drop(3) |
                std::views::transform([](std::string_view s){
                    if (s.back() == ',') {
                        s.remove_suffix(1);
                    }
                    return std::string{s};
                }) |
                std::ranges::to<std::vector>();
        parts[1].remove_prefix(1);
        parts[1].remove_suffix(1);
        return {std::string{parts[0]}, parse<int>(parts[1]), std::move(subs)};
    }

    std::vector<program> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_prog) | std::ranges::to<std::vector>();
    }

    std::optional<std::string> get_parent(const std::string& name, const std::vector<program>& nodes) {
        for (const auto& node : nodes) {
            const auto found = std::find(node.subs.begin(), node.subs.end(), name);
            if (found != node.subs.end()) {
                return node.name;
            }
        }
        return std::nullopt;
    }

    struct elem {
        int value = 0;
        int node_w = 0;
        int count = 1;

        elem(int v, int w) : value{v}, node_w{w} {}

        auto operator<=>(const elem& rhs) const { return count <=> rhs.count; }

        auto operator<=>(const int& rhs) const { return value <=> rhs; }
        bool operator==(const int& rhs) const { return value == rhs; }
    };

    //Returns own weight, total weight, and fixed weight
    std::tuple<int, int, std::optional<int>> get_weight(const std::string& name, const std::vector<program>& nodes) {
        //Probably could be faster by sorting and using lower_bound.
        const auto found = std::find_if(nodes.begin(), nodes.end(), [&name](const program& p){ return p.name == name; });
        auto weight = found->weight;
        std::vector<std::tuple<int, int, std::optional<int>>> subs;
        std::transform(found->subs.begin(), found->subs.end(), std::back_inserter(subs),
                       [&nodes](const std::string& sn){ return get_weight(sn, nodes); });
        std::optional<int> to_fix;
        for (const auto& [sw, tsw, fix] : subs) {
            weight += tsw;
            if (fix) {
                to_fix = fix;
            }
        }
        //Check if we are where things are unbalanced
        if (!to_fix && !subs.empty()) {
            std::vector<elem> checks;
            for (const auto& [nw, tw, f] : subs) {
                auto found_check = std::find(checks.begin(), checks.end(), tw);
                if (found_check != checks.end()) {
                    ++(found_check->count);
                }
                else {
                    checks.emplace_back(tw, nw);
                }
            }
            if (checks.size() > 1) {
                std::sort(checks.begin(), checks.end());
                const auto del = checks.front().value - checks.back().value;
                to_fix = checks.front().node_w - del;
            }
        }
        return {found->weight, weight, to_fix};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        for (const auto& node : input) {
            const auto parent = get_parent(node.name, input);
            if (!parent) {
                return node.name;
            }
        }
        return "Not Found";
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        for (const auto& node : input) {
            const auto [node_w, total_w, to_fix] = get_weight(node.name, input);
            if (to_fix) {
                return std::to_string(*to_fix);
            }
        }
        return "Not Found";
    }

    aoc::registration r{2017, 7, part_1, part_2};

    TEST_SUITE("2017_day07") {
        TEST_CASE("2017_day07:example") {
            const std::vector<program> input {
                    {"pbga", 66, {}},
                    {"xhth", 57, {}},
                    {"ebii", 61, {}},
                    {"havc", 66, {}},
                    {"ktlj", 57, {}},
                    {"fwft", 72, {"ktlj", "cntj", "xhth"}},
                    {"qoyq", 66, {}},
                    {"padx", 45, {"pbga", "havc", "qoyq"}},
                    {"tknk", 41, {"ugml", "padx", "fwft"}},
                    {"jptl", 61, {}},
                    {"ugml", 68, {"gyxo", "ebii", "jptl"}},
                    {"gyxo", 61, {}},
                    {"cntj", 57, {}}
            };
            for (const auto& node : input) {
                const auto parent = get_parent(node.name, input);
                fmt::println("{} has parent:  {}", node.name, parent.value_or("NONE"));
            }
            CHECK(true);
        }
    }

} /* namespace <anon> */