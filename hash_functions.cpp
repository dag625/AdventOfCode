//
// Created by Daniel Garcia on 4/27/21.
//

#include "hash_functions.h"
#include "utility_templates.h"

namespace std {

    std::size_t hash<aoc::position>::operator()(const aoc::position& s) const noexcept {
        return aoc::multi_hash(s.x, s.y);
    }

} /* namespace std */