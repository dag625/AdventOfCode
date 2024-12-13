//
// Created by Dan on 12/12/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "point.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/20
     */

    struct particle {
        position3d pos;
        velocity3d vel;
        velocity3d acc;

        auto acceleration() const {
            const auto x = std::abs(acc.dx);
            const auto y = std::abs(acc.dy);
            const auto z = std::abs(acc.dz);
            return std::sqrt(x*x + y*y + z*z);
        }
    };

    particle parse_part(std::string_view s) {
        const auto parts = split_by_all_no_empty(s, "pva=<>, ");
        return {{parse<int>(parts[0]), parse<int>(parts[1]), parse<int>(parts[2])},
                {parse<int>(parts[3]), parse<int>(parts[4]), parse<int>(parts[5])},
                {parse<int>(parts[6]), parse<int>(parts[7]), parse<int>(parts[8])}};
    }

    std::vector<particle> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_20_input.txt");
        return lines | std::views::transform(&parse_part) | std::ranges::to<std::vector>();
    }

    std::optional<std::pair<double, double>> solve(const int x0, const int x1, const int v0, const int v1, const int a0, const int a1) {
        const auto c = x0 - x1;
        const auto b = v0 - v1;
        const auto a = a0 - a1;
        const auto bot = 2 * a;
        if (bot == 0) {
            if (b == 0) {
                return std::pair{0.0, 0.0}; //Stationary
            }
            else {
                return std::pair{static_cast<double>(c) / b, static_cast<double>(c) / b}; //Non-quadratic
            }
        }
        const auto top_sqrt_body = b * b - 4 * a * c;
        if (top_sqrt_body < 0) {
            return std::nullopt; //Only complex solutions
        }
        const auto top_sqrt = std::sqrt(top_sqrt_body);
        return std::pair{(top_sqrt - b) / bot, (-top_sqrt - b) / bot};
    }

    void solve(const particle& a, const particle& b) {
        const auto xts = solve(a.pos.x, b.pos.x, a.vel.dx, b.vel.dx, a.acc.dx, b.acc.dx);
        const auto yts = solve(a.pos.y, b.pos.y, a.vel.dy, b.vel.dy, a.acc.dy, b.acc.dy);
        const auto zts = solve(a.pos.z, b.pos.z, a.vel.dz, b.vel.dz, a.acc.dz, b.acc.dz);
        if (xts && yts && zts) {
            //fmt::println("Yay");
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto accels = input | std::views::transform([](const particle& p){ return p.acceleration(); }) | std::ranges::to<std::vector>();
        const auto slowest = std::min_element(accels.begin(), accels.end());
        return std::to_string(std::distance(accels.begin(), slowest));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);

        particle a {{-6,0,0}, {3,0,0}, {1,0,0}};
        particle b {{-4,0,0}, {2,0,0}, {1,0,0}};

        solve(a, b);
        return std::to_string(-1);
    }

    aoc::registration r{2017, 20, part_1, part_2};

//    TEST_SUITE("2017_day20") {
//        TEST_CASE("2017_day20:example") {
//
//        }
//    }

} /* namespace <anon> */