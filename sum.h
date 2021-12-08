//
// Created by Dan on 12/8/2021.
//

#ifndef ADVENTOFCODE_SUM_H
#define ADVENTOFCODE_SUM_H

#include <numeric>
#include <ranges>

namespace aoc {

    template <std::ranges::range R>
    int64_t sum(const R& r){
        return std::accumulate(std::begin(r), std::end(r), 0ll, [](int64_t v, int64_t next){ return v + next; });
    }

    template <std::ranges::range R, std::invocable<std::ranges::range_value_t<R>> F>
    int64_t sum(const R& r, F get_value){
        return std::accumulate(std::begin(r), std::end(r), 0ll, [&get_value](int64_t v, const auto& next){ return v + get_value(next); });
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_SUM_H
