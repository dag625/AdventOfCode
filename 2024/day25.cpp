//
// Created by Dan on 12/24/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/25
     */

    std::vector<grid<char>> get_input(const std::vector<std::string>& lines) {
        std::vector<grid<char>> retval;
        auto start = lines.begin();
        auto end = std::find(lines.begin(), lines.end(), "");
        do {
            retval.push_back(to_grid(start, end));
            start = end + 1;
            end = std::find(start, lines.end(), "");
        } while (end != lines.end());
        retval.push_back(to_grid(start, end));
        return retval;
    }

    using info = std::array<int, 5>;

    struct keys_and_locks {
        std::vector<info> keys;
        std::vector<info> locks;
    };

    keys_and_locks get_keys_and_locks(const std::vector<grid<char>>& grids) {
        keys_and_locks retval;
        for (const auto& g : grids) {
            info i{};
            for (int c = 0; c < g.num_cols(); ++c) {
                const auto col = g.column(c);
                i[c] = static_cast<int>(std::count(col.begin(), col.end(), '#')) - 1;//-1 for top or bottom row always being filled
            }
            if (g[{0, 0}] == '#') {
                //lock
                retval.locks.push_back(i);
            }
            else {
                //key
                retval.keys.push_back(i);
            }
        }
        return retval;
    }

    constexpr int MAX_HEIGHT = 5;

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto [keys, locks] = get_keys_and_locks(input);
        int num_fit = 0;
        for (const auto& k : keys) {
            for (const auto& l : locks) {
                int num_good = 0;
                for (int i = 0; i < k.size(); ++i) {
                    if (k[i] + l[i] <= MAX_HEIGHT) {
                        ++num_good;
                    }
                }
                if (num_good == k.size()) {
                    ++num_fit;
                }
            }
        }
        return std::to_string(num_fit);
    }

    aoc::registration r{2024, 25, part_1};

//    TEST_SUITE("2024_day25") {
//        TEST_CASE("2024_day25:example") {
//
//        }
//    }

} /* namespace <anon> */