//
// Created by Dan on 12/7/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_literals;

    struct node {
        std::string id;
        std::pair<std::string, std::string> paths;

        node(std::string_view i, std::string_view l, std::string_view r) : id{i}, paths{std::string{l}, std::string{r}} {}
        [[nodiscard]] bool operator<(const node& rhs) const noexcept { return id < rhs.id; }
        [[nodiscard]] const std::string& next(const char dir) const noexcept {
            if (dir == 'L') { return paths.first; }
            else { return paths.second; }
        }
    };

    node parse_node(std::string_view s) {
        return {s.substr(0, 3), s.substr(7, 3), s.substr(12, 3)};
    }

    std::pair<std::string, std::vector<node>> get_input(const std::vector<std::string>& lines) {
        auto nodes = lines | std::views::drop(2) | std::views::transform(&parse_node) | to<std::vector<node>>();
        std::sort(nodes.begin(), nodes.end());
        return {lines.front(), std::move(nodes)};
    }

    /*
    --- Day 8: Haunted Wasteland ---
    You're still riding a camel across Desert Island when you spot a sandstorm quickly approaching. When you turn to warn the Elf, she disappears before your eyes! To be fair, she had just finished warning you about ghosts a few minutes ago.

    One of the camel's pouches is labeled "maps" - sure enough, it's full of documents (your puzzle input) about how to navigate the desert. At least, you're pretty sure that's what they are; one of the documents contains a list of left/right instructions, and the rest of the documents seem to describe some kind of network of labeled nodes.

    It seems like you're meant to use the left/right instructions to navigate the network. Perhaps if you have the camel follow the same instructions, you can escape the haunted wasteland!

    After examining the maps for a bit, two nodes stick out: AAA and ZZZ. You feel like AAA is where you are now, and you have to follow the left/right instructions until you reach ZZZ.

    This format defines each node of the network individually. For example:

    RL

    AAA = (BBB, CCC)
    BBB = (DDD, EEE)
    CCC = (ZZZ, GGG)
    DDD = (DDD, DDD)
    EEE = (EEE, EEE)
    GGG = (GGG, GGG)
    ZZZ = (ZZZ, ZZZ)
    Starting with AAA, you need to look up the next element based on the next left/right instruction in your input. In this example, start with AAA and go right (R) by choosing the right element of AAA, CCC. Then, L means to choose the left element of CCC, ZZZ. By following the left/right instructions, you reach ZZZ in 2 steps.

    Of course, you might not find ZZZ right away. If you run out of left/right instructions, repeat the whole sequence of instructions as necessary: RL really means RLRLRLRLRLRLRLRL... and so on. For example, here is a situation that takes 6 steps to reach ZZZ:

    LLR

    AAA = (BBB, BBB)
    BBB = (AAA, ZZZ)
    ZZZ = (ZZZ, ZZZ)
    Starting at AAA, follow the left/right instructions. How many steps are required to reach ZZZ?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [directions, nodes] = get_input(lines);
        auto current = nodes.begin();
        int step = 0;
        while (true) {
            for (const char d: directions) {
                const auto &next = current->next(d);
                current = std::find_if(nodes.begin(), nodes.end(), [&next](const node &n) { return n.id == next; });
                ++step;
                //fmt::print("Step {}:  {}\n", step, current->id);
                if (current->id == "ZZZ"s) {
                    break;
                }
            }
            if (current->id == "ZZZ"s) {
                break;
            }
        }
        return std::to_string(step);
    }

    /*
    --- Part Two ---
    The sandstorm is upon you and you aren't any closer to escaping the wasteland. You had the camel follow the instructions, but you've barely left your starting position. It's going to take significantly more steps to escape!

    What if the map isn't for people - what if the map is for ghosts? Are ghosts even bound by the laws of spacetime? Only one way to find out.

    After examining the maps a bit longer, your attention is drawn to a curious fact: the number of nodes with names ending in A is equal to the number ending in Z! If you were a ghost, you'd probably just start at every node that ends with A and follow all of the paths at the same time until they all simultaneously end up at nodes that end with Z.

    For example:

    LR

    11A = (11B, XXX)
    11B = (XXX, 11Z)
    11Z = (11B, XXX)
    22A = (22B, XXX)
    22B = (22C, 22C)
    22C = (22Z, 22Z)
    22Z = (22B, 22B)
    XXX = (XXX, XXX)
    Here, there are two starting nodes, 11A and 22A (because they both end with A). As you follow each left/right instruction, use that instruction to simultaneously navigate away from both nodes you're currently on. Repeat this process until all of the nodes you're currently on end with Z. (If only some of the nodes you're on end with Z, they act like any other node and you continue as normal.) In this example, you would proceed as follows:

    Step 0: You are at 11A and 22A.
    Step 1: You choose all of the left paths, leading you to 11B and 22B.
    Step 2: You choose all of the right paths, leading you to 11Z and 22C.
    Step 3: You choose all of the left paths, leading you to 11B and 22Z.
    Step 4: You choose all of the right paths, leading you to 11Z and 22B.
    Step 5: You choose all of the left paths, leading you to 11B and 22C.
    Step 6: You choose all of the right paths, leading you to 11Z and 22Z.
    So, in this example, you end up entirely on nodes that end in Z after 6 steps.

    Simultaneously start on every node that ends with A. How many steps does it take before you're only on nodes that end with Z?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto [directions, nodes] = get_input(lines);
        std::vector<decltype(nodes.begin())> current;
        for (auto n = nodes.begin(); n != nodes.end(); ++n) {
            if (n->id.back() == 'A') {
                current.push_back(n);
            }
        }
        int64_t step = 0;
        std::vector<std::vector<int64_t>> last_visits;
        last_visits.resize(current.size());
        bool all_at_z = false, have_all_intervals = false;
        while (true) {
            for (const char d: directions) {
                int num_at_z = 0;
                ++step;
                for (int i = 0; i < current.size(); ++i) {
                    const auto &next = current[i]->next(d);
                    current[i] = std::find_if(nodes.begin(), nodes.end(), [&next](const node &n) { return n.id == next; });
                    if (current[i]->id.back() == 'Z') {
                        last_visits[i].push_back(step);
//                        if (last_visits[i].size() > 1) {
//                            const auto& l = last_visits[i];
//                            fmt::print("Delta {}:  {} - {} = {}\n", i, l[l.size() - 1], l[l.size() - 2], (l[l.size() - 1] - l[l.size() - 2]));
//                        }
                    }
                }
                all_at_z = std::all_of(current.begin(), current.end(), [](const decltype(nodes.begin())& c){ return c->id.back() == 'Z'; });
                if (all_at_z) {
                    break;
                }
                have_all_intervals = std::all_of(last_visits.begin(), last_visits.end(), [](const std::vector<int64_t>& l){ return l.size() > 1; });
                if (have_all_intervals)
                {
                    break;
                }
            }
            if (all_at_z) {
                break;
            }
            if (have_all_intervals) {
                break;
            }
        }
        if (all_at_z) {
            return std::to_string(step);
        }
        else {
            return std::to_string(std::accumulate(last_visits.begin(), last_visits.end(), static_cast<int64_t>(1), [](int64_t tot, const std::vector<int64_t>& v){ return std::lcm(tot, (v[1] - v[0])); }));
        }
    }

    aoc::registration r{2023, 8, part_1, part_2};

    TEST_SUITE("2023_day08") {
        TEST_CASE("2023_day08:example") {
            using namespace std::string_view_literals;
            const std::string directions {"LR"};
            const std::vector<node> nodes {
                    {"11A"sv, "11B"sv, "XXX"sv},
                    {"11B"sv, "XXX"sv, "11Z"sv},
                    {"11Z"sv, "11B"sv, "XXX"sv},
                    {"22A"sv, "22B"sv, "XXX"sv},
                    {"22B"sv, "22C"sv, "22C"sv},
                    {"22C"sv, "22Z"sv, "22Z"sv},
                    {"22Z"sv, "22B"sv, "22B"sv},
                    {"XXX"sv, "XXX"sv, "XXX"sv}
            };
            std::vector<decltype(nodes.begin())> current;
            for (auto n = nodes.begin(); n != nodes.end(); ++n) {
                if (n->id.back() == 'A') {
                    current.push_back(n);
                }
            }
            int64_t step = 0;
            while (true) {
                bool all_at_z = false;
                for (const char d: directions) {
                    int num_at_z = 0;
                    for (auto& c : current) {
                        const auto &next = c->next(d);
                        c = std::find_if(nodes.begin(), nodes.end(), [&next](const node &n) { return n.id == next; });
                    }
                    ++step;
                    all_at_z = std::all_of(current.begin(), current.end(), [](const decltype(nodes.begin())& c){ return c->id.back() == 'Z'; });
                    if (all_at_z) {
                        break;
                    }
                }
                if (all_at_z) {
                    break;
                }
            }
            CHECK_EQ(step, 6);
        }
    }

}