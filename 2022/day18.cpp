//
// Created by Dan on 12/18/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "point_nd.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    using point_3d = point<3>;

    constexpr int MIN_FREE = -1;
    constexpr int MAX_FREE = 22;

    bool operator<(const point_3d& lhs, const point_3d& rhs) noexcept {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    point_3d parse_point(std::string_view s) {
        const auto parts = split(s, ',');
        return {parse<int>(parts[0]), parse<int>(parts[1]), parse<int>(parts[2])};
    }

    std::vector<point_3d> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_18_input.txt");
        auto retval = lines | std::views::transform(parse_point) | to<std::vector<point_3d>>();
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    bool can_get_free(const std::vector<point_3d>& input, const point_3d& start) {
        const auto dirs = get_cardinal_directions<3>();
        std::vector<point_3d> queue, checked;
        queue.push_back(start);
        while (!queue.empty()) {
            const auto p = queue.front();
            queue.erase(queue.begin());
            const auto done = std::lower_bound(checked.begin(), checked.end(), p);
            if (done != checked.end() && *done == p) {
                continue;
            }
            else {
                checked.insert(done, p);
            }

            for (const auto &d: dirs) {
                const auto n = p + d;
                const auto found = std::lower_bound(input.begin(), input.end(), n);
                if (found == input.end() || *found != n) {
                    if (n[0] == MIN_FREE || n[0] == MAX_FREE ||
                        n[1] == MIN_FREE || n[1] == MAX_FREE ||
                        n[2] == MIN_FREE || n[2] == MAX_FREE) {
                        return true;
                    } else {
                        queue.push_back(n);
                    }
                }
            }
        }
        return false;
    }

    /*
    --- Day 18: Boiling Boulders ---
    You and the elephants finally reach fresh air. You've emerged near the base of a large volcano that seems to be actively erupting! Fortunately, the lava seems to be flowing away from you and toward the ocean.

    Bits of lava are still being ejected toward you, so you're sheltering in the cavern exit a little longer. Outside the cave, you can see the lava landing in a pond and hear it loudly hissing as it solidifies.

    Depending on the specific compounds in the lava and speed at which it cools, it might be forming obsidian! The cooling rate should be based on the surface area of the lava droplets, so you take a quick scan of a droplet as it flies past you (your puzzle input).

    Because of how quickly the lava is moving, the scan isn't very good; its resolution is quite low and, as a result, it approximates the shape of the lava droplet with 1x1x1 cubes on a 3D grid, each given as its x,y,z position.

    To approximate the surface area, count the number of sides of each cube that are not immediately connected to another cube. So, if your scan were only two adjacent cubes like 1,1,1 and 2,1,1, each cube would have a single side covered and five sides exposed, a total surface area of 10 sides.

    Here's a larger example:

    2,2,2
    1,2,2
    3,2,2
    2,1,2
    2,3,2
    2,2,1
    2,2,3
    2,2,4
    2,2,6
    1,2,5
    3,2,5
    2,1,5
    2,3,5
    In the above example, after counting up all the sides that aren't connected to another cube, the total surface area is 64.

    What is the surface area of your scanned lava droplet?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto dirs = get_cardinal_directions<3>();
        int free_faces = 0;
        for (const auto& p : input) {
            for (const auto& d : dirs) {
                const auto n = p + d;
                const auto found = std::lower_bound(input.begin(), input.end(), n);
                if (found == input.end() || *found != n) {
                    ++free_faces;
                }
            }
        }
        return std::to_string(free_faces);
    }

    /*
    --- Part Two ---
    Something seems off about your calculation. The cooling rate depends on exterior surface area, but your calculation also included the surface area of air pockets trapped in the lava droplet.

    Instead, consider only cube sides that could be reached by the water and steam as the lava droplet tumbles into the pond. The steam will expand to reach as much as possible, completely displacing any air on the outside of the lava droplet but never expanding diagonally.

    In the larger example above, exactly one cube of air is trapped within the lava droplet (at 2,2,5), so the exterior surface area of the lava droplet is 58.

    What is the exterior surface area of your scanned lava droplet?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto dirs = get_cardinal_directions<3>();
        int free_faces = 0;
        for (const auto& p : input) {
            for (const auto& d : dirs) {
                const auto n = p + d;
                const auto found = std::lower_bound(input.begin(), input.end(), n);
                if ((found == input.end() || *found != n) && can_get_free(input, n)) {
                    ++free_faces;
                }
            }
        }
        return std::to_string(free_faces);
    }

    aoc::registration r{2022, 18, part_1, part_2};

//    TEST_SUITE("2022_day18") {
//        TEST_CASE("2022_day18:example") {
//
//        }
//    }

}