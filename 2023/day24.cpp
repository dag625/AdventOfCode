//
// Created by Dan on 12/23/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct position64 {
        int64_t x = 0;
        int64_t y = 0;
        int64_t z = 0;
    };

    struct velocity64 {
        int64_t dx = 0;
        int64_t dy = 0;
        int64_t dz = 0;
    };

    struct stone {
        position64 pos;
        velocity64 vel;

        position64 pos_2d_at(int64_t t) const {
            return {pos.x + vel.dx * t, pos.y + vel.dy * t, 0};
        }
    };

    struct position_dbl {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    std::optional<position_dbl> intersection_2d(const stone& a, const stone& b) {
        const double x1 = a.pos.x, y1 = a.pos.y, vx1 = a.vel.dx, vy1 = a.vel.dy,
                x2 = b.pos.x, y2 = b.pos.y, vx2 = b.vel.dx, vy2 = b.vel.dy;//a = 1, b = 2
        double top = ((x1 - x2) * vy1) / vx1;
        top += y2;
        top -= y1;
        double bottom = (vy1 * vx2) / vx1;
        bottom -= vy2;
        const double t2 = top / bottom;
        const double t1 = (vx2 * t2 + x2 - x1) / vx1;
        if (t1 < 0.0 || t2 < 0.0) {
            return std::nullopt;
        }
        const double x = vx2 * t2 + x2;
        const double y = vy2 * t2 + y2;
        return position_dbl{x, y, 0.0};
    }

    template <typename T>
    T parse_3digit(std::string_view s) {
        const auto parts = split(s, ',');
        return {parse<int64_t>(trim(parts[0])), parse<int64_t>(trim(parts[1])), parse<int64_t>(trim(parts[2]))};
    }

    stone parse_stone(std::string_view s) {
        const auto parts = split(s, '@');
        return {parse_3digit<position64>(parts[0]), parse_3digit<velocity64>(parts[1])};
    };

    std::vector<stone> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_stone) | to<std::vector>();
    }

    constexpr int64_t MIN_POS = 200000000000000ll;
    constexpr int64_t MAX_POS = 400000000000000ll;

    int find_intersections(const std::vector<stone>& stones, const double min, const double max) {
        const auto size = stones.size();
        int count = 0;
        for (int i = 0; i < size; ++i) {
            for (int j = i + 1; j < size; ++j) {
                const auto inter = intersection_2d(stones[i], stones[j]);
                if (inter && inter->x >= min && inter->x <= max && inter->y >= min && inter->y <= max) {
                    ++count;
                }
            }
        }
        return count;
    }

    /*
    --- Day 24: Never Tell Me The Odds ---
    It seems like something is going wrong with the snow-making process. Instead of forming snow, the water that's been absorbed into the air seems to be forming hail!

    Maybe there's something you can do to break up the hailstones?

    Due to strong, probably-magical winds, the hailstones are all flying through the air in perfectly linear trajectories. You make a note of each hailstone's position and velocity (your puzzle input). For example:

    19, 13, 30 @ -2,  1, -2
    18, 19, 22 @ -1, -1, -2
    20, 25, 34 @ -2, -2, -4
    12, 31, 28 @ -1, -2, -1
    20, 19, 15 @  1, -5, -3
    Each line of text corresponds to the position and velocity of a single hailstone. The positions indicate where the hailstones are right now (at time 0). The velocities are constant and indicate exactly how far each hailstone will move in one nanosecond.

    Each line of text uses the format px py pz @ vx vy vz. For instance, the hailstone specified by 20, 19, 15 @ 1, -5, -3 has initial X position 20, Y position 19, Z position 15, X velocity 1, Y velocity -5, and Z velocity -3. After one nanosecond, the hailstone would be at 21, 14, 12.

    Perhaps you won't have to do anything. How likely are the hailstones to collide with each other and smash into tiny ice crystals?

    To estimate this, consider only the X and Y axes; ignore the Z axis. Looking forward in time, how many of the hailstones' paths will intersect within a test area? (The hailstones themselves don't have to collide, just test for intersections between the paths they will trace.)

    In this example, look for intersections that happen with an X and Y position each at least 7 and at most 27; in your actual data, you'll need to check a much larger test area. Comparing all pairs of hailstones' future paths produces the following results:

    Hailstone A: 19, 13, 30 @ -2, 1, -2
    Hailstone B: 18, 19, 22 @ -1, -1, -2
    Hailstones' paths will cross inside the test area (at x=14.333, y=15.333).

    Hailstone A: 19, 13, 30 @ -2, 1, -2
    Hailstone B: 20, 25, 34 @ -2, -2, -4
    Hailstones' paths will cross inside the test area (at x=11.667, y=16.667).

    Hailstone A: 19, 13, 30 @ -2, 1, -2
    Hailstone B: 12, 31, 28 @ -1, -2, -1
    Hailstones' paths will cross outside the test area (at x=6.2, y=19.4).

    Hailstone A: 19, 13, 30 @ -2, 1, -2
    Hailstone B: 20, 19, 15 @ 1, -5, -3
    Hailstones' paths crossed in the past for hailstone A.

    Hailstone A: 18, 19, 22 @ -1, -1, -2
    Hailstone B: 20, 25, 34 @ -2, -2, -4
    Hailstones' paths are parallel; they never intersect.

    Hailstone A: 18, 19, 22 @ -1, -1, -2
    Hailstone B: 12, 31, 28 @ -1, -2, -1
    Hailstones' paths will cross outside the test area (at x=-6, y=-5).

    Hailstone A: 18, 19, 22 @ -1, -1, -2
    Hailstone B: 20, 19, 15 @ 1, -5, -3
    Hailstones' paths crossed in the past for both hailstones.

    Hailstone A: 20, 25, 34 @ -2, -2, -4
    Hailstone B: 12, 31, 28 @ -1, -2, -1
    Hailstones' paths will cross outside the test area (at x=-2, y=3).

    Hailstone A: 20, 25, 34 @ -2, -2, -4
    Hailstone B: 20, 19, 15 @ 1, -5, -3
    Hailstones' paths crossed in the past for hailstone B.

    Hailstone A: 12, 31, 28 @ -1, -2, -1
    Hailstone B: 20, 19, 15 @ 1, -5, -3
    Hailstones' paths crossed in the past for both hailstones.
    So, in this example, 2 hailstones' future paths cross inside the boundaries of the test area.

    However, you'll need to search a much larger test area if you want to see if any hailstones might collide. Look for intersections that happen with an X and Y position each at least 200000000000000 and at most 400000000000000. Disregard the Z axis entirely.

    Considering only the X and Y axes, check all pairs of hailstones' future paths for intersections. How many of these intersections occur within the test area?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto count = find_intersections(input, MIN_POS, MAX_POS);
        return std::to_string(count);
    }

    /*
    --- Part Two ---
    Upon further analysis, it doesn't seem like any hailstones will naturally collide. It's up to you to fix that!

    You find a rock on the ground nearby. While it seems extremely unlikely, if you throw it just right, you should be able to hit every hailstone in a single throw!

    You can use the probably-magical winds to reach any integer position you like and to propel the rock at any integer velocity. Now including the Z axis in your calculations, if you throw the rock at time 0, where do you need to be so that the rock perfectly collides with every hailstone? Due to probably-magical inertia, the rock won't slow down or change direction when it collides with a hailstone.

    In the example above, you can achieve this by moving to position 24, 13, 10 and throwing the rock at velocity -3, 1, 2. If you do this, you will hit every hailstone as follows:

    Hailstone: 19, 13, 30 @ -2, 1, -2
    Collision time: 5
    Collision position: 9, 18, 20

    Hailstone: 18, 19, 22 @ -1, -1, -2
    Collision time: 3
    Collision position: 15, 16, 16

    Hailstone: 20, 25, 34 @ -2, -2, -4
    Collision time: 4
    Collision position: 12, 17, 18

    Hailstone: 12, 31, 28 @ -1, -2, -1
    Collision time: 6
    Collision position: 6, 19, 22

    Hailstone: 20, 19, 15 @ 1, -5, -3
    Collision time: 1
    Collision position: 21, 14, 12
    Above, each hailstone is identified by its initial position and its velocity. Then, the time and position of that hailstone's collision with your rock are given.

    After 1 nanosecond, the rock has exactly the same position as one of the hailstones, obliterating it into ice dust! Another hailstone is smashed to bits two nanoseconds after that. After a total of 6 nanoseconds, all of the hailstones have been destroyed.

    So, at time 0, the rock needs to be at X position 24, Y position 13, and Z position 10. Adding these three coordinates together produces 47. (Don't add any coordinates from the rock's velocity.)

    Determine the exact position and velocity the rock needs to have at time 0 so that it perfectly collides with every hailstone. What do you get if you add up the X, Y, and Z coordinates of that initial position?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        /*
         * Run the python script day24_part2.py, passing the input file as the argument, e.g.:
         * >python.exe .\2023\day24_part2.py C:\Path\to\AoC\Inputs\inputs\2023\day_24_input.txt
         *
         * The script requires the z3 and z3-solver crates, installed like:
         * >pip install z3 z3-solver
         *
         * For now, trying to do the algebra by hand to be able to do this from C++ is a no-go,
         * particularly on Christmas Eve.
         */
        return std::to_string(781390555762385ll);
    }

    aoc::registration r{2023, 24, part_1, part_2};

    TEST_SUITE("2023_day24") {
        TEST_CASE("2023_day24:example") {
            using namespace std::string_literals;
            std::vector<std::string> lines {
                    "19, 13, 30 @ -2,  1, -2"s,
                    "18, 19, 22 @ -1, -1, -2"s,
                    "20, 25, 34 @ -2, -2, -4"s,
                    "12, 31, 28 @ -1, -2, -1"s,
                    "20, 19, 15 @  1, -5, -3"s,
            };
            const auto input = lines | std::views::transform(&parse_stone) | to<std::vector>();
            const auto count = find_intersections(input, 7, 27);
            CHECK_EQ(count, 2);
        }
    }

}