//
// Created by Dan on 12/10/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <compare>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/10
     */

    grid<char> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_10_input.txt");
        return to_grid(lines);
    }

    std::vector<position> find_trailheads(const grid<char>& g) {
        std::vector<position> retval;
        for (const auto p : g.list_positions()) {
            if (g[p] == '0') {
                retval.push_back(p);
            }
        }
        return retval;
    }

    std::vector<position> find_reachable_peaks(const position head, const grid<char>& g) {
        std::vector<position> retval, to_visit;
        to_visit.push_back(head);
        while (!to_visit.empty()) {
            const auto current = to_visit.front();
            to_visit.erase(to_visit.begin());

            const auto neighbors = g.cardinal_neighbors(current);
            for (const auto n : neighbors) {
                if (g[n] == g[current] + 1) {
                    if (g[n] == '9') {
                        const auto found = std::lower_bound(retval.begin(), retval.end(), n);
                        if (found == retval.end() || *found != n) {
                            retval.insert(found, n);
                        }
                    }
                    else {
                        const auto found = std::lower_bound(to_visit.begin(), to_visit.end(), n);
                        if (found == to_visit.end() || *found != n) {
                            to_visit.insert(found, n);
                        }
                    }
                }
            }
        }
        return retval;
    }

    struct path {
        std::vector<position> visited;

        path() = default;
        path(const position p) : visited{} { visited.push_back(p); }

        [[nodiscard]] path add(const position p) const { auto retval = *this; retval.visited.push_back(p); return retval; }

        auto operator<=>(const path& rhs) const { return std::lexicographical_compare_three_way(visited.begin(), visited.end(), rhs.visited.begin(), rhs.visited.end()); }
        bool operator==(const path& rhs) const { return std::equal(visited.begin(), visited.end(), rhs.visited.begin(), rhs.visited.end()); }
    };

    std::vector<path> find_paths(const position head, const grid<char>& g) {
        std::vector<path> retval, to_visit;
        to_visit.emplace_back(head);
        while (!to_visit.empty()) {
            const auto current = to_visit.front();
            to_visit.erase(to_visit.begin());

            const auto neighbors = g.cardinal_neighbors(current.visited.back());
            for (const auto n : neighbors) {
                if (g[n] == g[current.visited.back()] + 1) {
                    auto next = current.add(n);
                    if (g[n] == '9') {
                        const auto found = std::lower_bound(retval.begin(), retval.end(), next);
                        if (found == retval.end() || *found != next) {
                            retval.insert(found, std::move(next));
                        }
                    }
                    else {
                        const auto found = std::lower_bound(to_visit.begin(), to_visit.end(), next);
                        if (found == to_visit.end() || *found != next) {
                            to_visit.insert(found, std::move(next));
                        }
                    }
                }
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto trailheads = find_trailheads(input);
        int num = 0;
        for (const auto& th : trailheads) {
            const auto peaks = find_reachable_peaks(th, input);
            num += static_cast<int>(peaks.size());
        }
        return std::to_string(num);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto trailheads = find_trailheads(input);
        int num = 0;
        for (const auto& th : trailheads) {
            const auto paths = find_paths(th, input);
            num += static_cast<int>(paths.size());
        }
        return std::to_string(num);
    }

    aoc::registration r{2024, 10, part_1, part_2};

    TEST_SUITE("2024_day10") {
        TEST_CASE("2024_day10:example") {
            const std::vector<std::string> lines1 {
                    "...0...",
                    "...1...",
                    "...2...",
                    "6543456",
                    "7.....7",
                    "8.....8",
                    "9.....9"
            };
            const auto input1 = to_grid(lines1);
            const auto trailheads1 = find_trailheads(input1);
            int num1 = 0;
            for (const auto& th : trailheads1) {
                const auto peaks = find_reachable_peaks(th, input1);
                num1 += static_cast<int>(peaks.size());
            }
            CHECK_EQ(num1, 2);


            const std::vector<std::string> lines2 {
                    "..90..9",
                    "...1.98",
                    "...2..7",
                    "6543456",
                    "765.987",
                    "876....",
                    "987...."
            };
            const auto input2 = to_grid(lines2);
            const auto trailheads2 = find_trailheads(input2);
            int num2 = 0;
            for (const auto& th : trailheads2) {
                const auto peaks = find_reachable_peaks(th, input2);
                num2 += static_cast<int>(peaks.size());
            }
            CHECK_EQ(num2, 4);


            const std::vector<std::string> lines3 {
                    "10..9..",
                    "2...8..",
                    "3...7..",
                    "4567654",
                    "...8..3",
                    "...9..2",
                    ".....01"
            };
            const auto input3 = to_grid(lines3);
            const auto trailheads3 = find_trailheads(input3);
            int num3 = 0;
            for (const auto& th : trailheads3) {
                const auto peaks = find_reachable_peaks(th, input3);
                num3 += static_cast<int>(peaks.size());
            }
            CHECK_EQ(num3, 3);
        }
    }

} /* namespace <anon> */