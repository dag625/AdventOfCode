//
// Created by Daniel Garcia on 12/13/2024.
//

#ifndef ADVENTOFCODE_GRID_COMBINE_H
#define ADVENTOFCODE_GRID_COMBINE_H

#include "grid.h"

#include <algorithm>
#include <numeric>

namespace aoc {

    template <typename T>
    std::optional<grid<T>> combine(const grid<T>& top_left, const grid<T>& top_right, const grid<T>& bot_left, const grid<T>& bot_right) {
        if (top_left.num_rows() != top_right.num_rows() ||
            bot_left.num_rows() != bot_right.num_rows() ||
            top_left.num_cols() != bot_left.num_cols() ||
            top_right.num_cols() != bot_right.num_cols())
        {
            return std::nullopt;
        }
        grid<T> retval {static_cast<std::size_t>(top_left.num_rows() + bot_left.num_rows()),
                        static_cast<std::size_t>(top_left.num_cols() + top_right.num_cols())};
        auto do_copy = [&retval](const grid<T>& sub, const position offset){
            for (int r = 0; r < sub.num_rows(); ++r) {
                for (int c = 0; c < sub.num_cols(); ++c) {
                    retval[offset.x + r][offset.y + c] = sub[r][c];
                }
            }
        };

        position offset {};
        do_copy(top_left, offset);

        offset.x = 0;
        offset.y = top_left.num_cols();
        do_copy(top_right, offset);

        offset.x = top_left.num_rows();
        offset.y = 0;
        do_copy(bot_left, offset);

        offset.x = top_left.num_rows();
        offset.y = top_left.num_cols();
        do_copy(bot_right, offset);

        return retval;
    }

    template <typename T>
    std::optional<grid<T>> combine_horizontal(const grid<T>& left, const grid<T>& right) {
        if (left.num_rows() != right.num_rows()) {
            return std::nullopt;
        }
        grid<T> retval {static_cast<std::size_t>(left.num_rows()),
                        static_cast<std::size_t>(left.num_cols() + right.num_cols())};
        auto do_copy = [&retval](const grid<T>& sub, const position offset){
            for (int r = 0; r < sub.num_rows(); ++r) {
                for (int c = 0; c < sub.num_cols(); ++c) {
                    retval[offset.x + r][offset.y + c] = sub[r][c];
                }
            }
        };

        position offset {};
        do_copy(left, offset);

        offset.x = 0;
        offset.y = left.num_cols();
        do_copy(right, offset);

        return retval;
    }

    template <typename T>
    std::optional<grid<T>> combine_horizontal(const std::vector<grid<T>>& grids) {
        if (grids.size() < 2 || !std::all_of(grids.begin(), grids.end(), [num = grids.front().num_rows()](const auto& g){ return g.num_rows() == num; })) {
            return std::nullopt;
        }
        const auto num_cols = std::accumulate(grids.begin(), grids.end(), static_cast<std::size_t>(0),
                                              [](std::size_t tot, const auto& g){ return tot + g.num_cols(); });
        grid<T> retval {static_cast<std::size_t>(grids.front().num_rows()),
                        static_cast<std::size_t>(num_cols)};
        auto do_copy = [&retval](const grid<T>& sub, const position offset){
            for (int r = 0; r < sub.num_rows(); ++r) {
                for (int c = 0; c < sub.num_cols(); ++c) {
                    retval[offset.x + r][offset.y + c] = sub[r][c];
                }
            }
        };

        position offset {};
        for (const auto& g : grids) {
            do_copy(g, offset);
            offset.y += g.num_cols();
        }

        return retval;
    }

    template <typename T>
    std::optional<grid<T>> combine_vertical(const grid<T>& top, const grid<T>& bot) {
        if (top.num_cols() != bot.num_cols()) {
            return std::nullopt;
        }
        grid<T> retval {static_cast<std::size_t>(top.num_rows() + bot.num_rows()),
                        static_cast<std::size_t>(top.num_cols())};
        auto do_copy = [&retval](const grid<T>& sub, const position offset){
            for (int r = 0; r < sub.num_rows(); ++r) {
                for (int c = 0; c < sub.num_cols(); ++c) {
                    retval[offset.x + r][offset.y + c] = sub[r][c];
                }
            }
        };

        position offset {};
        do_copy(top, offset);

        offset.x = top.num_rows();
        offset.y = 0;
        do_copy(bot, offset);

        return retval;
    }

    template <typename T>
    std::optional<grid<T>> combine_vertical(const std::vector<grid<T>>& grids) {
        if (grids.size() < 2 || !std::all_of(grids.begin(), grids.end(), [num = grids.front().num_cols()](const auto& g){ return g.num_cols() == num; })) {
            return std::nullopt;
        }
        const auto num_rows = std::accumulate(grids.begin(), grids.end(), static_cast<std::size_t>(0),
                                              [](std::size_t tot, const auto& g){ return tot + g.num_rows(); });
        grid<T> retval {static_cast<std::size_t>(num_rows),
                        static_cast<std::size_t>(grids.front().num_cols())};
        auto do_copy = [&retval](const grid<T>& sub, const position offset){
            for (int r = 0; r < sub.num_rows(); ++r) {
                for (int c = 0; c < sub.num_cols(); ++c) {
                    retval[offset.x + r][offset.y + c] = sub[r][c];
                }
            }
        };

        position offset {};
        for (const auto& g : grids) {
            do_copy(g, offset);
            offset.x += g.num_rows();
        }

        return retval;
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_GRID_COMBINE_H
