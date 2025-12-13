//
// Created by Dan on 12/10/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <bitset>
#include <bit>
#include <iostream>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"
#include "matrix.h"

namespace {
    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/10
     */

    using light_t = decltype(std::declval<std::bitset<16>>().to_ulong());

    struct indicator {
        light_t desired = 0u;
        std::string desired_str;
        std::vector<light_t> toggles;
        std::vector<int> joltages;

        indicator() = default;
        indicator(std::string_view l, std::vector<light_t> t, std::vector<int> j) :
            desired_str{l}, toggles{std::move(t)}, joltages{std::move(j)}
        {
            std::bitset<16> lights;
            for (int i = 0; i < lights.size() && i < l.size(); ++i) {
                if (l[i] == '#') {
                    lights.set(i);
                }
            }
            desired = lights.to_ulong();
        }
    };

    std::string_view next_between(std::string_view s, int& offset, const char begin, const char end) {
        auto retval = s;
        const auto start = retval.find(begin, offset);
        const auto finish = retval.find(end, offset);
        if (start == std::string_view::npos || finish == std::string_view::npos) {
            offset = static_cast<int>(s.size());
            return {};
        }
        retval = retval.substr(start + 1, finish - start - 1);
        offset = static_cast<int>(finish + 1);
        return retval;
    }

    std::string_view next_between(std::string_view s, const char begin, const char end) {
        int offset = 0;
        return next_between(s, offset, begin, end);
    }

    std::vector<int> comma_sep_to_vec(std::string_view s) {
        const auto parts = split(s, ',');
        return parts | std::views::transform(&parse32) | std::ranges::to<std::vector>();
    }

    indicator parse_indicator(std::string_view str) {
        const auto lights_str = next_between(str, '[', ']');
        const auto jolts_str = next_between(str, '{', '}');

        int offset = 0;
        std::vector<light_t> toggles;
        while (offset < str.size()) {
            const auto toggle_str = next_between(str, offset, '(', ')');
            if (offset < str.size()) {
                const auto idxs = comma_sep_to_vec(toggle_str);
                std::bitset<16> lights;
                for (const int i : idxs) {
                    lights.set(i);
                }
                toggles.push_back(lights.to_ulong());
            }
        }

        return {std::string{lights_str}, std::move(toggles), comma_sep_to_vec(jolts_str)};
    }

