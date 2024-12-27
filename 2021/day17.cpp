//
// Created by Dan on 12/17/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <cmath>

#include "point.h"

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    //Input:  target area: x=185..221, y=-122..-74
    constexpr position TARGET_TOP_LEFT {185, -74};
    constexpr position TARGET_BOTTOM_RIGHT {221, -122};
    /*
     * There are target areas where this doesn't apply, e.g.:  x=352..377, y=-49..-30
     * The issue is the x-range doesn't contain any triangular numbers, so the v_x can't
     * be equal to 0 inside this range, which is an assumption in coming to this solution.
     * Because of this the intercept with the highest peak must have some horizontal
     * velocity which constrains the starting vertical velocities.
     *
     * It is also true that if the x range contains the triangular numbers starting with
     * T_n and n is larger than the bottom of the intercept range this will not work as
     * the trajectory will always fall below the intercept range before hitting it.
     */
    constexpr int MAX_DY_0 = -(TARGET_BOTTOM_RIGHT.y + 1);

    bool is_in(const position p, const position tl, const position br) noexcept {
        return p.x >= tl.x && p.x <= br.x &&
            p.y <= tl.y && p.y >= br.y;
    }

    int does_velocity_hit(const velocity v_0, const position tl, const position br) {
        velocity v = v_0;
        position p = {0,0};
        while (p.y >= br.y) {
            if (is_in(p, tl, br)) {
                return true;
            }
            p += v;
            v.dx = std::max(v.dx - 1, 0);
            v.dy -= 1;
        }
        return false;
    }

    std::pair<int, int> get_x_vel_range() {
        const auto min = static_cast<int>(std::ceil(-0.5 + std::sqrt(0.25 + 2.0 * TARGET_TOP_LEFT.x)));
        const auto max = TARGET_BOTTOM_RIGHT.x;
        return {min, max};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        return std::to_string((MAX_DY_0 * (MAX_DY_0 + 1)) / 2);
    }

    /************************* Part 1 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        int count = 0;
        const auto x_vels = get_x_vel_range();
        for (int dx = x_vels.first; dx <= x_vels.second; ++dx) {
            for (int dy = TARGET_BOTTOM_RIGHT.y; dy <= MAX_DY_0; ++dy) {
                if (does_velocity_hit({dx, dy}, TARGET_TOP_LEFT, TARGET_BOTTOM_RIGHT)) {
                    ++count;
                }
            }
        }
        return std::to_string(count);
    }

    aoc::registration r {2021, 17, part_1, part_2};

    TEST_SUITE("2021_day17") {
        TEST_CASE("2021_day17:example") {
            constexpr position EX_TARGET_TOP_LEFT {20, -5};
            constexpr position EX_TARGET_BOTTOM_RIGHT {30, -10};
            constexpr int EX_MAX_DY_0 = -(EX_TARGET_BOTTOM_RIGHT.y + 1);

            const auto x_min = static_cast<int>(std::ceil(-0.5 + std::sqrt(0.25 + 2.0 * EX_TARGET_TOP_LEFT.x)));
            const auto x_max = EX_TARGET_BOTTOM_RIGHT.x;

            int count = 0;
            for (int dx = x_min; dx <= x_max; ++dx) {
                for (int dy = EX_TARGET_BOTTOM_RIGHT.y; dy <= EX_MAX_DY_0; ++dy) {
                    if (does_velocity_hit({dx, dy}, EX_TARGET_TOP_LEFT, EX_TARGET_BOTTOM_RIGHT)) {
                        ++count;
                    }
                }
            }
            REQUIRE_EQ(count, 112);
        }
    }

}