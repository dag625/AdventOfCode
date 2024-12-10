//
// Created by Dan on 12/9/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <compare>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/12
     */

    struct node {
        int id;
        std::vector<int> connections;

        auto operator<=>(const node& rhs) const { return id <=> rhs.id; }
        bool operator==(const node& rhs) const { return id == rhs.id; }

        auto operator<=>(const int rhs) const { return id <=> rhs; }
        bool operator==(const int rhs) const { return id == rhs; }
    };

    node parse_node(std::string_view s) {
        const auto parts = split(s, "<->");
        const auto list = split(parts[1], ',');
        return {parse<int>(trim(parts[0])),
                list | std::views::transform([](std::string_view v){ return parse<int>(trim(v)); }) | std::ranges::to<std::vector>()};
    }

    std::vector<node> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_12_input.txt");
        return lines | std::views::transform(&parse_node) | std::ranges::to<std::vector>();
        //Note that the input is sorted, so the list of nodes is also sorted.
    }

    std::vector<int> group_containing(const int seed, const std::vector<node>& nodes) {
        std::vector<int> retval, check;
        check.push_back(seed);
        while (!check.empty()) {
            const auto rfound = std::lower_bound(retval.begin(), retval.end(), check.front());
            if (rfound == retval.end() || *rfound != check.front()) {
                retval.insert(rfound, check.front());

                const auto nfound = std::lower_bound(nodes.begin(), nodes.end(), check.front());
                for (const auto c : nfound->connections) {
                    const auto cfound = std::lower_bound(retval.begin(), retval.end(), c);
                    if (cfound == retval.end() || *cfound != c) {
                        check.push_back(c);
                    }
                }
                check.erase(check.begin());
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto group_0 = group_containing(0, input);
        return std::to_string(group_0.size());
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto ungrouped = std::views::iota(0, 2000) | std::ranges::to<std::vector>();
        std::vector<std::vector<int>> groups;
        while (!ungrouped.empty()) {
            const int seed = ungrouped.front();
            auto grp = group_containing(seed, input);
            std::vector<int> new_ungrouped;
            new_ungrouped.reserve(ungrouped.size());
            std::set_difference(ungrouped.begin(), ungrouped.end(), grp.begin(), grp.end(), std::back_inserter(new_ungrouped));
            ungrouped.swap(new_ungrouped);
            groups.push_back(std::move(grp));
        }
        return std::to_string(groups.size());
    }

    aoc::registration r{2017, 12, part_1, part_2};

//    TEST_SUITE("2017_day12") {
//        TEST_CASE("2017_day12:example") {
//
//        }
//    }

} /* namespace <anon> */