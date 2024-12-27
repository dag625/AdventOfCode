//
// Created by Dan on 12/11/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <algorithm>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/16
     */

    using dancers = std::string;

    struct spin_mv {
        int num = 0;

        void move(dancers& d) const {
            std::rotate(d.rbegin(), d.rbegin() + num, d.rend());
        }
    };

    struct exchange_mv {
        int pos1 = 0;
        int pos2 = 0;

        void move(dancers& d) const {
            std::swap(d[pos1], d[pos2]);
        }
    };

    struct partner_mv {
        char dancer1 = '\0';
        char dancer2 = '\0';

        void move(dancers& d) const {
            auto pos1 = std::find(d.begin(), d.end(), dancer1);
            auto pos2 = std::find(d.begin(), d.end(), dancer2);
            std::swap(*pos1, *pos2);
        }
    };

    using dance_move = std::variant<spin_mv, exchange_mv, partner_mv>;

    spin_mv parse_spin(std::string_view s) {
        return {parse<int>(s)};
    }

    exchange_mv parse_exchange(std::string_view s) {
        const auto parts = split(s, '/');
        return {parse<int>(parts[0]), parse<int>(parts[1])};
    }

    partner_mv parse_partner(std::string_view s) {
        const auto parts = split(s, '/');
        return {parts[0][0], parts[1][0]};
    }

    dance_move parse_move(std::string_view s) {
        const auto m = s.front();
        s.remove_prefix(1);
        if (m == 's') {
            return parse_spin(s);
        }
        else if (m == 'x') {
            return parse_exchange(s);
        }
        else {
            return parse_partner(s);
        }
    }

    std::vector<dance_move> parse_moves(std::string_view s) {
        const auto move_strs = split(s, ',');
        return move_strs | std::views::transform(&parse_move) | std::ranges::to<std::vector>();
    }

    std::vector<dance_move> get_input(const std::vector<std::string>& lines) {
        return parse_moves(lines.front());
    }

    void do_the_dance(dancers& d, const std::vector<dance_move>& moves) {
        for (const auto& m : moves) {
            std::visit([&d](const auto& mv){ mv.move(d); }, m);
        }
    }

    std::string find_the_loop(dancers& d, const std::vector<dance_move>& moves, const int num_dances) {
        std::vector<dancers> positions;
        positions.push_back(d);
        bool found_latest = false;
        int loop_len = 0, loop_start_idx = -1;
        do {
            do_the_dance(d, moves);
            const auto found = std::find(positions.begin(), positions.end(), d);
            found_latest = found != positions.end();
            loop_start_idx = static_cast<int>(std::distance(positions.begin(), found));
            loop_len = static_cast<int>(positions.size()) - loop_start_idx;
            positions.push_back(d);
        } while(!found_latest);

        const auto mod_idx = num_dances % loop_len;
        auto retval = positions[loop_start_idx + mod_idx];
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto d = std::views::iota('a', 'q') | std::ranges::to<std::string>();
        do_the_dance(d, input);
        return d;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto d = std::views::iota('a', 'q') | std::ranges::to<std::string>();
        const auto res = find_the_loop(d, input, 1'000'000'000);
        return res;
    }

    aoc::registration r{2017, 16, part_1, part_2};

//    TEST_SUITE("2017_day16") {
//        TEST_CASE("2017_day16:example") {
//
//        }
//    }

} /* namespace <anon> */