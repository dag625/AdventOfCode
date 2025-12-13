//
// Created by Dan on 12/12/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <cmath>

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

        [[nodiscard]] auto acceleration() const {
            const auto x = std::abs(acc.dx);
            const auto y = std::abs(acc.dy);
            const auto z = std::abs(acc.dz);
            return std::sqrt(x*x + y*y + z*z);
        }

        void step() {
            vel += acc;
            pos += vel;
        }

        bool operator==(const particle& rhs) const { return pos == rhs.pos; }
    };

    particle parse_part(std::string_view s) {
        const auto parts = split_by_all_no_empty(s, "pva=<>, ");
        return {{parse<int>(parts[0]), parse<int>(parts[1]), parse<int>(parts[2])},
                {parse<int>(parts[3]), parse<int>(parts[4]), parse<int>(parts[5])},
                {parse<int>(parts[6]), parse<int>(parts[7]), parse<int>(parts[8])}};
    }

    std::vector<particle> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_part) | std::ranges::to<std::vector>();
    }

    void simulate(std::vector<particle>& atoms) {
        std::for_each(atoms.begin(), atoms.end(), [](particle& a){ a.step(); });
        std::vector<int> dels;
        for (int i = 0; i < atoms.size(); ++i) {
            for (int j = i + 1; j < atoms.size(); ++j) {
                if (atoms[i] == atoms[j]) {
                    dels.push_back(i);
                    dels.push_back(j);
                }
            }
        }
        std::sort(dels.begin(), dels.end(), std::greater<>{});
        dels.erase(std::unique(dels.begin(), dels.end()), dels.end());
        for (const int idx : dels) {
            atoms.erase(atoms.begin() + idx);
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto accels = input | std::views::transform([](const particle& p){ return p.acceleration(); }) | std::ranges::to<std::vector>();
        const auto slowest = std::min_element(accels.begin(), accels.end());
        return std::to_string(std::distance(accels.begin(), slowest));
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        //1000 is just a stab in the dark.  10000 returned the same result, so this is probably fine.
        for (int i = 0; i < 1000; ++i) {
            simulate(input);
        }
        return std::to_string(input.size());
    }

    aoc::registration r{2017, 20, part_1, part_2};

//    TEST_SUITE("2017_day20") {
//        TEST_CASE("2017_day20:example") {
//
//        }
//    }

} /* namespace <anon> */