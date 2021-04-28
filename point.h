//
// Created by Daniel Garcia on 12/17/20.
//

#ifndef ADVENTOFCODE_POINT_H
#define ADVENTOFCODE_POINT_H

#include <tuple>
#include <iterator>

namespace aoc {

    struct velocity {
        int dx = 0;
        int dy = 0;
    };

    inline bool operator==(velocity a, velocity b) noexcept {
        return a.dx == b.dx && a.dy == b.dy;
    }

    inline bool operator!=(velocity a, velocity b) noexcept {
        return a.dx != b.dx || a.dy != b.dy;
    }

    struct position {
        int x = 0;
        int y = 0;

        position &operator+=(velocity v) noexcept {
            x += v.dx;
            y += v.dy;
            return *this;
        }

        position &operator-=(velocity v) noexcept {
            x -= v.dx;
            y -= v.dy;
            return *this;
        }
    };

    inline position operator+(position a, position b) noexcept {
        return {a.x + b.x, a.y + b.y};
    }

    inline position operator-(position a, position b) noexcept {
        return {a.x - b.x, a.y - b.y};
    }

    inline position operator+(position p, velocity v) noexcept {
        return p += v;
    }

    inline position operator-(position p, velocity v) noexcept {
        return p -= v;
    }

    inline bool operator==(position a, position b) noexcept {
        return a.x == b.x && a.y == b.y;
    }

    inline bool operator!=(position a, position b) noexcept {
        return a.x != b.x || a.y != b.y;
    }

    inline bool operator<(position a, position b) noexcept {
        return std::tie(a.x, a.y) < std::tie(b.x, b.y);
    }

    inline position top_left() noexcept {
        return {0, 0};
    }

    constexpr velocity STANDARD_DIRECTIONS[] = {{1,  -1},
                                                {1,  0},
                                                {1,  1},
                                                {0,  -1},
                                                {0,  1},
                                                {-1, -1},
                                                {-1, 0},
                                                {-1, 1}};

    static_assert(std::size(STANDARD_DIRECTIONS) == 8, "Invalid number of 2D neighbors.");

    struct velocity3d : public velocity {
        int dz = 0;
    };

    inline bool operator==(velocity3d a, velocity3d b) noexcept {
        return a.dx == b.dx && a.dy == b.dy && a.dz == b.dz;
    }

    inline bool operator!=(velocity3d a, velocity3d b) noexcept {
        return a.dx != b.dx || a.dy != b.dy || a.dz != b.dz;
    }

    struct position3d : public position {
        int z = 0;

        position3d() = default;
        position3d(int x, int y, int z) : position{x, y}, z{z} {}
        position3d(position p) : position{p}, z{0} {}

        position3d &operator+=(velocity3d v) noexcept {
            x += v.dx;
            y += v.dy;
            z += v.dz;
            return *this;
        }

        position3d &operator-=(velocity3d v) noexcept {
            x -= v.dx;
            y -= v.dy;
            z -= v.dz;
            return *this;
        }
    };

    inline position3d operator+(position3d a, position3d b) noexcept {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    inline position3d operator-(position3d a, position3d b) noexcept {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    inline position3d operator+(position3d p, velocity3d v) noexcept {
        return p += v;
    }

    inline position3d operator-(position3d p, velocity3d v) noexcept {
        return p -= v;
    }

    inline bool operator==(position3d a, position3d b) noexcept {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    inline bool operator!=(position3d a, position3d b) noexcept {
        return a.x != b.x || a.y != b.y || a.z != b.z;
    }

    constexpr velocity3d STANDARD_3D_DIRECTIONS[] = {{1,  -1, 1},
                                                     {1,  0, 1},
                                                     {1,  1, 1},
                                                     {0,  -1, 1},
                                                     {0,  0, 1},
                                                     {0,  1, 1},
                                                     {-1, -1, 1},
                                                     {-1, 0, 1},
                                                     {-1, 1, 1},
                                                     {1,  -1, 0},
                                                     {1,  0, 0},
                                                     {1,  1, 0},
                                                     {0,  -1, 0},
                                                     {0,  1, 0},
                                                     {-1, -1, 0},
                                                     {-1, 0, 0},
                                                     {-1, 1, 0},
                                                     {1,  -1, -1},
                                                     {1,  0, -1},
                                                     {1,  1, -1},
                                                     {0,  -1, -1},
                                                     {0,  0, -1},
                                                     {0,  1, -1},
                                                     {-1, -1, -1},
                                                     {-1, 0, -1},
                                                     {-1, 1, -1}};

    static_assert(std::size(STANDARD_3D_DIRECTIONS) == 26, "Invalid number of 3D neighbors.");

} /* namespace aoc */

#endif //ADVENTOFCODE_POINT_H
