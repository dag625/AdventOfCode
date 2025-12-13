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
#include "matrix.h"
#include "int128.h"

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

    struct vec3d {
        //The numbers are big enough and we are doing multiplications, so we need this.
        int128 x = 0;
        int128 y = 0;
        int128 z = 0;

        vec3d() = default;
        vec3d(int64_t x, int64_t y, int64_t z) : x(x), y(y), z(z) {}
        vec3d(int128 x, int128 y, int128 z) : x(x), y(y), z(z) {}
        vec3d(const position64& p) : x(p.x), y(p.y), z(p.z) {}
        vec3d(const velocity64& v) : x(v.dx), y(v.dy), z(v.dz) {}

        bool operator==(const vec3d& rhs) const noexcept = default;

        [[nodiscard]] int128 dot(const vec3d& rhs) const noexcept { return x * rhs.x + y * rhs.y + z * rhs.z; }
        [[nodiscard]] vec3d cross(const vec3d& rhs) const noexcept {
            vec3d retval;
            retval.x = y * rhs.z - z * rhs.y;
            retval.y = z * rhs.x - x * rhs.z;
            retval.z = x * rhs.y - y * rhs.x;
            return retval;
        }

        [[nodiscard]] vec3d operator+(const vec3d& rhs) const noexcept { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
        [[nodiscard]] vec3d operator-(const vec3d& rhs) const noexcept { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
        vec3d& operator+=(const vec3d& rhs) noexcept { x += rhs.x, y += rhs.y, z += rhs.z; return *this; }
        vec3d& operator-=(const vec3d& rhs) noexcept { x -= rhs.x, y -= rhs.y, z -= rhs.z; return *this; }

        [[nodiscard]] vec3d operator+(const int64_t rhs) const noexcept { return {x + rhs, y + rhs, z + rhs}; }
        [[nodiscard]] vec3d operator-(const int64_t rhs) const noexcept { return {x - rhs, y - rhs, z - rhs}; }
        [[nodiscard]] vec3d operator*(const int64_t rhs) const noexcept { return {x * rhs, y * rhs, z * rhs}; }
        [[nodiscard]] vec3d operator/(const int64_t rhs) const noexcept { return {x / rhs, y / rhs, z / rhs}; }
        [[nodiscard]] vec3d operator%(const int64_t rhs) const noexcept { return {x % rhs, y % rhs, z % rhs}; }
        vec3d& operator+=(const int64_t rhs) noexcept { x += rhs, y += rhs, z += rhs; return *this; }
        vec3d& operator-=(const int64_t rhs) noexcept { x -= rhs, y -= rhs, z -= rhs; return *this; }
        vec3d& operator*=(const int64_t rhs) noexcept { x *= rhs, y *= rhs, z *= rhs; return *this; }
        vec3d& operator/=(const int64_t rhs) noexcept { x /= rhs, y /= rhs, z /= rhs; return *this; }
        vec3d& operator%=(const int64_t rhs) noexcept { x %= rhs, y %= rhs, z %= rhs; return *this; }

        [[nodiscard]] vec3d operator+(const int128 rhs) const noexcept { return {x + rhs, y + rhs, z + rhs}; }
        [[nodiscard]] vec3d operator-(const int128 rhs) const noexcept { return {x - rhs, y - rhs, z - rhs}; }
        [[nodiscard]] vec3d operator*(const int128 rhs) const noexcept { return {x * rhs, y * rhs, z * rhs}; }
        [[nodiscard]] vec3d operator/(const int128 rhs) const noexcept { return {x / rhs, y / rhs, z / rhs}; }
        [[nodiscard]] vec3d operator%(const int128 rhs) const noexcept { return {x % rhs, y % rhs, z % rhs}; }
        vec3d& operator+=(const int128 rhs) noexcept { x += rhs, y += rhs, z += rhs; return *this; }
        vec3d& operator-=(const int128 rhs) noexcept { x -= rhs, y -= rhs, z -= rhs; return *this; }
        vec3d& operator*=(const int128 rhs) noexcept { x *= rhs, y *= rhs, z *= rhs; return *this; }
        vec3d& operator/=(const int128 rhs) noexcept { x /= rhs, y /= rhs, z /= rhs; return *this; }
        vec3d& operator%=(const int128 rhs) noexcept { x %= rhs, y %= rhs, z %= rhs; return *this; }
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

    std::pair<vec3d, vec3d> find_intersection(const std::vector<stone>& stones) {
        //Math from: https://www.reddit.com/r/adventofcode/comments/18pnycy/comment/kxqjg33/
        const auto s0 =  vec3d{stones[0].pos};
        const auto s1 =  vec3d{stones[1].pos};
        const auto s2 =  vec3d{stones[2].pos};
        const auto vs0 =  vec3d{stones[0].vel};
        const auto vs1 =  vec3d{stones[1].vel};
        const auto vs2 =  vec3d{stones[2].vel};

        const auto p1 = s1 - s0;
        const auto p2 = s2 - s0;
        const auto v1 = vs1 - vs0;
        const auto v2 = vs2 - vs0;

        const auto t1_top = -(p1.cross(p2).dot(v2));
        const auto t1_bot = v1.cross(p2).dot(v2);
        const auto t2_top = -(p1.cross(p2).dot(v1));
        const auto t2_bot = p1.cross(v2).dot(v1);
        const auto t1 = t1_top / t1_bot;
        const auto t2 = t2_top / t2_bot;

        const auto d1 = s1 + (vs1 * t1);
        const auto d2 = s2 + (vs2 * t2);
        const auto dp = d2 - d1;
        const auto dt = t2 - t1;
        const auto v = dp / dt;
        const auto p = d1 - v * t1;

        return {p, v};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto count = find_intersections(input, MIN_POS, MAX_POS);
        return std::to_string(count);
    }

    /************************* Part 2 *************************/
    std::string part_2_python(const std::vector<std::string>& lines) {
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
        return std::to_string(0ll/* Redacted */);
    }

    std::string part_2_cpp(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [p, v] = find_intersection(input);
        const auto res = p.x + p.y + p.z;
        return std::to_string(static_cast<int64_t>(res));
    }

    aoc::registration r{2023, 24, part_1, part_2_cpp};

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

            const auto [p, v] = find_intersection(input);
            CHECK_EQ(p.x, 24);
            CHECK_EQ(p.y, 13);
            CHECK_EQ(p.z, 10);
            CHECK_EQ(v.x, -3);
            CHECK_EQ(v.y, 1);
            CHECK_EQ(v.z, 2);
            CHECK_EQ(p.x + p.y + p.z, 47);
        }
    }

}