    std::vector<indicator> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_indicator) | std::ranges::to<std::vector>();
    }

    int find_match_steps(const indicator& ind) {
        static constexpr std::array<light_t, 2> MASKS {0x0u, 0xffffffffu};
        const auto max = 1u << ind.toggles.size();
        int min = std::numeric_limits<int>::max();
        for (unsigned t = 1; t < max; ++t) {
            std::bitset<16> lights {t};
            light_t val = 0u;
            for (int i = 0; i < ind.toggles.size(); ++i) {
                const auto& a = MASKS[(int)lights[i]];
                const auto& b = ind.toggles[i];
                val ^= a & b;
            }
            const auto num = std::popcount(t);
            if (val == ind.desired && num < min) {
                min = num;
            }
        }
        return min;
    }

    bool try_solution(const matrix& A, const vector& b, const vector& x) {
        if (std::any_of(x.data().begin(), x.data().end(), [&](const auto v) { return v < 0.0; })) {
            return false;
        }
        const auto res = A * x;
        return res == b;
    }

    int64_t solution_count(const vector& x) {
        return std::accumulate(x.data().begin(), x.data().end(), 0ll,[](int64_t total, double val){ return total + static_cast<int64_t>(std::round(val)); });
    }

    vector build_solution(const matrix& A_r, const vector& b_r, const std::vector<size_t>& free_var_idxs, const std::vector<double>& free_vars, const double tolerance) {
        vector retval (A_r.num_cols());
        for (int i = 0; i < free_var_idxs.size(); i++) {
            retval[free_var_idxs[i]] = free_vars[i];
        }
        for (size_t c = 0; c < retval.size(); c++) {
            if (std::any_of(free_var_idxs.begin(), free_var_idxs.end(), [c](const size_t i){ return c == i; })) {
                continue;
            }
            const auto col = A_r.col_span(c);
            for (size_t r = 0; r < col.size(); r++) {
                if (col[r] == 1.0) {
                    retval[c] = b_r[r];
                    for (size_t i = 0; i < free_var_idxs.size(); i++) {
                        retval[c] -= free_vars[i] * A_r[r, free_var_idxs[i]];
                    }
                    break;
                }
            }
            retval[c] = std::round(retval[c]);
            if (std::abs(retval[c]) < tolerance) {
                retval[c] = 0.0;
            }
        }
        return retval;
    }

    int64_t find_solution(const matrix& A, const vector& b) {
        const double tolerance = 0.0001;
        auto eq = A.adjoin(b.to_matrix());
        to_reduced_row_echelon(eq);
        eq.set_zeros(tolerance);
        const auto A_r = eq.submatrix(A.num_rows(), A.num_cols());
        const auto b_r = eq.col_vector(eq.num_cols() - 1);

        std::vector<size_t> free_var_idxs;
        for (size_t col = 0; col < A.num_cols(); col++) {
            int num_nonzero = 0;
            for (size_t row = 0; row < eq.num_rows(); row++) {
                if (eq[row, col] != 0.0) {
                    ++num_nonzero;
                }
            }
            if (num_nonzero > 1) {
                free_var_idxs.push_back(col);
            }
        }

        /*
         *Just because a matrix is square or taller and thus fully- or over-determined in theory
         *doesn't mean that there are no free variables.  There can be redundant equations.
         */
        if (free_var_idxs.empty()) {
            //At least a square matrix, should be either fully determined or overdetermined with no free variables.
            const auto sol = b_r.subvector(A.num_cols());//Actually this shouldn't matter given we only want the count and the last entry should be 0 if it's overdetermined.
            const auto count = solution_count(sol);
            return static_cast<int64_t>(count);
        }

        const auto max_presses = *std::max_element(b.data().begin(), b.data().end());

        std::vector<double> free_vars;
        free_vars.resize(free_var_idxs.size());
        auto best_count = std::numeric_limits<int64_t>::max();
        vector best;
        while (std::all_of(free_vars.begin(), free_vars.end(), [max_presses](const auto v) { return v <= max_presses; })) {
            auto x = build_solution(A_r, b_r, free_var_idxs, free_vars, tolerance);
            const auto count = solution_count(x);
            if (count < best_count && try_solution(A, b, x)) {
                best = x;
                best_count = count;
            }
            int incr = 0;
            for (double& v : free_vars) {
                ++v;
                if (v <= max_presses) {
                    break;
                }
                ++incr;
                v = 0;
            }
            if (incr == free_vars.size() && free_vars.back() == 0) {
                break; //Incremented everything, but last is 0 means we looped everything and have tried all values.
            }
        }

        return static_cast<int64_t>(best_count);
    }

    int64_t find_count_p2(const indicator& ind) {
        matrix A {ind.joltages.size(), ind.toggles.size()};
        vector b {vector::convert_tag{}, ind.joltages.begin(), ind.joltages.end()};
        for (const auto [c, t] : ind.toggles | std::views::enumerate) {
            for (size_t r = 0; r < A.num_rows(); r++) {
                light_t v = 1u << r;
                if (v & t) {
                    A[r, c] = 1.0;
                }
            }
        }
        const auto solution = find_solution(A, b);
        return solution;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int sum = 0;
        for (const auto& ind : input) {
            const auto steps = find_match_steps(ind);
            sum += steps;
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (const auto& ind : input) {
            const auto presses = find_count_p2(ind);
            sum += presses;
        }
        return std::to_string(sum);
    }

    aoc::registration r {2025, 10, part_1, part_2};

    TEST_SUITE("2025_day10") {
        TEST_CASE("2025_day10:example") {
            const std::vector<std::string> lines {
                "[.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}",
                "[...#.] (0,2,3,4) (2,3) (0,4) (0,1,2) (1,2,3,4) {7,5,12,7,2}",
                "[.###.#] (0,1,2,3,4) (0,3,4) (0,1,2,4,5) (1,2) {10,11,11,5,10,5}"
            };
            const auto input = get_input(lines);
            const auto steps0 = find_match_steps(input[0]);
            CHECK_EQ(steps0, 2);
        }
    }

} /* namespace <anon> */