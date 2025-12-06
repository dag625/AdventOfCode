//
// Created by Dan on 12/6/2025.
//

#include <numeric>

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/6
     */

    struct p1_numbers {
        std::vector<std::vector<int64_t>> numbers;
        std::vector<char> ops;
    };

    std::vector<int64_t> parse_numbers_p1(std::string_view str) {
        const auto parts = split_no_empty(str, ' ');
        return parts | std::views::transform(&parse64) | std::ranges::to<std::vector>();
    }

    std::vector<char> parse_ops(std::string_view str) {
        const auto parts = split_no_empty(str, ' ');
        return parts | std::views::transform([](std::string_view s){ return s.front(); }) | std::ranges::to<std::vector>();
    }

    p1_numbers get_input_p1(const std::vector<std::string>& lines) {
        return {
            lines | std::views::take(lines.size() - 1) | std::views::transform(&parse_numbers_p1) | std::ranges::to<std::vector>(),
            parse_ops(lines.back())
        };
    }

    struct p2_op {
        std::vector<int64_t> numbers;
        char op = '\0';

        [[nodiscard]] int64_t do_op() const {
            if (op == '+') {
                return std::accumulate(numbers.begin(), numbers.end(), 0ll);
            }
            else {
                return std::accumulate(numbers.begin(), numbers.end(), 1ll,
                    [](int64_t tot, int64_t n){ return tot *= n; });
            }
        }
    };

    using p2_numbers = std::vector<p2_op>;

    p2_numbers get_input_p2(const std::vector<std::string>& lines) {
        std::vector<std::string> cols;
        for (int i = 0; i < lines.front().size(); ++i) {
            std::string s;
            for (const auto& l : lines) {
                s.push_back(l[i]);
            }
            s = trim(s);
            cols.push_back(std::move(s));
        }
        p2_numbers retval;
        for (const auto& grp : cols | std::views::lazy_split("")) {
            p2_op op;
            op.op = grp.front().back();
            for (const auto& s : grp) {
                op.numbers.push_back(parse64(s));
            }
            retval.push_back(std::move(op));
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input_p1(lines);
        int64_t grand_total = 0;
        for (int i = 0; i < input.ops.size(); ++i) {
            int64_t total = 0;
            if (input.ops.at(i) == '+') {
                for (const auto& vec : input.numbers) {
                    total += vec[i];
                }
            }
            else {
                total = 1;
                for (const auto& vec : input.numbers) {
                    total *= vec[i];
                }
            }
            grand_total += total;
        }
        return std::to_string(grand_total);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input_p2(lines);
        int64_t grand_total = 0;
        for (const auto& op : input) {
            const auto total = op.do_op();
            grand_total += total;
        }
        return std::to_string(grand_total);
    }

    aoc::registration r {2025, 6, part_1, part_2};

//    TEST_SUITE("2025_day06") {
//        TEST_CASE("2025_day06:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */