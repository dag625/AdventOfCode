//
// Created by Dan on 12/8/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "point.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2025/day/8
     */

    position3d parse_position(std::string_view str) {
        const auto parts = split(str, ',');
        return {parse32(parts[0]), parse32(parts[1]), parse32(parts[2])};
    }

    std::vector<position3d> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_position) | std::ranges::to<std::vector>();
    }

    struct pos_dist {
        int idx1 = 0;
        int idx2 = 0;
        double dist = 0;

        pos_dist() = default;
        pos_dist(int i1, int i2, double d) : idx1{i1}, idx2{i2}, dist{d} {}
        pos_dist(const position3d& a, int ai, const position3d& b, int bi) : idx1 {ai}, idx2{bi} {
            const auto ax = static_cast<double>(a.x);
            const auto ay = static_cast<double>(a.y);
            const auto az = static_cast<double>(a.z);
            const auto bx = static_cast<double>(b.x);
            const auto by = static_cast<double>(b.y);
            const auto bz = static_cast<double>(b.z);
            const auto dx = ax - bx;
            const auto dy = ay - by;
            const auto dz = az - bz;
            const auto sq = dx * dx + dy * dy + dz * dz;
            dist = std::sqrt(sq);
        }

        [[nodiscard]] auto operator<=>(const pos_dist& rhs) const noexcept { return dist <=> rhs.dist; }
    };

    using circuit = std::vector<int>;

    void add_to_circuits(std::vector<circuit>& circuits, const pos_dist& dist) {
        if (circuits.empty()) {
            circuit c;
            c.push_back(dist.idx1);
            c.push_back(dist.idx2);
            circuits.push_back(std::move(c));
        }
        else {
            const auto finder1 = [&dist](const circuit& c) {
                return std::find(c.begin(), c.end(), dist.idx1) != c.end();
            };
            const auto finder2 = [&dist](const circuit& c) {
                return std::find(c.begin(), c.end(), dist.idx2) != c.end();
            };
            const auto found1 = std::find_if(circuits.begin(), circuits.end(), finder1);
            const auto found2 = std::find_if(circuits.begin(), circuits.end(), finder2);
            if (found1 == circuits.end() && found2 == circuits.end()) {
                circuit c;
                c.push_back(dist.idx1);
                c.push_back(dist.idx2);
                circuits.push_back(std::move(c));
            }
            else if (found1 != circuits.end() && found2 != circuits.end() && found1 != found2) {
                found1->insert(found1->end(), found2->begin(), found2->end());
                circuits.erase(found2);
            }
            else if (found1 == circuits.end() && found2 != circuits.end()) {
                found2->push_back(dist.idx1);
            }
            else if (found1 != circuits.end() && found2 == circuits.end()) {
                found1->push_back(dist.idx2);
            }
            //else both found but in same circuit already, then do nothing.
        }
    }

    bool all_in_one(const std::vector<circuit>& circuits, const int num_boxes) {
        return circuits.size() == 1 && circuits.front().size() == num_boxes;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<pos_dist> dists;
        dists.reserve(input.size() * input.size());
        for (int i = 0; i < input.size(); ++i) {
            for (int j = i + 1; j < input.size(); ++j) {
                dists.emplace_back(input[i], i, input[j], j);
            }
        }
        std::sort(dists.begin(), dists.end());

        constexpr int NUM_CONNECTIONS = 1000;
        std::vector<circuit> circuits;
        for (int i = 0; i < NUM_CONNECTIONS; ++i) {
            add_to_circuits(circuits, dists[i]);
        }

        std::sort(circuits.begin(), circuits.end(), [](const auto& c1, const auto& c2){ return c1.size() > c2.size(); });
        const auto res = std::accumulate(circuits.begin(), circuits.begin() + 3, 1ull,
            [](std::size_t tot, const auto& c){ return tot *= c.size(); });
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<pos_dist> dists;
        dists.reserve(input.size() * input.size());
        for (int i = 0; i < input.size(); ++i) {
            for (int j = i + 1; j < input.size(); ++j) {
                dists.emplace_back(input[i], i, input[j], j);
            }
        }
        std::sort(dists.begin(), dists.end());

        std::vector<circuit> circuits;
        int idx = 0;
        while (!all_in_one(circuits, static_cast<int>(input.size()))) {
            add_to_circuits(circuits, dists[idx]);
            ++idx;
        }
        const auto& last = dists[idx - 1];
        const auto& d1 = input[last.idx1];
        const auto& d2 = input[last.idx2];
        return std::to_string(static_cast<int64_t>(d1.x) * d2.x);
    }

    aoc::registration r{2025, 8, part_1, part_2};

    //    TEST_SUITE("2025_day08") {
    //        TEST_CASE("2025_day08:example") {
    //            const std::vector<std::string> lines {
    //            
    //            };
    //            const auto input = get_input(lines);
    //
    //        }
    //    }

} /* namespace <anon> */