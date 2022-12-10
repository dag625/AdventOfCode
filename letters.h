//
// Created by Daniel Garcia on 12/10/2022.
//

#ifndef ADVENTOFCODE_LETTERS_H
#define ADVENTOFCODE_LETTERS_H

#include "grid.h"

#include <string>

namespace aoc {

    char to_char(const std::vector<stride_span<const char>>& cols);

    std::string letters_from_grid(const grid<char>& g);

} /* namespace aoc */

#endif //ADVENTOFCODE_LETTERS_H
