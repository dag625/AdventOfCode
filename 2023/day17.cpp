//
// Created by Dan on 12/16/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>
#include <compare>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    int parse_int(const char c) {
        return static_cast<int>(c - '0');
    }

    grid<int> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_17_input.txt");
        const auto row_len = lines.front().size();
        return {lines | std::views::join | std::views::transform(&parse_int) | to<std::vector<int>>(), row_len};
    }

    enum class direction : int {
        none,
        up,
        down,
        left,
        right
    };

    inline std::ostream& operator<<(std::ostream& os, const direction d) {
        switch (d) {
            case direction::none:  os << "None"; break;
            case direction::up:  os << "Up"; break;
            case direction::down:  os << "Down"; break;
            case direction::left:  os << "Left"; break;
            case direction::right:  os << "Right"; break;
            default: break;
        }
        return os;
    }

    struct state {
        int x = 0;
        int y = 0;
        direction from_dir = direction::none;
        int dist = 0;

        state() = default;
        state(position p) : x{p.x}, y{p.y} {}

        [[nodiscard]] position position() const { return {x, y}; }

        [[nodiscard]] std::vector<state> neighbors(const grid<int>& g, int min_dist, int max_dist) const {
            std::vector<state> retval;
            retval.reserve(4 * (max_dist - min_dist + 1));
            if (from_dir != direction::up && from_dir != direction::down) {
                for (int d = min_dist; d <= max_dist; ++d) {
                    state n = *this;
                    n.dist = d;
                    n.from_dir = direction::up;
                    n.x += d;
                    if (g.in({n.x, n.y})) {
                        retval.push_back(n);
                    }
                }
            }
            if (from_dir != direction::down && from_dir != direction::up) {
                for (int d = min_dist; d <= max_dist; ++d) {
                    state n = *this;
                    n.dist = d;
                    n.from_dir = direction::down;
                    n.x -= d;
                    if (g.in({n.x, n.y})) {
                        retval.push_back(n);
                    }
                }
            }
            if (from_dir != direction::left && from_dir != direction::right) {
                for (int d = min_dist; d <= max_dist; ++d) {
                    state n = *this;
                    n.dist = d;
                    n.from_dir = direction::left;
                    n.y += d;
                    if (g.in({n.x, n.y})) {
                        retval.push_back(n);
                    }
                }
            }
            if (from_dir != direction::right && from_dir != direction::left) {
                for (int d = min_dist; d <= max_dist; ++d) {
                    state n = *this;
                    n.dist = d;
                    n.from_dir = direction::right;
                    n.y -= d;
                    if (g.in({n.x, n.y})) {
                        retval.push_back(n);
                    }
                }
            }
            return retval;
        }

        bool operator==(const state&) const = default;
        auto operator<=>(const state&) const = default;
    };

    inline std::ostream& operator<<(std::ostream& os, const state& s) {
        os << s.position() << " " << s.from_dir << "-" << s.dist;
        return os;
    }

    std::optional<int64_t> cost_f(const grid<int>& g, const state& prev, const state& current) {
        if (prev.from_dir == current.from_dir) {
            return std::nullopt;
        }
        const auto del = current.position() - prev.position();
        const velocity vel { del.x != 0 ? del.x / std::abs(del.x) : 0, del.y != 0 ? del.y / std::abs(del.y) : 0 };
        int64_t cost = 0;
        for (position p = prev.position() + vel; p != current.position() + vel; p += vel) {
            cost += g[p];
        }
        return cost;
    }

    constexpr int MAX_PAST_SIZE = 4;

    struct day17_dijkstra_pos {
        state current;
        state prev;
        int64_t cost = std::numeric_limits<int64_t>::max();

        day17_dijkstra_pos() = default;
        day17_dijkstra_pos(state c, state p) : current{c}, prev{p} {}
        day17_dijkstra_pos(state c, state p, int64_t ct) : current{c}, prev{p}, cost{ct} {}

        bool operator<(const day17_dijkstra_pos& rhs) const noexcept {
            //Want a min-heap, but the std-library algorithms create a max heap.
            return cost > rhs.cost;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const day17_dijkstra_pos& p) {
        os << p.current << " <= " << p.prev << " @ " << p.cost;
        return os;
    }

    using day17_dijkstra_result = std::vector<day17_dijkstra_pos>;

    day17_dijkstra_result day17_dijkstra(const grid<int>& g, const position start, int min_dist, int max_dist) {
        day17_dijkstra_result retval;
        retval.reserve(g.num_rows() * g.num_cols() * 4 * (max_dist - min_dist + 1));
        const auto retval_less = [](const day17_dijkstra_pos& a, const state& b){ return a.current < b; };
        retval.emplace_back(start, start, 0);
        std::vector<day17_dijkstra_pos> queue;
        queue.emplace_back(start, start, 0);
        while (!queue.empty()) {
            const auto next = queue.front();
            std::pop_heap(queue.begin(), queue.end());
            queue.erase(queue.end() - 1);

            const auto neighbors = next.current.neighbors(g, min_dist, max_dist);
            for (const auto& n : neighbors) {
                const std::optional<int64_t> step_cost = cost_f(g, next.current, n);
                if (!step_cost) {
                    continue;
                }
                const auto total_cost = next.cost + *step_cost;
                auto n_found = std::lower_bound(retval.begin(), retval.end(), n, retval_less);
                if (n_found == retval.end() || n_found->current != n) {
                    n_found = retval.emplace(n_found, n, next.current);
                }
                if (total_cost <= n_found->cost) {
                    n_found->cost = total_cost;
                    n_found->prev = next.current;

                    auto found = std::find_if(queue.begin(), queue.end(), [&n](const day17_dijkstra_pos& q){ return q.current == n; });
                    if (found != queue.end()) {
                        found->cost = total_cost;
                        std::make_heap(queue.begin(), queue.end());
                    }
                    else {
                        queue.emplace_back(n, next.current, total_cost);
                        std::push_heap(queue.begin(), queue.end());
                    }
                }
            }
        }
        return retval;
    }

    int64_t get_best_cost(const day17_dijkstra_result& res, position p) {
        const auto lower_less = [](const day17_dijkstra_pos& a, const position& b){ return a.current.position() < b; };
        const auto upper_less = [](const position& a, const day17_dijkstra_pos& b){ return a < b.current.position(); };
        const auto start = std::lower_bound(res.begin(), res.end(), p, lower_less);
        const auto finish = std::upper_bound(res.begin(), res.end(), p, upper_less);
        int64_t retval = std::numeric_limits<int64_t>::max();
        for (auto current = start; current != finish; ++current) {
            if (current->cost < retval) {
                retval = current->cost;
            }
        }
        return retval;
    }

    /*
    --- Day 17: Clumsy Crucible ---
    The lava starts flowing rapidly once the Lava Production Facility is operational. As you leave, the reindeer offers you a parachute, allowing you to quickly reach Gear Island.

    As you descend, your bird's-eye view of Gear Island reveals why you had trouble finding anyone on your way up: half of Gear Island is empty, but the half below you is a giant factory city!

    You land near the gradually-filling pool of lava at the base of your new lavafall. Lavaducts will eventually carry the lava throughout the city, but to make use of it immediately, Elves are loading it into large crucibles on wheels.

    The crucibles are top-heavy and pushed by hand. Unfortunately, the crucibles become very difficult to steer at high speeds, and so it can be hard to go in a straight line for very long.

    To get Desert Island the machine parts it needs as soon as possible, you'll need to find the best way to get the crucible from the lava pool to the machine parts factory. To do this, you need to minimize heat loss while choosing a route that doesn't require the crucible to go in a straight line for too long.

    Fortunately, the Elves here have a map (your puzzle input) that uses traffic patterns, ambient temperature, and hundreds of other parameters to calculate exactly how much heat loss can be expected for a crucible entering any particular city block.

    For example:

    2413432311323
    3215453535623
    3255245654254
    3446585845452
    4546657867536
    1438598798454
    4457876987766
    3637877979653
    4654967986887
    4564679986453
    1224686865563
    2546548887735
    4322674655533
    Each city block is marked by a single digit that represents the amount of heat loss if the crucible enters that block. The starting point, the lava pool, is the top-left city block; the destination, the machine parts factory, is the bottom-right city block. (Because you already start in the top-left block, you don't incur that block's heat loss unless you leave that block and then return to it.)

    Because it is difficult to keep the top-heavy crucible going in a straight line for very long, it can move at most three blocks in a single direction before it must turn 90 degrees left or right. The crucible also can't reverse direction; after entering each city block, it may only turn left, continue straight, or turn right.

    One way to minimize heat loss is this path:

    2>>34^>>>1323
    32v>>>35v5623
    32552456v>>54
    3446585845v52
    4546657867v>6
    14385987984v4
    44578769877v6
    36378779796v>
    465496798688v
    456467998645v
    12246868655<v
    25465488877v5
    43226746555v>
    This path never moves more than three consecutive blocks in the same direction and incurs a heat loss of only 102.

    Directing the crucible from the lava pool to the machine parts factory, but not moving more than three consecutive blocks in the same direction, what is the least heat loss it can incur?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto res = day17_dijkstra(input, {}, 1, 3);
        const position end_pos {static_cast<int>(input.num_cols()) - 1, static_cast<int>(input.num_rows()) - 1};
        return std::to_string(get_best_cost(res, end_pos));
    }

    /*
    --- Part Two ---
    The crucibles of lava simply aren't large enough to provide an adequate supply of lava to the machine parts factory. Instead, the Elves are going to upgrade to ultra crucibles.

    Ultra crucibles are even more difficult to steer than normal crucibles. Not only do they have trouble going in a straight line, but they also have trouble turning!

    Once an ultra crucible starts moving in a direction, it needs to move a minimum of four blocks in that direction before it can turn (or even before it can stop at the end). However, it will eventually start to get wobbly: an ultra crucible can move a maximum of ten consecutive blocks without turning.

    In the above example, an ultra crucible could follow this path to minimize heat loss:

    2>>>>>>>>1323
    32154535v5623
    32552456v4254
    34465858v5452
    45466578v>>>>
    143859879845v
    445787698776v
    363787797965v
    465496798688v
    456467998645v
    122468686556v
    254654888773v
    432267465553v
    In the above example, an ultra crucible would incur the minimum possible heat loss of 94.

    Here's another example:

    111111111111
    999999999991
    999999999991
    999999999991
    999999999991
    Sadly, an ultra crucible would need to take an unfortunate path like this one:

    1>>>>>>>1111
    9999999v9991
    9999999v9991
    9999999v9991
    9999999v>>>>
    This route causes the ultra crucible to incur the minimum possible heat loss of 71.

    Directing the ultra crucible from the lava pool to the machine parts factory, what is the least heat loss it can incur?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto res = day17_dijkstra(input, {}, 4, 10);
        const position end_pos {static_cast<int>(input.num_cols()) - 1, static_cast<int>(input.num_rows()) - 1};
        return std::to_string(get_best_cost(res, end_pos));
    }

    aoc::registration r{2023, 17, part_1, part_2};

    TEST_SUITE("2023_day17") {
        TEST_CASE("2023_day17:example") {
            std::string data = "2413432311323"
                               "3215453535623"
                               "3255245654254"
                               "3446585845452"
                               "4546657867536"
                               "1438598798454"
                               "4457876987766"
                               "3637877979653"
                               "4654967986887"
                               "4564679986453"
                               "1224686865563"
                               "2546548887735"
                               "4322674655533";
            const int row_len = 13;
            const grid<int> input {data | std::views::transform(&parse_int) | to<std::vector<int>>(), row_len};
            const auto res = day17_dijkstra(input, {}, 1, 3);
            const position end_pos {static_cast<int>(input.num_cols()) - 1, static_cast<int>(input.num_rows()) - 1};
            const auto val = get_best_cost(res, end_pos);
            CHECK_EQ(val, 102);
        }
    }

}