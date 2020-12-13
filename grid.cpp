//
// Created by Daniel Garcia on 12/12/20.
//

#include "grid.h"

#include <algorithm>
#include <utility>
#include <stdexcept>

namespace aoc {

    namespace {

        std::vector<char>&& check_data(std::vector<char>&& d, std::size_t row_len) {
            if (d.empty() || d.size() % row_len != 0) {
                throw std::runtime_error{"Invalid grid data."};
            }
            return std::move(d);
        }

    }

    grid::grid(const std::vector<std::string>& lines) : m_data{}, m_num_cols{0}, m_num_rows{lines.size()} {
        if (lines.empty() || lines.front().empty()) {
            throw std::runtime_error{"Cannot create grid from empty set of lines."};
        }
        m_num_cols = lines.front().size();
        if (std::any_of(lines.begin(), lines.end(), [num = m_num_cols](const std::string& s){ return s.size() != num; })) {
            throw std::runtime_error{"Cannot create grid from mismatched set of lines."};
        }
        m_data.reserve(m_num_cols * m_num_rows);
        std::for_each(lines.begin(), lines.end(), [&data = m_data](const std::string& s){ data.insert(data.end(), s.begin(), s.end()); });
    }

    grid::grid(std::vector<char> data, std::size_t row_len) :
        m_data{check_data(std::move(data), row_len)}, m_num_cols{row_len}, m_num_rows{m_data.size() / m_num_cols} {}

    std::optional<position> grid::wrap(position p) const noexcept {
        if (p.x < 0 || p.x >= m_num_rows) {
            //False if we are vertically outside the map.
            return std::nullopt;
        }
        while (p.y < 0) {
            p.y += m_num_cols;
        }
        while (p.y >= m_num_cols) {
            p.y -= m_num_cols;
        }
        return p;
    }

    std::vector<position> grid::neighbors(position p) const {
        std::vector<position> retval;
        retval.reserve(std::size(STANDARD_DIRECTIONS));
        for (auto v : STANDARD_DIRECTIONS) {
            auto np = p + v;
            if (in(np)) {
                retval.push_back(np);
            }
        }
        return retval;
    }

    std::ostream& operator<<(std::ostream& os, const grid& g) {
        std::size_t idx = 0;
        for (auto c : g) {
            os << c;
            ++idx;
            if (idx % g.num_cols() == 0) {
                os << '\n';
            }
        }
        return os;
    }

} /* namespace aoc */