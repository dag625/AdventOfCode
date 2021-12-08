//
// Created by Dan on 12/8/2021.
//

#ifndef ADVENTOFCODE_RANGES_H
#define ADVENTOFCODE_RANGES_H

#include <ranges>

/*
 * This implementation is from https://stackoverflow.com/a/60971856.
 */

namespace aoc {

    namespace detail {
        // Type acts as a tag to find the correct operator| overload
        template <typename C>
        struct to_helper {
        };

        // This actually does the work
        template <typename Container, std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, typename Container::value_type>
        Container operator|(R&& r, to_helper<Container>) {
            return Container{r.begin(), r.end()};
        }
    }

    // Couldn't find an concept for container, however a
    // container is a range, but not a view.
    template <typename Container>
    auto to() {
        return detail::to_helper<Container>{};
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_RANGES_H
