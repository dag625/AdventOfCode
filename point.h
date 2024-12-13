//
// Created by Daniel Garcia on 12/17/20.
//

#ifndef ADVENTOFCODE_POINT_H
#define ADVENTOFCODE_POINT_H

#include <tuple>
#include <iterator>
#include <compare>

namespace aoc {

    struct velocity {
        int dx = 0;
        int dy = 0;

        [[nodiscard]] bool operator==(const velocity& rhs) const = default;
        [[nodiscard]] auto operator<=>(const velocity& rhs) const = default;

        velocity& operator+=(velocity v) noexcept {
            dx += v.dx;
            dy += v.dy;
            return *this;
        }

        velocity& operator-=(velocity v) noexcept {
            dx -= v.dx;
            dy -= v.dy;
            return *this;
        }

        velocity& operator*=(int v) noexcept {
            dx += v;
            dy += v;
            return *this;
        }

        velocity& operator/=(int v) noexcept {
            dx -= v;
            dy -= v;
            return *this;
        }
    };

    struct position {
        int x = 0;
        int y = 0;

        [[nodiscard]] bool operator==(const position& rhs) const = default;
        [[nodiscard]] auto operator<=>(const position& rhs) const = default;

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

    inline velocity to_vel(position p) noexcept {
        return {p.x, p.y};
    }

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

    inline position operator*(position p, int v) noexcept {
        p.x *= v;
        p.y *= v;
        return p;
    }

    inline position operator/(position p, int v) noexcept {
        p.x /= v;
        p.y /= v;
        return p;
    }

    inline velocity operator*(velocity p, int v) noexcept {
        p.dx *= v;
        p.dy *= v;
        return p;
    }

    inline velocity operator/(velocity p, int v) noexcept {
        p.dx /= v;
        p.dy /= v;
        return p;
    }

    inline velocity operator-(velocity v) noexcept {
        return {-v.dx, -v.dy};
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

    constexpr velocity CARDINAL_DIRECTIONS[] = {{1,  0},
                                                {0,  -1},
                                                {0,  1},
                                                {-1, 0}};

    struct velocity3d : public velocity {
        int dz = 0;

        [[nodiscard]] bool operator==(const velocity3d& rhs) const = default;
        [[nodiscard]] auto operator<=>(const velocity3d& rhs) const = default;

        velocity3d& operator+=(velocity3d v) noexcept {
            dx += v.dx;
            dy += v.dy;
            dz += v.dz;
            return *this;
        }

        velocity3d& operator-=(velocity3d v) noexcept {
            dx -= v.dx;
            dy -= v.dy;
            dz -= v.dz;
            return *this;
        }
    };

    struct position3d : public position {
        int z = 0;

        position3d() = default;
        position3d(int x, int y, int z) : position{x, y}, z{z} {}
        position3d(position p) : position{p}, z{0} {}

        [[nodiscard]] position project_xy() const { return {x, y}; }

        [[nodiscard]] bool operator==(const position3d& rhs) const = default;
        [[nodiscard]] auto operator<=>(const position3d& rhs) const = default;

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

    constexpr velocity3d HEX_CUBE_DIRECTIONS[] = {{1, 0, -1},
                                                  {1, -1, 0},
                                                  {0, -1, 1},
                                                  {-1, 0, 1},
                                                  {-1, 1, 0},
                                                  {0, 1, -1}};

    inline int hex_dist(const position3d a, const position3d b) noexcept {
        const auto dist = a - b;
        return (std::abs(dist.x) + std::abs(dist.y) + std::abs(dist.z)) / 2;
    }

    inline int hex_dist_from_origin(const position3d a) noexcept {
        return (std::abs(a.x) + std::abs(a.y) + std::abs(a.z)) / 2;
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_POINT_H
