//
// Created by Daniel Garcia on 12/17/20.
//

#ifndef ADVENTOFCODE_POINT_ND_H
#define ADVENTOFCODE_POINT_ND_H

#include <array>
#include <numeric>

namespace aoc {

    template <std::size_t D>
    using point = std::array<int, D>;

    //Not useful, but hopefully having this other name will help keep things sorted mentally.
    template <std::size_t D>
    using vector = point<D>;

    template <std::size_t D>
    point<D> operator+(const point<D>& a, const vector<D>& b) {
        point<D> retval{};
        for (std::size_t i = 0; i < D; ++i) {
            retval[i] = a[i] + b[i];
        }
        return retval;
    }

    template <std::size_t D>
    point<D> operator-(const point<D>& a, const vector<D>& b) {
        point<D> retval{};
        for (std::size_t i = 0; i < D; ++i) {
            retval[i] = a[i] - b[i];
        }
        return retval;
    }

    //Dot/inner product
    template <std::size_t D>
    int64_t operator*(const point<D>& a, const vector<D>& b) {
        int64_t retval = 0;
        for (std::size_t i = 0; i < D; ++i) {
            retval += a[i] * b[i];
        }
        return retval;
    }

    //Scale
    template <std::size_t D>
    point<D> operator*(const point<D>& a, const int b) {
        point<D> retval{};
        for (std::size_t i = 0; i < D; ++i) {
            retval[i] = a[i] * b;
        }
        return retval;
    }

    template <std::size_t D, typename = std::enable_if_t<(D >= 2), void>>
    point<D> from_flat(int x, int y) {
        point<D> retval{};
        retval[0] = x;
        retval[1] = y;
        return retval;
    }

    template <std::size_t D>
    constexpr std::size_t NUM_NEIGHBORS_AND_SELF = NUM_NEIGHBORS_AND_SELF<D-1> * 3;

    template <>
    constexpr std::size_t NUM_NEIGHBORS_AND_SELF<1> = 3;

    template <std::size_t D>
    constexpr std::size_t NUM_NEIGHBORS = NUM_NEIGHBORS_AND_SELF<D> - 1;

    template <std::size_t D>
    constexpr std::array<vector<D>, NUM_NEIGHBORS_AND_SELF<D>> get_unit_cube() {
        std::array<point<D>, NUM_NEIGHBORS_AND_SELF<D>> retval{};
        auto less_dims = get_unit_cube<D - 1>();
        std::size_t idx = 0;
        for (const auto& p : less_dims) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < D - 1; ++j) {
                    retval[idx][j] = p[j];
                }
                retval[idx][D - 1] = i - 1;
                ++idx;
            }
        }
        return retval;
    }

    template <>
    constexpr std::array<vector<1>, NUM_NEIGHBORS_AND_SELF<1>> get_unit_cube() {
        return {vector<1>{1}, vector<1>{0}, vector<1>{-1}};
    }

    template <std::size_t D>
    constexpr std::array<vector<D>, NUM_NEIGHBORS<D>> get_standard_directions() {
        std::array<vector<D>, NUM_NEIGHBORS<D>> retval{};
        auto cube = get_unit_cube<D>();
        std::size_t idx = 0;
        for (const auto& p : cube) {
            bool all0 = true;
            for (const auto v : p) {
                if (v != 0) {
                    all0 = false;
                    break;
                }
            }
            if (all0) {
                continue;
            }
            retval[idx] = p;
            ++idx;
        }
        return retval;
    }

    template <std::size_t D>
    constexpr std::size_t NUM_CARDINALS = NUM_CARDINALS<D-1> + 2;

    template <>
    constexpr std::size_t NUM_CARDINALS<1> = 2;

    template <std::size_t D>
    constexpr std::array<vector<D>, NUM_NEIGHBORS<D>> get_cardinal_directions() {
        std::array<vector<D>, NUM_NEIGHBORS<D>> retval{};
        auto cube = get_unit_cube<D>();
        std::size_t idx = 0;
        for (const auto& p : cube) {
            int num_non_zero = 0;
            for (const auto v : p) {
                if (v != 0) {
                    ++num_non_zero;
                }
            }
            if (num_non_zero != 1) {
                continue;
            }
            retval[idx] = p;
            ++idx;
        }
        return retval;
    }

    template <std::size_t D>
    constexpr std::size_t count_count_preserving_directions() {
        std::size_t retval = 0;
        auto cube = get_unit_cube<D>();
        for (const auto& p : cube) {
            int count = 0;
            int abs = 0;
            for (const auto v : p) {
                count += v;
                abs += (v < 0 ? -v : v);
            }
            if (count != 0 || abs == 0) {
                continue;
            }
            ++retval;
        }
        return retval;
    }

    template <std::size_t D>
    constexpr std::size_t NUM_COUNT_PRESERVING_DIRS = count_count_preserving_directions<4>();

    template <std::size_t D>
    constexpr std::array<vector<D>, NUM_COUNT_PRESERVING_DIRS<D>> get_count_preserving_directions() {
        std::array<vector<D>, NUM_COUNT_PRESERVING_DIRS<D>> retval{};
        auto cube = get_unit_cube<D>();
        std::size_t idx = 0;
        for (const auto& p : cube) {
            int count = 0;
            int abs = 0;
            for (const auto v : p) {
                count += v;
                abs += (v < 0 ? -v : v);
            }
            if (count != 0 || abs == 0) {
                continue;
            }
            retval[idx] = p;
            ++idx;
        }
        return retval;
    }

    /* Specializations for Performance: */

    template <>
    inline point<3> operator-(const point<3>& a, const vector<3>& b) {
        return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_POINT_ND_H
