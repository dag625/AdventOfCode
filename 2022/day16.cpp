//
// Created by Dan on 12/16/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <unordered_map>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct raw_valve {
        std::string name;
        int rate;
        std::vector<std::string> connections;
    };

    struct valve {
        int rate = 0;
        int dist = std::numeric_limits<int>::max();
        std::vector<int> connections;

        valve() = default;
        explicit valve(int r) : rate{r} {}
    };

    raw_valve parse_valve(std::string_view s) {
        const auto parts = split(s, ' ');
        const auto rparts = split(parts[4], '=');
        std::vector<std::string> conns;
        for (int i = 9; i < parts.size(); ++i) {
            conns.emplace_back(parts[i].begin(), parts[i].begin() + 2);
        }
        return {std::string{parts[1]}, parse<int>(rparts[1]), std::move(conns)};
    }

    std::vector<valve> to_valves(const std::vector<std::string>& lines) {
        auto raw = lines | std::views::transform(parse_valve) | to<std::vector<raw_valve>>();
        const auto found = std::find_if(raw.begin(), raw.end(), [](const raw_valve& v){ return v.name == "AA"; });
        std::swap(*found, raw[0]);//So that I will start at 0
        std::vector<valve> retval;
        retval.reserve(raw.size());
        for (const auto& rv : raw) {
            retval.emplace_back(rv.rate);
            for (const auto& c : rv.connections) {
                const auto rf = std::find_if(raw.begin(), raw.end(), [&c](const raw_valve& v){ return v.name == c; });
                retval.back().connections.push_back(static_cast<int>(std::distance(raw.begin(), rf)));
            }
        }
        return retval;
    }

    std::vector<valve> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_16_input.txt");
        return to_valves(lines);
    }

    struct dist_info {
        int index;
        int dist;

        bool operator<(const dist_info& rhs) const noexcept {
            return dist > rhs.dist;
        }
    };

    std::vector<int> find_distances(std::vector<valve> valves, const int from) {
        std::vector<dist_info> queue;
        queue.push_back({from, 0});
        valves[from].dist = 0;

        while (!queue.empty()) {
            const auto next = queue.front();
            std::pop_heap(queue.begin(), queue.end());
            queue.erase(queue.end() - 1);

            for (const auto& n : valves[next.index].connections) {
                const auto cost = next.dist + 1;
                if (cost < valves[n].dist) {
                    valves[n].dist = cost;

                    auto found = std::find_if(queue.begin(), queue.end(), [&n](const dist_info& q){ return q.index == n; });
                    if (found != queue.end()) {
                        found->dist = cost;
                        std::make_heap(queue.begin(), queue.end());
                    }
                    else {
                        queue.push_back({n, cost});
                        std::push_heap(queue.begin(), queue.end());
                    }
                }
            }
        }
        std::vector<int> retval;
        retval.reserve(valves.size());
        for (const auto& v : valves) {
            retval.push_back(v.dist);
        }
        return retval;
    }

    struct flow_valve {
        std::vector<int> distances;
        int from_origin = 0;
        int rate = 0;
        bool visited = false;

        flow_valve() = default;
        flow_valve(const valve& v, int fo) : rate{v.rate}, from_origin{fo} {}
    };

    std::vector<flow_valve> preprocess(const std::vector<valve>& valves) {
        std::vector<flow_valve> retval;
        const auto dists_from_origin = find_distances(valves, 0);
        for (int i = 0; i < valves.size(); ++i) {
            if (valves[i].rate > 0) {
                retval.emplace_back(valves[i], dists_from_origin[i]);
                const auto dists = find_distances(valves, i);
                for (int j = 0; j < valves.size(); ++j) {
                    if (valves[j].rate > 0) {
                        retval.back().distances.push_back(dists[j]);
                    }
                }
            }
        }
        return retval;
    }

    int time_spent(const std::vector<flow_valve>& valves, const std::vector<int>& path) {
        int retval = 0;
        std::optional<int> prev;
        for (const auto p : path) {
            if (!prev) {
                retval += valves[p].from_origin + 1;
            }
            else {
                retval += valves[*prev].distances[p] + 1;
            }
            prev = p;
        }
        return retval;
    }

    int total_flow(const std::vector<flow_valve>& valves, int minutes_left, const std::vector<int>::const_iterator path_begin, const std::vector<int>::const_iterator path_end) {
        int retval = 0;
        std::optional<int> prev;
        for (auto current = path_begin; current != path_end; ++current) {
            if (!prev) {
                minutes_left -= valves[*current].from_origin + 1;
            }
            else {
                minutes_left -= valves[*prev].distances[*current] + 1;
            }
            retval += minutes_left * valves[*current].rate;
            prev = *current;
        }
        return retval;
    }

    int dfs(const std::vector<flow_valve>& valves, std::vector<int>& path, const int max_time) {
        if (path.size() == valves.size()) {
            return total_flow(valves, max_time, path.begin(), path.end());
        }
        path.push_back(-1);
        int best = -1;
        for (int i = 0; i < valves.size(); ++i) {
            const auto fp = std::find(path.begin(), path.end(), i);
            if (fp == path.end()) {
                path.back() = i;
                if (time_spent(valves, path) < max_time) {
                    const auto flow = dfs(valves, path, max_time);
                    if (flow > best) {
                        best = flow;
                    }
                }
                else {
                    const auto flow = total_flow(valves, max_time, path.begin(), path.end() - 1);
                    if (flow > best) {
                        best = flow;
                    }
                }
            }
        }
        path.pop_back();
        return best;
    }

    int dfs(const std::vector<flow_valve>& valves, std::vector<int>& path1, std::vector<int>& path2, const int max_time) {
        if ((path1.size() + path2.size()) == valves.size()) {
            const auto flow = total_flow(valves, max_time, path1.begin(), path1.end()) + total_flow(valves, max_time, path2.begin(), path2.end());
            return flow;
        }
        path1.push_back(-1);
        int best = -1;
        for (int i = 0; i < valves.size(); ++i) {
            const auto fp1 = std::find(path1.begin(), path1.end(), i);
            const auto fp2 = std::find(path2.begin(), path2.end(), i);
            if (fp1 == path1.end() && fp2 == path2.end()) {
                path1.back() = i;
                if (time_spent(valves, path1) < max_time) {
                    const auto flow = dfs(valves, path2, path1, max_time);
                    if (flow > best) {
                        best = flow;
                    }
                }
                else {
                    const auto flow = total_flow(valves, max_time, path1.begin(), path1.end() - 1) +
                            total_flow(valves, max_time, path2.begin(), path2.end());
                    if (flow > best) {
                        best = flow;
                    }
                }
            }
        }
        path1.pop_back();
        return best;
    }

    int search(const std::vector<valve>& valves, const int max_time, const bool helper = false) {
        auto graph = preprocess(valves);
        std::vector<int> path;
        if (helper) {
            std::vector<int> helper_path;
            return dfs(graph, path, helper_path, max_time);
        }
        else {
            return dfs(graph, path, max_time);
        }
    }

    /*
    --- Day 16: Proboscidea Volcanium ---
    The sensors have led you to the origin of the distress signal: yet another handheld device, just like the one the Elves gave you. However, you don't see any Elves around; instead, the device is surrounded by elephants! They must have gotten lost in these tunnels, and one of the elephants apparently figured out how to turn on the distress signal.

    The ground rumbles again, much stronger this time. What kind of cave is this, exactly? You scan the cave with your handheld device; it reports mostly igneous rock, some ash, pockets of pressurized gas, magma... this isn't just a cave, it's a volcano!

    You need to get the elephants out of here, quickly. Your device estimates that you have 30 minutes before the volcano erupts, so you don't have time to go back out the way you came in.

    You scan the cave for other options and discover a network of pipes and pressure-release valves. You aren't sure how such a system got into a volcano, but you don't have time to complain; your device produces a report (your puzzle input) of each valve's flow rate if it were opened (in pressure per minute) and the tunnels you could use to move between the valves.

    There's even a valve in the room you and the elephants are currently standing in labeled AA. You estimate it will take you one minute to open a single valve and one minute to follow any tunnel from one valve to another. What is the most pressure you could release?

    For example, suppose you had the following scan output:

    Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
    Valve BB has flow rate=13; tunnels lead to valves CC, AA
    Valve CC has flow rate=2; tunnels lead to valves DD, BB
    Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE
    Valve EE has flow rate=3; tunnels lead to valves FF, DD
    Valve FF has flow rate=0; tunnels lead to valves EE, GG
    Valve GG has flow rate=0; tunnels lead to valves FF, HH
    Valve HH has flow rate=22; tunnel leads to valve GG
    Valve II has flow rate=0; tunnels lead to valves AA, JJ
    Valve JJ has flow rate=21; tunnel leads to valve II
    All of the valves begin closed. You start at valve AA, but it must be damaged or jammed or something: its flow rate is 0, so there's no point in opening it. However, you could spend one minute moving to valve BB and another minute opening it; doing so would release pressure during the remaining 28 minutes at a flow rate of 13, a total eventual pressure release of 28 * 13 = 364. Then, you could spend your third minute moving to valve CC and your fourth minute opening it, providing an additional 26 minutes of eventual pressure release at a flow rate of 2, or 52 total pressure released by valve CC.

    Making your way through the tunnels like this, you could probably open many or all of the valves by the time 30 minutes have elapsed. However, you need to release as much pressure as possible, so you'll need to be methodical. Instead, consider this approach:

    == Minute 1 ==
    No valves are open.
    You move to valve DD.

    == Minute 2 ==
    No valves are open.
    You open valve DD.

    == Minute 3 ==
    Valve DD is open, releasing 20 pressure.
    You move to valve CC.

    == Minute 4 ==
    Valve DD is open, releasing 20 pressure.
    You move to valve BB.

    == Minute 5 ==
    Valve DD is open, releasing 20 pressure.
    You open valve BB.

    == Minute 6 ==
    Valves BB and DD are open, releasing 33 pressure.
    You move to valve AA.

    == Minute 7 ==
    Valves BB and DD are open, releasing 33 pressure.
    You move to valve II.

    == Minute 8 ==
    Valves BB and DD are open, releasing 33 pressure.
    You move to valve JJ.

    == Minute 9 ==
    Valves BB and DD are open, releasing 33 pressure.
    You open valve JJ.

    == Minute 10 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve II.

    == Minute 11 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve AA.

    == Minute 12 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve DD.

    == Minute 13 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve EE.

    == Minute 14 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve FF.

    == Minute 15 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve GG.

    == Minute 16 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You move to valve HH.

    == Minute 17 ==
    Valves BB, DD, and JJ are open, releasing 54 pressure.
    You open valve HH.

    == Minute 18 ==
    Valves BB, DD, HH, and JJ are open, releasing 76 pressure.
    You move to valve GG.

    == Minute 19 ==
    Valves BB, DD, HH, and JJ are open, releasing 76 pressure.
    You move to valve FF.

    == Minute 20 ==
    Valves BB, DD, HH, and JJ are open, releasing 76 pressure.
    You move to valve EE.

    == Minute 21 ==
    Valves BB, DD, HH, and JJ are open, releasing 76 pressure.
    You open valve EE.

    == Minute 22 ==
    Valves BB, DD, EE, HH, and JJ are open, releasing 79 pressure.
    You move to valve DD.

    == Minute 23 ==
    Valves BB, DD, EE, HH, and JJ are open, releasing 79 pressure.
    You move to valve CC.

    == Minute 24 ==
    Valves BB, DD, EE, HH, and JJ are open, releasing 79 pressure.
    You open valve CC.

    == Minute 25 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    == Minute 26 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    == Minute 27 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    == Minute 28 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    == Minute 29 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    == Minute 30 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.
    This approach lets you release the most pressure possible in 30 minutes with this valve layout, 1651.

    Work out the steps to release the most pressure in 30 minutes. What is the most pressure you can release?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto result = search(input, 30);
        return std::to_string(result);
    }

    /*
    --- Part Two ---
    You're worried that even with an optimal approach, the pressure released won't be enough. What if you got one of the elephants to help you?

    It would take you 4 minutes to teach an elephant how to open the right valves in the right order, leaving you with only 26 minutes to actually execute your plan. Would having two of you working together be better, even if it means having less time? (Assume that you teach the elephant before opening any valves yourself, giving you both the same full 26 minutes.)

    In the example above, you could teach the elephant to help you as follows:

    == Minute 1 ==
    No valves are open.
    You move to valve II.
    The elephant moves to valve DD.

    == Minute 2 ==
    No valves are open.
    You move to valve JJ.
    The elephant opens valve DD.

    == Minute 3 ==
    Valve DD is open, releasing 20 pressure.
    You open valve JJ.
    The elephant moves to valve EE.

    == Minute 4 ==
    Valves DD and JJ are open, releasing 41 pressure.
    You move to valve II.
    The elephant moves to valve FF.

    == Minute 5 ==
    Valves DD and JJ are open, releasing 41 pressure.
    You move to valve AA.
    The elephant moves to valve GG.

    == Minute 6 ==
    Valves DD and JJ are open, releasing 41 pressure.
    You move to valve BB.
    The elephant moves to valve HH.

    == Minute 7 ==
    Valves DD and JJ are open, releasing 41 pressure.
    You open valve BB.
    The elephant opens valve HH.

    == Minute 8 ==
    Valves BB, DD, HH, and JJ are open, releasing 76 pressure.
    You move to valve CC.
    The elephant moves to valve GG.

    == Minute 9 ==
    Valves BB, DD, HH, and JJ are open, releasing 76 pressure.
    You open valve CC.
    The elephant moves to valve FF.

    == Minute 10 ==
    Valves BB, CC, DD, HH, and JJ are open, releasing 78 pressure.
    The elephant moves to valve EE.

    == Minute 11 ==
    Valves BB, CC, DD, HH, and JJ are open, releasing 78 pressure.
    The elephant opens valve EE.

    (At this point, all valves are open.)

    == Minute 12 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    ...

    == Minute 20 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.

    ...

    == Minute 26 ==
    Valves BB, CC, DD, EE, HH, and JJ are open, releasing 81 pressure.
    With the elephant helping, after 26 minutes, the best you could do would release a total of 1707 pressure.

    With you and an elephant working together for 26 minutes, what is the most pressure you could release?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto result = search(input, 26, true);
        return std::to_string(result);
    }

    aoc::registration r{2022, 16, part_1, part_2};

    TEST_SUITE("2022_day16") {
        TEST_CASE("2022_day16:example") {
            const std::vector<std::string> lines {
                    "Valve AA has flow rate=0; tunnels lead to valves DD, II, BB",
                    "Valve BB has flow rate=13; tunnels lead to valves CC, AA",
                    "Valve CC has flow rate=2; tunnels lead to valves DD, BB",
                    "Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE",
                    "Valve EE has flow rate=3; tunnels lead to valves FF, DD",
                    "Valve FF has flow rate=0; tunnels lead to valves EE, GG",
                    "Valve GG has flow rate=0; tunnels lead to valves FF, HH",
                    "Valve HH has flow rate=22; tunnel leads to valve GG",
                    "Valve II has flow rate=0; tunnels lead to valves AA, JJ",
                    "Valve JJ has flow rate=21; tunnel leads to valve II"
            };
            const auto input = to_valves(lines);
            const auto result1 = search(input, 30);
            CHECK_EQ(result1, 1651);
            const auto result2 = search(input, 26, true);
            CHECK_EQ(result2, 1707);
        }
    }

}