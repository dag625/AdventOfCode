//
// Created by Dan on 12/10/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "ranges.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/14
     */

    std::string get_input(const std::vector<std::string>& lines) {
        return lines.front();
    }

    //Copied from 2017 day 14:

    void do_reverse(std::vector<int>& list, const int pos, const int len) {
        const int size = static_cast<int>(list.size());
        const int end = (pos + len) % size;
        if (end >= pos) {
            std::reverse(list.begin() + pos, list.begin() + end);
        }
        else {
            std::vector<int> buf;
            buf.reserve(len);
            buf.insert(buf.end(), list.begin() + pos, list.end());
            buf.insert(buf.end(), list.begin(), list.begin() + end);
            std::reverse(buf.begin(), buf.end());
            int lp = pos;
            for (const int v : buf) {
                list[lp] = v;
                lp = (lp + 1) % size;
            }
        }
    }

    std::vector<uint8_t> knot_hash(std::string_view s) {
        auto input = s | std::views::transform([](const char c){ return static_cast<int>(c); }) | std::ranges::to<std::vector>();
        input.push_back(17);
        input.push_back(31);
        input.push_back(73);
        input.push_back(47);
        input.push_back(23);
        auto list = std::views::iota(0, 256) | std::ranges::to<std::vector>();
        const auto size = static_cast<int>(list.size());
        int pos = 0, skip = 0;
        for (int i = 0; i < 64; ++i) {
            for (const auto len : input) {
                do_reverse(list, pos, len);
                pos = (pos + len + skip) % size;
                ++skip;
            }
        }

        std::vector<uint8_t> retval;
        for (const auto grp : list | std::views::chunk(16)) {
            uint8_t v = 0;
            for (const auto i : grp) {
                v = v ^ static_cast<uint8_t>(i);
            }
            retval.push_back(v);
        }
        return retval;
    }

    std::string to_hex(const std::vector<uint8_t>& bin) {
        std::string hex;
        for (const auto v : bin) {
            hex = fmt::format("{}{:02x}", hex, v);
        }
        return hex;
    }

    bool is_bit_set(const uint8_t v, const int bit) {
        return ((v >> (7 - bit)) & 0x01u) == 0x01u;
    }

    char grid_value(const uint8_t v, const int bit) {
        return is_bit_set(v, bit) ? '#' : '.';
    }

    int set_adjancencies(grid<int>& g) {
        const auto positions = g.list_positions();
        int next = 0;
        for (const auto pos : positions) {
            if (g[pos] == 0) {
                ++next;

                std::vector<position> checks;
                g[pos] = next;
                checks.push_back(pos);
                while (!checks.empty()) {
                    const auto p = checks.front();
                    checks.erase(checks.begin());

                    const auto neighbors = g.cardinal_neighbors(p);
                    for (const auto n : neighbors) {
                        if (g[n] == 0) {
                            g[n] = next;
                            checks.push_back(n);
                        }
                    }
                }
            }
        }
        return next;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        grid<char> g {128u, 128u};
        int used = 0;
        for (int r = 0; r < g.num_rows(); ++r) {
            const auto hash = knot_hash(fmt::format("{}-{}", input, r));
            for (int c = 0; c < g.num_cols(); ++c) {
                const auto val = grid_value(hash[c / 8], c % 8);
                g[r][c] = val;
                if (val == '#') {
                    ++used;
                }
            }
        }
        return std::to_string(used);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        grid<int> g {128u, 128u};
        for (int r = 0; r < g.num_rows(); ++r) {
            const auto hash = knot_hash(fmt::format("{}-{}", input, r));
            for (int c = 0; c < g.num_cols(); ++c) {
                const auto val = is_bit_set(hash[c / 8], c % 8);
                g[r][c] = val ? 0 : -1;
            }
        }

        const auto num_groups = set_adjancencies(g);
        return std::to_string(num_groups);
    }

    aoc::registration r{2017, 14, part_1, part_2};

    TEST_SUITE("2017_day14") {
        TEST_CASE("2017_day14:example") {
            const auto hash1 = to_hex(knot_hash(""));
            const auto hash2 = to_hex(knot_hash("AoC 2017"));
            const auto hash3 = to_hex(knot_hash("1,2,3"));
            const auto hash4 = to_hex(knot_hash("1,2,4"));
            CHECK_EQ(hash1, "a2582a3a0e66e6e86e3812dcb672a272");
            CHECK_EQ(hash2, "33efeb34ea91902bb2f59c9920caa6cd");
            CHECK_EQ(hash3, "3efbe78a8d82f29979031a4aa0b16a9d");
            CHECK_EQ(hash4, "63960835bcdc130f0b66d7ff4f6a5a8e");

            const std::string input = "flqrgnkx";
            grid<char> g {128u, 128u};
            int used = 0;
            for (int r = 0; r < g.num_rows(); ++r) {
                const auto hash = knot_hash(fmt::format("{}-{}", input, r));
                for (int c = 0; c < g.num_cols(); ++c) {
                    const auto val = grid_value(hash[c / 8], c % 8);
                    g[r][c] = val;
                    if (val == '#') {
                        ++used;
                    }
                }
            }
            g.display(std::cout);
            CHECK_EQ(used, 8108);
        }
    }

} /* namespace <anon> */