//
// Created by Daniel Garcia on 12/13/2025.
//

#ifndef ADVENTOFCODE_INT128_H
#define ADVENTOFCODE_INT128_H

#include <cstdint>

#ifdef AOC_COMPILER_MSVC
#include <__msvc_int128.hpp>
#endif

namespace aoc {


#ifdef AOC_COMPILER_GCC
    using int128 = __int128;
#elifdef AO_COMPILER_CLANG
    using int128 = __int128;
#elifdef AOC_COMPILER_MSVC
    using int128 = std::_Signed128;
#endif


} /* namespace aoc */


#endif //ADVENTOFCODE_INT128_H