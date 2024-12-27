//
// Created by Daniel Garcia on 12/3/20.
//

#include "registration.h"

#include "utilities.h"
#include "grid.h"

#include <string>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <array>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        constexpr char OPEN_SPACE = '.';
        constexpr char TREE_SPACE = '#';
        constexpr auto VALID_VALUES = std::array{ OPEN_SPACE, TREE_SPACE };

        grid<char> get_input(const std::vector<std::string>& lines) {
            grid<char> retval = to_grid(lines);
            if (std::any_of(retval.begin(), retval.end(),
                            [](char c){ return std::none_of(VALID_VALUES.begin(), VALID_VALUES.end(),
                                                     [c](char v){ return c == v; }); }))
            {
                throw std::runtime_error{"Invalid input data.  Input must contain only '.' and '#' characters."};
            }
            return retval;
        }

        int trees_in_space(const grid<char>& g, const position p) noexcept {
            //We assume wrap() has been called on the position since the last change.
            constexpr int denom = TREE_SPACE - OPEN_SPACE;
            return static_cast<int>(g[p.x][p.y] - OPEN_SPACE) / denom;
        }

        int num_trees_in_path(const grid<char>& g, position pos, const velocity vel) noexcept {
            int num_trees = 0;
            std::optional<position> op = pos;
            while (op) {
                num_trees += trees_in_space(g, *op);
                op = g.wrap(*op + vel);
            }
            return num_trees;
        }

    }

    /************************* Part 1 *************************/
    std::string solve_day_3_1(const std::vector<std::string>& lines) {
        const auto map = get_input(lines);
        return std::to_string(num_trees_in_path(map, top_left(), {1, 3}));
    }

    /************************* Part 2 *************************/
    std::string solve_day_3_2(const std::vector<std::string>& lines) {
        const auto map = get_input(lines);
        const auto pos = top_left();
        const velocity vels[] = { {1, 1}, {1, 3}, {1, 5}, {1, 7}, {2, 1} };
        return std::to_string(std::accumulate(std::begin(vels), std::end(vels), 1LL,
                                             [pos, &map](int64_t acc, velocity vel){ return acc * num_trees_in_path(map, pos, vel); }));
    }

    static registration r {2020, 3, solve_day_3_1, solve_day_3_2};

} /* namespace aoc2020 */