//
// Created by Dan on 12/8/2021.
//

#ifndef ADVENTOFCODE_RANGES_H
#define ADVENTOFCODE_RANGES_H

#include <ranges>

namespace aoc {

    /*
     * We used to have our own implementation before the C++ standard added it to the
     * STL.  Now that it's in the standard, we replace our version with theirs which
     * is nicer.
     */

    using std::ranges::to;

} /* namespace aoc */

#endif //ADVENTOFCODE_RANGES_H
