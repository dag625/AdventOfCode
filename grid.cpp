//
// Created by Daniel Garcia on 12/12/20.
//

#include "grid.h"

#include <algorithm>
#include <utility>
#include <stdexcept>
#include <iomanip>

namespace aoc {

    grid<char> to_grid(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end) {
        auto num_rows = std::distance(begin, end);
        if (num_rows == 0 || begin->empty()) {
            throw std::runtime_error{"Cannot create grid from empty set of lines."};
        }
        auto num_cols = begin->size();
        if (std::any_of(begin, end, [num = num_cols](const std::string& s){ return s.size() != num; })) {
            throw std::runtime_error{"Cannot create grid from mismatched set of lines."};
        }
        std::vector<char> data;
        data.reserve(num_cols * num_rows);
        std::for_each(begin, end, [&data](const std::string& s){ data.insert(data.end(), s.begin(), s.end()); });
        return {std::move(data), num_cols};
    }

    grid<char> to_grid(const std::vector<std::string>& lines) { return to_grid(lines.begin(), lines.end()); }

    std::ostream& operator<<(std::ostream& os, position p) {
        os << "(" << p.x << ", " << p.y << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, velocity v) {
        os << "[" << v.dx << ", " << v.dy << "]";
        return os;
    }

} /* namespace aoc */