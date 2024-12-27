//
// Created by Dan on 12/8/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/8
     */

    grid<char> get_input(const std::vector<std::string>& lines) {
        return to_grid(lines);
    }

    struct node_list {
        char name = '\0';
        std::vector<position> nodes;

        node_list() = default;
        node_list(const char c) : name{c} {}
    };

    int to_index(const char c) {
        if (isdigit(c)) { return c - '0'; }
        else if (isupper(c)) { return c - 'A' + 10; }
        else { return c - 'a' + 36; }
    }

    std::vector<node_list> get_nodes(const grid<char>& g) {
        std::vector<node_list> retval;
        retval.reserve(10 + 2 * 26);
        for (int i = 0; i < 10; ++i) {
            retval.emplace_back(i + '0');
        }
        for (int i = 0; i < 26; ++i) {
            retval.emplace_back(i + 'A');
        }
        for (int i = 0; i < 26; ++i) {
            retval.emplace_back(i + 'a');
        }
        for (const auto p : g.list_positions()) {
            if (g[p] != '.') {
                retval[to_index(g[p])].nodes.push_back(p);
            }
        }
        return retval;
    }

    using antinode_f = std::vector<position> (*)(const grid<char>&, const std::vector<position>&);

    std::vector<position> get_normal_antinodes(const grid<char>& g, const std::vector<position>& nodes) {
        std::vector<position> retval;
        for (int i = 0; i < nodes.size(); ++i) {
            for (int j = i + 1; j < nodes.size(); ++j) {
                const auto delta = nodes[i] - nodes[j];
                const velocity v {delta.x, delta.y};
                const auto candidate1 = nodes[i] + v;
                const auto candidate2 = nodes[j] - v;
                if (g.in(candidate1)) {
                    retval.push_back(candidate1);
                }
                if (g.in(candidate2)) {
                    retval.push_back(candidate2);
                }
            }
        }
        std::sort(retval.begin(), retval.end());
        retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        return retval;
    }

    std::vector<position> get_resonant_antinodes(const grid<char>& g, const std::vector<position>& nodes) {
        std::vector<position> retval;
        for (int i = 0; i < nodes.size(); ++i) {
            for (int j = i + 1; j < nodes.size(); ++j) {
                retval.push_back(nodes[i]);
                retval.push_back(nodes[j]);
                const auto delta = nodes[i] - nodes[j];
                const velocity v {delta.x, delta.y};

                for (auto ian = nodes[i] + v; g.in(ian); ian += v) {
                    retval.push_back(ian);
                }
                for (auto jan = nodes[j] - v; g.in(jan); jan -= v) {
                    retval.push_back(jan);
                }
            }
        }
        std::sort(retval.begin(), retval.end());
        retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        return retval;
    }

    std::vector<position> get_antinodes(const grid<char>& g, const std::vector<node_list>& nodes, antinode_f func) {
        std::vector<position> retval;
        for (const auto& n : nodes) {
            const auto an = func(g, n.nodes);
            const auto mid = static_cast<std::ptrdiff_t>(retval.size());
            retval.insert(retval.end(), an.begin(), an.end());
            std::inplace_merge(retval.begin(), retval.begin() + mid, retval.end());
            retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto nodes = get_nodes(input);
        const auto antinodes = get_antinodes(input, nodes, &get_normal_antinodes);
        return std::to_string(antinodes.size());
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto nodes = get_nodes(input);
        const auto antinodes = get_antinodes(input, nodes, &get_resonant_antinodes);
        return std::to_string(antinodes.size());
    }

    aoc::registration r{2024, 8, part_1, part_2};

//    TEST_SUITE("2024_day08") {
//        TEST_CASE("2024_day08:example") {
//
//        }
//    }

} /* namespace <anon> */