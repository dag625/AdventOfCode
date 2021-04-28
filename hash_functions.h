//
// Created by Daniel Garcia on 4/27/21.
//

#ifndef ADVENTOFCODE_HASH_FUNCTIONS_H
#define ADVENTOFCODE_HASH_FUNCTIONS_H

#include "point.h"

#include <functional>

namespace std {

    template<> struct hash<aoc::position>
    {
        std::size_t operator()(const aoc::position& s) const noexcept;
    };

} /* namespace std */

#endif //ADVENTOFCODE_HASH_FUNCTIONS_H
