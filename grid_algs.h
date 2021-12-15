//
// Created by Dan on 12/15/2021.
//

#ifndef ADVENTOFCODE_GRID_ALGS_H
#define ADVENTOFCODE_GRID_ALGS_H

#include "grid.h"

#include <algorithm>
#include <cmath>
#include <concepts>

namespace aoc {

    int grid_pos_distance(const position a, const position b) {
        const auto v = b - a;
        return static_cast<int>(std::round(std::sqrt(v.x * v.x + v.y * v.y)));
    }

    template <typename T>
    int64_t grid_pos_distance_heuristic(const grid<T>&, const position a, const position b) {
        return grid_pos_distance(a, b);
    }

    auto operator<=>(const position& lhs, const position& rhs) noexcept {
        return std::tie(lhs.x, lhs.y) <=> std::tie(rhs.x, rhs.y);
    }

    template <class F, typename T>
    concept cost_function = std::regular_invocable<F, const grid<T>&, position> && std::convertible_to<std::invoke_result_t<F, const grid<T>&, position>, int64_t>;

    template <class F, typename T>
    concept heuristic_function = std::regular_invocable<F, const grid<T>&, position, position> && std::convertible_to<std::invoke_result_t<F, const grid<T>&, position, position>, int64_t>;

    class grid_a_star_info {
    public:
        struct info {
            position pos;
            int64_t f_score = 0;

            info(position p, int64_t fs) : pos{p}, f_score{fs} {}

            bool operator<(const info& rhs) const noexcept {
                //Want a min-heap, but the std-library algorithms create a max heap.
                return f_score > rhs.f_score;
            }
        };
        struct grid_info {
            position prev {-1, -1};
            int64_t score = std::numeric_limits<int64_t>::max();
            int64_t h_score = 0;
            int64_t f_score = std::numeric_limits<int64_t>::max();

            bool update(const position last, int64_t cost, int64_t h_cost) {
                if (cost < score) {
                    prev = last;
                    score = cost;
                    h_score = h_cost;
                    f_score = score + h_score;
                    return true;
                }
                else {
                    return false;
                }
            }
        };
    private:
        std::vector<info> m_heap;
        grid<grid_info> m_grid;
    public:
        template <typename T, heuristic_function<T> HF>
        grid_a_star_info(const grid<T>& g, const position start, const position stop, HF heuristic_func) :
            m_heap{},
            m_grid{g.num_rows(), g.num_cols()}
        {
            check_and_update(start, start, 0, heuristic_func(g, start, stop));
        }

        [[nodiscard]] bool more() const { return !m_heap.empty(); }
        [[nodiscard]] const info& next() const { return m_heap.front(); }
        void pop() { std::pop_heap(m_heap.begin(), m_heap.end());  m_heap.erase(m_heap.end() - 1); }
        [[nodiscard]] info pop_next() { auto retval = next(); pop(); return retval; }

        [[nodiscard]] int64_t score(position p) const { return m_grid[p].score; }
        [[nodiscard]] std::vector<position> path(const position start, const position stop) const {
            std::vector<position> retval;
            position current = stop;
            while (current != start) {
                retval.push_back(current);
                current = m_grid[current].prev;
            }
            retval.push_back(start);
            return {retval.rbegin(), retval.rend()};
        }

        void check_and_update(const position p, const position last, const int64_t cost, const int64_t h_cost) {
            if (cost < score(p)) {
                m_grid[p].update(last, cost, h_cost);
                auto found = std::find_if(m_heap.begin(), m_heap.end(), [p](const info& i){ return i.pos == p; });
                if (found != m_heap.end()) {
                    found->f_score = m_grid[p].f_score;
                    std::make_heap(m_heap.begin(), m_heap.end());
                }
                else {
                    m_heap.emplace_back(p, cost + h_cost);
                    std::push_heap(m_heap.begin(), m_heap.end());
                }
            }
        }
    };

    struct grid_a_star_result {
        int64_t cost;
        std::vector<position> path;
    };

    template <typename T, cost_function<T> CF, heuristic_function<T> HF = decltype(&grid_pos_distance_heuristic<T>)>
    std::optional<grid_a_star_result> grid_a_star(const grid<T>& g, const position start, const position stop, CF cost_func, bool cardinal_neighbors_only = true, HF heuristic_func = grid_pos_distance_heuristic<T>) {
        grid_a_star_info info {g, start, stop, heuristic_func};

        while (info.more()) {
            const auto current = info.next();
            if (current.pos == stop) {
                return grid_a_star_result{ info.score(stop), info.path(start, stop)};
            }
            info.pop();
            for (const auto n : g.neighbors(current.pos, cardinal_neighbors_only)) {
                const auto cost = cost_func(g, n) + info.score(current.pos);
                const auto h_cost = heuristic_func(g, n, stop);
                info.check_and_update(n, current.pos, cost, h_cost);
            }
        }
        return std::nullopt;
    }

    template <std::integral T, heuristic_function<T> HF = decltype(&grid_pos_distance_heuristic<T>)>
    std::optional<grid_a_star_result>  grid_a_star(const grid<T>& g, const position start, const position stop, bool cardinal_neighbors_only = true, HF heuristic_func = grid_pos_distance_heuristic<T>) {
        return grid_a_star(g, start, stop, [](const auto& gg, const auto pos){ return gg[pos]; }, cardinal_neighbors_only, heuristic_func);
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_GRID_ALGS_H
