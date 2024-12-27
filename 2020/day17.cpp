//
// Created by Daniel Garcia on 12/17/20.
//

#include "registration.h"
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
            add_neighbors_of_active(data);
        }

        grid<char> get_input(const std::vector<std::string>& lines) {
            return to_grid(lines);
        }

    }

    /*
    As your flight slowly drifts through the sky, the Elves at the Mythical Information Bureau at the North Pole contact you. They'd like some help debugging a malfunctioning experimental energy source aboard one of their super-secret imaging satellites.

    The experimental energy source is based on cutting-edge technology: a set of Conway Cubes contained in a pocket dimension! When you hear it's having problems, you can't help but agree to take a look.

    The pocket dimension contains an infinite 3-dimensional grid. At every integer 3-dimensional coordinate (x,y,z), there exists a single cube which is either active or inactive.

    In the initial state of the pocket dimension, almost all cubes start inactive. The only exception to this is a small flat region of cubes (your puzzle input); the cubes in this region start in the specified active (#) or inactive (.) state.

    The energy source then proceeds to boot up by executing six cycles.

    Each cube only ever considers its neighbors: any of the 26 other cubes where any of their coordinates differ by at most 1. For example, given the cube at x=1,y=2,z=3, its neighbors include the cube at x=2,y=2,z=2, the cube at x=0,y=2,z=3, and so on.

    During a cycle, all cubes simultaneously change their state according to the following rules:

    If a cube is active and exactly 2 or 3 of its neighbors are also active, the cube remains active. Otherwise, the cube becomes inactive.
    If a cube is inactive but exactly 3 of its neighbors are active, the cube becomes active. Otherwise, the cube remains inactive.
    The engineers responsible for this experimental energy source would like you to simulate the pocket dimension and determine what the configuration of cubes should be at the end of the six-cycle boot process.

    For example, consider the following initial state:

    .#.
    ..#
    ###
    Even though the pocket dimension is 3-dimensional, this initial state represents a small 2-dimensional slice of it. (In particular, this initial state defines a 3x3x1 region of the 3-dimensional space.)

    Simulating a few cycles from this initial state produces the following configurations, where the result of each cycle is shown layer-by-layer at each given z coordinate (and the frame of view follows the active cells in each cycle):

    Before any cycles:

    z=0
    .#.
    ..#
    ###


    After 1 cycle:

    z=-1
    #..
    ..#
    .#.

    z=0
    #.#
    .##
    .#.

    z=1
    #..
    ..#
    .#.


    After 2 cycles:

    z=-2
    .....
    .....
    ..#..
    .....
    .....

    z=-1
    ..#..
    .#..#
    ....#
    .#...
    .....

    z=0
    ##...
    ##...
    #....
    ....#
    .###.

    z=1
    ..#..
    .#..#
    ....#
    .#...
    .....

    z=2
    .....
    .....
    ..#..
    .....
    .....


    After 3 cycles:

    z=-2
    .......
    .......
    ..##...
    ..###..
    .......
    .......
    .......

    z=-1
    ..#....
    ...#...
    #......
    .....##
    .#...#.
    ..#.#..
    ...#...

    z=0
    ...#...
    .......
    #......
    .......
    .....##
    .##.#..
    ...#...

    z=1
    ..#....
    ...#...
    #......
    .....##
    .#...#.
    ..#.#..
    ...#...

    z=2
    .......
    .......
    ..##...
    ..###..
    .......
    .......
    .......
    After the full six-cycle boot process completes, 112 cubes are left in the active state.

    Starting with your given initial configuration, simulate six cycles. How many cubes are left in the active state after the sixth cycle?
    */
    std::string solve_day_17_1(const std::vector<std::string>& lines) {
        auto cells = get_active<3>(get_input(lines));
        add_neighbors_of_active(cells);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        return std::to_string(std::count_if(cells.begin(), cells.end(), [](const cell<3>& c){ return c.active; }));
    }

    /*
    For some reason, your simulated results don't match what the experimental energy source engineers expected. Apparently, the pocket dimension actually has four spatial dimensions, not three.

    The pocket dimension contains an infinite 4-dimensional grid. At every integer 4-dimensional coordinate (x,y,z,w), there exists a single cube (really, a hypercube) which is still either active or inactive.

    Each cube only ever considers its neighbors: any of the 80 other cubes where any of their coordinates differ by at most 1. For example, given the cube at x=1,y=2,z=3,w=4, its neighbors include the cube at x=2,y=2,z=3,w=3, the cube at x=0,y=2,z=3,w=4, and so on.

    The initial state of the pocket dimension still consists of a small flat region of cubes. Furthermore, the same rules for cycle updating still apply: during each cycle, consider the number of active neighbors of each cube.

    For example, consider the same initial state as in the example above. Even though the pocket dimension is 4-dimensional, this initial state represents a small 2-dimensional slice of it. (In particular, this initial state defines a 3x3x1x1 region of the 4-dimensional space.)

    Simulating a few cycles from this initial state produces the following configurations, where the result of each cycle is shown layer-by-layer at each given z and w coordinate:

    Before any cycles:

    z=0, w=0
    .#.
    ..#
    ###


    After 1 cycle:

    z=-1, w=-1
    #..
    ..#
    .#.

    z=0, w=-1
    #..
    ..#
    .#.

    z=1, w=-1
    #..
    ..#
    .#.

    z=-1, w=0
    #..
    ..#
    .#.

    z=0, w=0
    #.#
    .##
    .#.

    z=1, w=0
    #..
    ..#
    .#.

    z=-1, w=1
    #..
    ..#
    .#.

    z=0, w=1
    #..
    ..#
    .#.

    z=1, w=1
    #..
    ..#
    .#.


    After 2 cycles:

    z=-2, w=-2
    .....
    .....
    ..#..
    .....
    .....

    z=-1, w=-2
    .....
    .....
    .....
    .....
    .....

    z=0, w=-2
    ###..
    ##.##
    #...#
    .#..#
    .###.

    z=1, w=-2
    .....
    .....
    .....
    .....
    .....

    z=2, w=-2
    .....
    .....
    ..#..
    .....
    .....

    z=-2, w=-1
    .....
    .....
    .....
    .....
    .....

    z=-1, w=-1
    .....
    .....
    .....
    .....
    .....

    z=0, w=-1
    .....
    .....
    .....
    .....
    .....

    z=1, w=-1
    .....
    .....
    .....
    .....
    .....

    z=2, w=-1
    .....
    .....
    .....
    .....
    .....

    z=-2, w=0
    ###..
    ##.##
    #...#
    .#..#
    .###.

    z=-1, w=0
    .....
    .....
    .....
    .....
    .....

    z=0, w=0
    .....
    .....
    .....
    .....
    .....

    z=1, w=0
    .....
    .....
    .....
    .....
    .....

    z=2, w=0
    ###..
    ##.##
    #...#
    .#..#
    .###.

    z=-2, w=1
    .....
    .....
    .....
    .....
    .....

    z=-1, w=1
    .....
    .....
    .....
    .....
    .....

    z=0, w=1
    .....
    .....
    .....
    .....
    .....

    z=1, w=1
    .....
    .....
    .....
    .....
    .....

    z=2, w=1
    .....
    .....
    .....
    .....
    .....

    z=-2, w=2
    .....
    .....
    ..#..
    .....
    .....

    z=-1, w=2
    .....
    .....
    .....
    .....
    .....

    z=0, w=2
    ###..
    ##.##
    #...#
    .#..#
    .###.

    z=1, w=2
    .....
    .....
    .....
    .....
    .....

    z=2, w=2
    .....
    .....
    ..#..
    .....
    .....
    After the full six-cycle boot process completes, 848 cubes are left in the active state.

    Starting with your given initial configuration, simulate six cycles in a 4-dimensional space. How many cubes are left in the active state after the sixth cycle?
    */
    std::string solve_day_17_2(const std::vector<std::string>& lines) {
        auto cells = get_active<4>(get_input(lines));
        add_neighbors_of_active(cells);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        return std::to_string(std::count_if(cells.begin(), cells.end(), [](const cell<4>& c){ return c.active; }));
    }

    static aoc::registration r {2020, 17, solve_day_17_1, solve_day_17_2};

} /* namespace aoc2020 */