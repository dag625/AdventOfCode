//
// Created by Daniel Garcia on 12/17/20.
//

#include "day17.h"
#include "utilities.h"
#include "grid.h"

#include <iostream>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        struct cell {
            position3d pos;
            bool active = false;
            bool was_active = false;
        };

        bool operator<(const cell& a, const cell& b) noexcept {
            return std::tie(a.pos.x, a.pos.y, a.pos.z) < std::tie(b.pos.x, b.pos.y, b.pos.z);
        }

        bool operator==(const cell& a, const cell& b) noexcept {
            return std::tie(a.pos.x, a.pos.y, a.pos.z) == std::tie(b.pos.x, b.pos.y, b.pos.z);
        }

        bool operator<(const cell& a, const position3d& b) noexcept {
            return std::tie(a.pos.x, a.pos.y, a.pos.z) < std::tie(b.x, b.y, b.z);
        }

        std::vector<cell> get_active(const grid<char>& g) {
            std::vector<cell> retval;
            for (const auto p : g.list_positions()) {
                if (g[p] == '#') {
                    retval.push_back({p, true, true});
                }
            }
            std::sort(retval.begin(), retval.end());
            return retval;
        }

        void add_neighbors_of_active(std::vector<cell>& data) {
            std::vector<cell> neighbors;
            neighbors.reserve(data.size() * std::size(STANDARD_3D_DIRECTIONS));
            for (const auto& c : data) {
                if (c.active) {
                    for (const auto &n : STANDARD_3D_DIRECTIONS) {
                        neighbors.push_back({c.pos + n, false, false});
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

        int count_active(const std::vector<cell>& data, const cell& c) {
            int retval = 0;
            for (const auto& n : STANDARD_3D_DIRECTIONS) {
                auto found = std::lower_bound(data.begin(), data.end(), c.pos + n);
                if (found != data.end() && found->was_active) {
                    ++retval;
                }
            }
            return retval;
        }

        void iterate(std::vector<cell>& data) {
            std::for_each(data.begin(), data.end(), [](cell& c){ c.was_active = c.active; });
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
        auto cells = get_active(get_input(input_dir));
        add_neighbors_of_active(cells);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        std::cout << '\t' << std::count_if(cells.begin(), cells.end(), [](const cell& c){ return c.active; }) << '\n';
    }

    /*

    */
    void solve_day_17_2(const std::filesystem::path& input_dir) {
        auto cells = get_active(get_input(input_dir));
        std::cout << '\t' << 0 << '\n';
    }

} /* namespace aoc2020 */