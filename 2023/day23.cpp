//
// Created by Dan on 12/22/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <deque>

#include "utilities.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    grid<char> get_input(const std::vector<std::string>& lines) {
        const auto row_len = lines.front().size();
        return {lines | std::views::join | to<std::vector>(), row_len};
    }

    std::vector<position> get_neighbors_slopeless(position p, const grid<char>& g) {
        return g.cardinal_neighbors(p) | std::views::filter([&g](const position p){ return g[p] != '#'; }) | to<std::vector<position>>();
    }

    struct node_nbr {
        position pos;
        position connected_to{};
        int distance = -1;
        char slope = '.';

        node_nbr() = default;
        node_nbr(position p, char s) : pos{p}, slope{s} {}
    };

    struct node {
        position pos;
        std::vector<node_nbr> neighbors;

        node() = default;
        node(bool is_start, position p) : pos{p} {
            if (is_start) {
                neighbors.push_back({{p.x + 1, p.y}, '.'});
            }
            else {
                neighbors.push_back({{p.x - 1, p.y}, '.'});
            }
        }
    };

    void get_neighbor_distance(const grid<char>& g, const position start, const position end, const position pstart, node_nbr& path) {
        std::vector<position> visited;
        visited.push_back(pstart);
        visited.push_back(path.pos);
        auto neighbors = get_neighbors_slopeless(path.pos, g);
        while (neighbors.size() == 2 || g[visited.back()] != '.') {
            const auto found = std::find(visited.begin(), visited.end(), neighbors[0]);
            auto next = neighbors[0];
            if (neighbors.size() == 2 && found != visited.end()) {
                next = neighbors[1];
            }
            visited.push_back(next);
            neighbors = get_neighbors_slopeless(next, g);
            if (next == start || next == end) {
                break;
            }
        }
        path.connected_to = visited.back();
        path.distance = static_cast<int>(visited.size()) - 1;
    }

    std::vector<node> condense(const grid<char>& g, const position start, const position end) {
        std::vector<node> retval;
        retval.emplace_back(true, start);
        retval.emplace_back(false, end);
        for (const auto pos : g.list_positions()) {
            if (g[pos] == '#') {
                continue;
            }
            const auto nbrs = g.cardinal_neighbors(pos) | std::views::filter([&g](const position p){ return g[p] != '#'; }) | to<std::vector<position>>();
            if (nbrs.size() > 2) {
                node next;
                next.pos = pos;
                for (const auto& n : nbrs) {
                    next.neighbors.emplace_back(n, g[n]);
                }
                retval.push_back(std::move(next));
            }
        }
        for (auto& nd : retval) {
            for (auto& nbr : nd.neighbors) {
                get_neighbor_distance(g, start, end, nd.pos, nbr);
            }
        }
        return retval;
    }

    struct node_path {
        std::vector<position> visited;
        int distance = 0;

        node_path(position p) { visited.push_back(p); }

        [[nodiscard]] bool operator<(const node_path& rhs) const {
            return distance > rhs.distance;
        }
    };

    bool can_move_sloped(const position dir, const char slope) {
        if (slope == '>') {
            return dir == position{0, 1};
        }
        else if (slope == '<') {
            return dir == position{0, -1};
        }
        else if (slope == '^') {
            return dir == position{-1, 0};
        }
        else if (slope == 'v') {
            return dir == position{1, 0};
        }
        else {
            return true;
        }
    }

    bool can_move_slopeless(const position dir, const char slope) {
        return true;
    }

    using can_move_function = bool (*)(position, char);

    int walk_nodes_recursive(const std::vector<node>& nodes, std::vector<position>& visited, int distance, const position end, can_move_function can_move) {
        if (visited.back() == end) {
            return distance;
        }
        const auto& current = visited.back();
        const auto found = std::find_if(nodes.begin(), nodes.end(), [&current](const node& n){ return n.pos == current; });
        int max_dist = 0;
        for (const auto& nbr : found->neighbors) {
            const auto dir = nbr.pos - current;
            if (!can_move(dir, nbr.slope)) {
                continue;
            }

            const auto found_nbr = std::find(visited.begin(), visited.end(), nbr.connected_to);
            if (found_nbr != visited.end()) {
                continue;
            }

            visited.push_back(nbr.connected_to);
            const auto dist = walk_nodes_recursive(nodes, visited, distance + nbr.distance, end, can_move);
            max_dist = std::max(max_dist, dist);
            visited.pop_back();
        }
        return max_dist;
    }

    int walk_nodes_recursive(const std::vector<node>& nodes, const position start, const position end, can_move_function can_move) {
        std::vector<position> visited;
        visited.reserve(nodes.size());
        visited.push_back(start);
        return walk_nodes_recursive(nodes, visited, 0, end, can_move);
    }

    /*
    --- Day 23: A Long Walk ---
    The Elves resume water filtering operations! Clean water starts flowing over the edge of Island Island.

    They offer to help you go over the edge of Island Island, too! Just hold on tight to one end of this impossibly long rope and they'll lower you down a safe distance from the massive waterfall you just created.

    As you finally reach Snow Island, you see that the water isn't really reaching the ground: it's being absorbed by the air itself. It looks like you'll finally have a little downtime while the moisture builds up to snow-producing levels. Snow Island is pretty scenic, even without any snow; why not take a walk?

    There's a map of nearby hiking trails (your puzzle input) that indicates paths (.), forest (#), and steep slopes (^, >, v, and <).

    For example:

    #.#####################
    #.......#########...###
    #######.#########.#.###
    ###.....#.>.>.###.#.###
    ###v#####.#v#.###.#.###
    ###.>...#.#.#.....#...#
    ###v###.#.#.#########.#
    ###...#.#.#.......#...#
    #####.#.#.#######.#.###
    #.....#.#.#.......#...#
    #.#####.#.#.#########v#
    #.#...#...#...###...>.#
    #.#.#v#######v###.###v#
    #...#.>.#...>.>.#.###.#
    #####v#.#.###v#.#.###.#
    #.....#...#...#.#.#...#
    #.#########.###.#.#.###
    #...###...#...#...#.###
    ###.###.#.###v#####v###
    #...#...#.#.>.>.#.>.###
    #.###.###.#.###.#.#v###
    #.....###...###...#...#
    #####################.#
    You're currently on the single path tile in the top row; your goal is to reach the single path tile in the bottom row. Because of all the mist from the waterfall, the slopes are probably quite icy; if you step onto a slope tile, your next step must be downhill (in the direction the arrow is pointing). To make sure you have the most scenic hike possible, never step onto the same tile twice. What is the longest hike you can take?

    In the example above, the longest hike you can take is marked with O, and your starting position is marked S:

    #S#####################
    #OOOOOOO#########...###
    #######O#########.#.###
    ###OOOOO#OOO>.###.#.###
    ###O#####O#O#.###.#.###
    ###OOOOO#O#O#.....#...#
    ###v###O#O#O#########.#
    ###...#O#O#OOOOOOO#...#
    #####.#O#O#######O#.###
    #.....#O#O#OOOOOOO#...#
    #.#####O#O#O#########v#
    #.#...#OOO#OOO###OOOOO#
    #.#.#v#######O###O###O#
    #...#.>.#...>OOO#O###O#
    #####v#.#.###v#O#O###O#
    #.....#...#...#O#O#OOO#
    #.#########.###O#O#O###
    #...###...#...#OOO#O###
    ###.###.#.###v#####O###
    #...#...#.#.>.>.#.>O###
    #.###.###.#.###.#.#O###
    #.....###...###...#OOO#
    #####################O#
    This hike contains 94 steps. (The other possible hikes you could have taken were 90, 86, 82, 82, and 74 steps long.)

    Find the longest hike you can take through the hiking trails listed on your map. How many steps long is the longest hike?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const position start {0, 1}, end{static_cast<int>(input.num_rows()) - 1, static_cast<int>(input.num_cols()) - 2};
        const auto nodes = condense(input, start, end);
        const auto result = walk_nodes_recursive(nodes, start, end, &can_move_sloped);
        return std::to_string(result);
    }

    /*
    --- Part Two ---
    As you reach the trailhead, you realize that the ground isn't as slippery as you expected; you'll have no problem climbing up the steep slopes.

    Now, treat all slopes as if they were normal paths (.). You still want to make sure you have the most scenic hike possible, so continue to ensure that you never step onto the same tile twice. What is the longest hike you can take?

    In the example above, this increases the longest hike to 154 steps:

    #S#####################
    #OOOOOOO#########OOO###
    #######O#########O#O###
    ###OOOOO#.>OOO###O#O###
    ###O#####.#O#O###O#O###
    ###O>...#.#O#OOOOO#OOO#
    ###O###.#.#O#########O#
    ###OOO#.#.#OOOOOOO#OOO#
    #####O#.#.#######O#O###
    #OOOOO#.#.#OOOOOOO#OOO#
    #O#####.#.#O#########O#
    #O#OOO#...#OOO###...>O#
    #O#O#O#######O###.###O#
    #OOO#O>.#...>O>.#.###O#
    #####O#.#.###O#.#.###O#
    #OOOOO#...#OOO#.#.#OOO#
    #O#########O###.#.#O###
    #OOO###OOO#OOO#...#O###
    ###O###O#O###O#####O###
    #OOO#OOO#O#OOO>.#.>O###
    #O###O###O#O###.#.#O###
    #OOOOO###OOO###...#OOO#
    #####################O#
    Find the longest hike you can take through the surprisingly dry hiking trails listed on your map. How many steps long is the longest hike?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const position start {0, 1}, end{static_cast<int>(input.num_rows()) - 1, static_cast<int>(input.num_cols()) - 2};
        const auto nodes = condense(input, start, end);
        const auto result = walk_nodes_recursive(nodes, start, end, &can_move_slopeless);
        return std::to_string(result);
    }

    aoc::registration r{2023, 23, part_1, part_2};

//    TEST_SUITE("2023_day23") {
//        TEST_CASE("2023_day23:example") {
//
//        }
//    }

}