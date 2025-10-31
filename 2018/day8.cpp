//
// Created by Dan on 10/30/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/8
     */

    std::vector<int> get_input(const std::vector<std::string>& lines) {
        return split_no_empty(lines.front(), ' ') | std::views::transform(&parse32) | std::ranges::to<std::vector>();
    }

    struct node {
        std::vector<std::unique_ptr<node>> children;
        std::vector<int> metadata;

        [[nodiscard]] int value() const {
            if (children.empty()) {
                return std::accumulate(metadata.begin(), metadata.end(), 0);
            }
            else {
                int sum = 0;
                for (const int idx : metadata | std::views::transform([](int m){ return m - 1; })) {
                    if (idx >= 0 && idx < children.size()) {
                        sum += children[idx]->value();
                    }
                }
                return sum;
            }
        }
    };

    using iter = std::vector<int>::const_iterator;

    node parse_tree(iter& current, const iter end) {
        node retval{};
        const int num_children = *current++;
        const int num_metadata = *current++;
        for (int i = 0; i < num_children; i++) {
            retval.children.push_back(std::make_unique<node>(parse_tree(current, end)));
        }
        retval.metadata.assign(current, current + num_metadata);
        current += num_metadata;
        return retval;
    }

    int sum_metadata(const node& n) {
        return std::accumulate(n.metadata.begin(), n.metadata.end(), 0) +
            std::accumulate(n.children.begin(), n.children.end(), 0, [](int total, const auto& c){ return total + sum_metadata(*c); });
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto current = input.begin();
        auto tree = parse_tree(current, input.end());
        const auto sum = sum_metadata(tree);
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto current = input.begin();
        auto tree = parse_tree(current, input.end());
        const auto sum = tree.value();
        return std::to_string(sum);
    }

    aoc::registration r {2018, 8, part_1, part_2};

//    TEST_SUITE("2018_day08") {
//        TEST_CASE("2018_day08:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */