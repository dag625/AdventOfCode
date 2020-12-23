//
// Created by Daniel Garcia on 12/23/20.
//

#include "registration.h"
#include "challenge.h"

namespace aoc {

    registration::registration(int y, int d, challenge_function c1, challenge_function c2) noexcept {
        if (c1 != nullptr) {
            challenges::list().add(y, d, 1, c1);
        }
        if (c2 != nullptr) {
            challenges::list().add(y, d, 2, c2);
        }
    }

} /* namespace aoc */