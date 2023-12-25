//
// Created by Dan on 12/24/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct component {
        std::string name;
        std::vector<std::string> connections;
    };

    component parse_comp(std::string_view s) {
        const auto parts = split(s, ':');
        const auto subparts = split_no_empty(parts[1], ' ');
        return {std::string{parts[0]}, subparts | to<std::vector<std::string>>()};
    }

    std::vector<component> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_25_input.txt");
        return lines | std::views::transform(&parse_comp) | to<std::vector>();
    }

    using adjacency_map = std::vector<std::vector<int>>;

    adjacency_map construct_map(const std::vector<component>& comps) {
        std::vector<std::string> components;
        std::vector<std::pair<std::string, std::string>> connections;
        for (const auto& comp : comps) {
            const auto found_comp = std::lower_bound(components.begin(), components.end(), comp.name);
            if (found_comp == components.end() || *found_comp != comp.name) {
                components.insert(found_comp, comp.name);
            }
            for (const auto& conn_name : comp.connections) {
                const auto found_conn = std::lower_bound(components.begin(), components.end(), conn_name);
                if (found_conn == components.end() || *found_conn != conn_name) {
                    components.insert(found_conn, conn_name);
                }

                std::pair<std::string, std::string> conn {comp.name, conn_name};
                if (conn_name < comp.name) {
                    std::swap(conn.first, conn.second);
                }
                const auto found_conp = std::lower_bound(connections.begin(), connections.end(), conn);
                if (found_conp == connections.end() || *found_conp != conn) {
                    connections.insert(found_conp, conn);
                }
            }
        }

        adjacency_map map;
        const auto size = components.size();
        for (int i = 0; i < size; ++i) {
            map.emplace_back();
            map.back().resize(size);
        }
        for (const auto& [first, second] : connections) {
            const auto idx1 = std::distance(components.begin(), std::lower_bound(components.begin(), components.end(), first));
            const auto idx2 = std::distance(components.begin(), std::lower_bound(components.begin(), components.end(), second));
            map[idx1][idx2] = 1;
            map[idx2][idx1] = 1;
        }
        return map;
    }

    //From https://en.wikipedia.org/wiki/Stoer%E2%80%93Wagner_algorithm
    std::pair<int, std::vector<int>> stoer_wagner(std::vector<std::vector<int>> mat) {
        std::pair<int, std::vector<int>> best = {std::numeric_limits<int>::max(), {}};
        int n = static_cast<int>(mat.size());
        std::vector<std::vector<int>> co(n);

        for (int i = 0; i < n; i++)
            co[i] = {i};

        for (int ph = 1; ph < n; ph++) {
            std::vector<int> w = mat[0];
            size_t s = 0, t = 0;
            for (int it = 0; it < n - ph; it++) { // O(V^2) -> O(E log V) with prio. queue
                w[t] = std::numeric_limits<int>::min();
                s = t, t = std::max_element(w.begin(), w.end()) - w.begin();
                for (int i = 0; i < n; i++) w[i] += mat[t][i];
            }
            best = std::min(best, {w[t] - mat[t][t], co[t]});
            co[s].insert(co[s].end(), co[t].begin(), co[t].end());
            for (int i = 0; i < n; i++) mat[s][i] += mat[t][i];
            for (int i = 0; i < n; i++) mat[i][s] = mat[s][i];
            mat[0][t] = std::numeric_limits<int>::min();
        }

        return best;
    }

    /*
    --- Day 25: Snowverload ---
    Still somehow without snow, you go to the last place you haven't checked: the center of Snow Island, directly below the waterfall.

    Here, someone has clearly been trying to fix the problem. Scattered everywhere are hundreds of weather machines, almanacs, communication modules, hoof prints, machine parts, mirrors, lenses, and so on.

    Somehow, everything has been wired together into a massive snow-producing apparatus, but nothing seems to be running. You check a tiny screen on one of the communication modules: Error 2023. It doesn't say what Error 2023 means, but it does have the phone number for a support line printed on it.

    "Hi, you've reached Weather Machines And So On, Inc. How can I help you?" You explain the situation.

    "Error 2023, you say? Why, that's a power overload error, of course! It means you have too many components plugged in. Try unplugging some components and--" You explain that there are hundreds of components here and you're in a bit of a hurry.

    "Well, let's see how bad it is; do you see a big red reset button somewhere? It should be on its own module. If you push it, it probably won't fix anything, but it'll report how overloaded things are." After a minute or two, you find the reset button; it's so big that it takes two hands just to get enough leverage to push it. Its screen then displays:

    SYSTEM OVERLOAD!

    Connected components would require
    power equal to at least 100 stars!
    "Wait, how many components did you say are plugged in? With that much equipment, you could produce snow for an entire--" You disconnect the call.

    You have nowhere near that many stars - you need to find a way to disconnect at least half of the equipment here, but it's already Christmas! You only have time to disconnect three wires.

    Fortunately, someone left a wiring diagram (your puzzle input) that shows how the components are connected. For example:

    jqt: rhn xhk nvd
    rsh: frs pzl lsr
    xhk: hfx
    cmg: qnr nvd lhk bvb
    rhn: xhk bvb hfx
    bvb: xhk hfx
    pzl: lsr hfx nvd
    qnr: nvd
    ntq: jqt hfx bvb xhk
    nvd: lhk
    lsr: lhk
    rzs: qnr cmg lsr rsh
    frs: qnr lhk lsr
    Each line shows the name of a component, a colon, and then a list of other components to which that component is connected. Connections aren't directional; abc: xyz and xyz: abc both represent the same configuration. Each connection between two components is represented only once, so some components might only ever appear on the left or right side of a colon.

    In this example, if you disconnect the wire between hfx/pzl, the wire between bvb/cmg, and the wire between nvd/jqt, you will divide the components into two separate, disconnected groups:

    9 components: cmg, frs, lhk, lsr, nvd, pzl, qnr, rsh, and rzs.
    6 components: bvb, hfx, jqt, ntq, rhn, and xhk.
    Multiplying the sizes of these groups together produces 54.

    Find the three wires you need to disconnect in order to divide the components into two separate groups. What do you get if you multiply the sizes of these two groups together?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto map = construct_map(input);
        const auto [num_edges_removed, group1] = stoer_wagner(map);
        const auto group2_size = map.size() - group1.size();
        return std::to_string(group1.size() * group2_size);
    }

    /*
    --- Part Two ---
    You climb over weather machines, under giant springs, and narrowly avoid a pile of pipes as you find and disconnect the three wires.

    A moment after you disconnect the last wire, the big red reset button module makes a small ding noise:

    System overload resolved!
    Power required is now 50 stars.
    Out of the corner of your eye, you notice goggles and a loose-fitting hard hat peeking at you from behind an ultra crucible. You think you see a faint glow, but before you can investigate, you hear another small ding:

    Power required is now 49 stars.

    Please supply the necessary stars and
    push the button to restart the system.
    If you like, you can [Push The Big Red Button Again].
    */

    aoc::registration r{2023, 25, part_1};

    TEST_SUITE("2023_day25") {
        TEST_CASE("2023_day25:example") {
            using namespace std::string_literals;
            std::vector<std::string> lines {
                    "jqt: rhn xhk nvd"s,
                    "rsh: frs pzl lsr"s,
                    "xhk: hfx"s,
                    "cmg: qnr nvd lhk bvb"s,
                    "rhn: xhk bvb hfx"s,
                    "bvb: xhk hfx"s,
                    "pzl: lsr hfx nvd"s,
                    "qnr: nvd"s,
                    "ntq: jqt hfx bvb xhk"s,
                    "nvd: lhk"s,
                    "lsr: lhk"s,
                    "rzs: qnr cmg lsr rsh"s,
                    "frs: qnr lhk lsr"s
            };
            const auto input = lines | std::views::transform(&parse_comp) | to<std::vector>();
            const auto map = construct_map(input);
            const auto [num_edges_removed, group1] = stoer_wagner(map);
            const auto group2_size = map.size() - group1.size();
            CHECK_EQ((group1.size() * group2_size), 54);
        }
    }

}