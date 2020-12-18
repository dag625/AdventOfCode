//
// Created by Daniel Garcia on 12/17/20.
//

#include "day17.h"
#include "utilities.h"
#include "grid.h"
#include "point_nd.h"

#include <iostream>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        template <std::size_t D>
        struct cell {
            point<D> pos{};
            bool active = false;
            bool was_active = false;
        };

        template <std::size_t D>
        bool operator<(const cell<D>& a, const cell<D>& b) noexcept {
            if (a.pos < b.pos) {
                return true;
            }
            else if (a.pos == b.pos) {
                return a.active > b.active;
            }
            else {
                return false;
            }
        }

        template <std::size_t D>
        bool operator==(const cell<D>& a, const cell<D>& b) noexcept {
            return a.pos == b.pos;
        }

        template <std::size_t D>
        bool operator<(const cell<D>& a, const point<D>& b) noexcept {
            return a.pos < b;
        }

        template <std::size_t D>
        std::vector<cell<D>> get_active(const grid<char>& g) {
            std::vector<cell<D>> retval;
            for (const auto p : g.list_positions()) {
                if (g[p] == '#') {
                    retval.push_back({from_flat<D>(p.x, p.y), true, true});
                }
            }
            std::sort(retval.begin(), retval.end());
            return retval;
        }

        template <std::size_t D>
        void add_neighbors_of_active(std::vector<cell<D>>& data) {
            std::vector<cell<D>> neighbors;
            neighbors.reserve(data.size() * std::size(get_standard_directions<D>()));
            for (const auto& c : data) {
                if (c.active) {
                    for (const auto &n : get_standard_directions<D>()) {
                        neighbors.push_back({ c.pos + n, false, false});
                    }
                }
            }
            std::sort(neighbors.begin(), neighbors.end());
            neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
            auto size = data.size();
            data.insert(data.end(), neighbors.begin(), neighbors.end());
            std::inplace_merge(data.begin(), data.begin() + size, data.end());
            data.erase(std::unique(data.begin(), data.end()), data.end());
        }

        template <std::size_t D>
        int count_active(const std::vector<cell<D>>& data, const cell<D>& c) {
            int retval = 0;
            for (const auto& n : get_standard_directions<D>()) {
                auto pos = c.pos + n;
                auto found = std::lower_bound(data.begin(), data.end(), pos);
                if (found != data.end() && found->pos == pos && found->was_active) {
                    ++retval;
                }
            }
            return retval;
        }

        template <std::size_t D>
        void iterate(std::vector<cell<D>>& data) {
            std::for_each(data.begin(), data.end(), [](cell<D>& c){ c.was_active = c.active; });
            for (auto& c : data) {
                auto num_active = count_active(data, c);
                if (num_active == 3) {
                    c.active = true;
                }
                else if (c.active && num_active != 2) {
                    c.active = false;
                }
            }
            add_neighbors_of_active(data);//todo we could prune this down, but it may or may not be worth it
        }

        grid<char> get_input(const fs::path &input_dir) {
            auto lines = read_file_lines(input_dir / "2020" / "day_17_input.txt");
            return to_grid(lines);
        }

    }

    /*

    */
    void solve_day_17_1(const std::filesystem::path& input_dir) {
        auto cells = get_active<3>(get_input(input_dir));
        add_neighbors_of_active(cells);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        std::cout << '\t' << std::count_if(cells.begin(), cells.end(), [](const cell<3>& c){ return c.active; }) << '\n';
    }

    /*

    */
    void solve_day_17_2(const std::filesystem::path& input_dir) {
        auto cells = get_active<4>(get_input(input_dir));
        add_neighbors_of_active(cells);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        std::cout << '\t' << std::count_if(cells.begin(), cells.end(), [](const cell<4>& c){ return c.active; }) << '\n';
    }

} /* namespace aoc2020 */