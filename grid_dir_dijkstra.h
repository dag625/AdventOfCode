//
// Created by Daniel Garcia on 12/12/2022.
//

#ifndef ADVENTOFCODE_GRID_DIJKSTRA_H
#define ADVENTOFCODE_GRID_DIJKSTRA_H

#include "grid.h"

#include <algorithm>
#include <optional>
#include <iostream>

namespace aoc {

    /*
     * grid_dir_dijkstra_cost_function is a function like:
     * std::optional<int64_t> cost_function(const grid<T>&, position current, velocity to_current, position next) {
     *      if (can_go) {
     *          return cost;
     *      }
     *      else {
     *          returm std::nullopt;
     *      }
     * }
     */
    template <class F, typename T>
    concept grid_dir_dijkstra_cost_function = std::regular_invocable<F, const grid<T>&, position, velocity, position> && std::convertible_to<std::invoke_result_t<F, const grid<T>&, position, velocity, position>, std::optional<int64_t>>;

    struct grid_dir_dijkstra_pos {
        position prev{};
        velocity to_current{};
        int64_t cost = std::numeric_limits<int64_t>::max();

        bool operator<(const grid_dir_dijkstra_pos& rhs) const noexcept {
            //Want a min-heap, but the std-library algorithms create a max heap.
            return cost > rhs.cost;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const grid_dir_dijkstra_pos& p) {
        os << p.cost;
        return os;
    }

    using grid_dir_dijkstra_result = grid<grid_dir_dijkstra_pos>;

    template <typename T, grid_dir_dijkstra_cost_function<T> F>
    grid_dir_dijkstra_result grid_dir_dijkstra(const grid<T>& g, const position start, const velocity init_v, F cost_func, bool cardinal_neighbors_only = true) {
        grid_dir_dijkstra_result retval {static_cast<std::size_t>(g.num_rows()), static_cast<std::size_t>(g.num_cols())};
        retval[start].cost = 0;
        retval[start].to_current = init_v;
        std::vector<grid_dir_dijkstra_pos> queue;
        queue.push_back({start, init_v, 0});
        while (!queue.empty()) {
            const auto next = queue.front();
            std::pop_heap(queue.begin(), queue.end());
            queue.erase(queue.end() - 1);

            const auto neighbors = retval.neighbors(next.prev, cardinal_neighbors_only);
            for (const auto n : neighbors) {
                const std::optional<int64_t> step_cost = cost_func(g, next.prev, next.to_current, n);
                if (!step_cost) {
                    continue;
                }
                const velocity to_n {n.x - next.prev.x, n.y - next.prev.y};
                const auto total_cost = next.cost + *step_cost;
                if (total_cost < retval[n].cost) {
                    retval[n].cost = total_cost;
                    retval[n].prev = next.prev;

                    auto found = std::find_if(queue.begin(), queue.end(), [&n](const grid_dir_dijkstra_pos& q){ return q.prev == n; });
                    if (found != queue.end()) {
                        found->cost = total_cost;
                        found->to_current = to_n;
                        std::make_heap(queue.begin(), queue.end());
                    }
                    else {
                        queue.emplace_back(n, to_n, total_cost);
                        std::push_heap(queue.begin(), queue.end());
                    }
                }
            }
        }
        return retval;
    }

    inline std::vector<position> grid_dir_dijkstra_get_path(const grid_dir_dijkstra_result& result, const position start, const position stop) {
        std::vector<position> reversed;
        position current = stop;
        while (current != start) {
            reversed.push_back(current);
            current = result[current].prev;
        }
        reversed.push_back(current);
        return {reversed.rbegin(), reversed.rend()};
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_GRID_DIJKSTRA_H
