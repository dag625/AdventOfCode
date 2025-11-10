//
// Created by Daniel Garcia on 12/12/20.
//

#ifndef ADVENTOFCODE_GRID_H
#define ADVENTOFCODE_GRID_H

#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <ostream>
#include <iomanip>
#include <cmath>

#include "point.h"
#include "stride_span.h"
#include "ranges.h"

namespace aoc {

    template <typename T>
    class grid {
        std::vector<T> m_data;
        std::size_t m_num_cols;
        std::size_t m_num_rows;

        static std::vector<T>&& check_data(std::vector<T>&& d, std::size_t row_len) {
            if (d.empty() || d.size() % row_len != 0) {
                throw std::runtime_error{"Invalid grid data."};
            }
            return std::move(d);
        }

    public:
        grid(std::size_t num_rows, std::size_t num_cols) :
                m_data(num_rows * num_cols), m_num_cols{num_cols}, m_num_rows{num_rows} {}
        grid(std::vector<T> data, std::size_t row_len) :
                m_data{check_data(std::move(data), row_len)}, m_num_cols{row_len}, m_num_rows{m_data.size() / m_num_cols} {}

        bool operator==(const grid<T>& rhs) const { return m_num_cols == rhs.m_num_cols && std::equal(m_data.begin(), m_data.end(), rhs.m_data.begin(), rhs.m_data.end()); }

        const std::vector<T>& data() const { return m_data; }

        [[nodiscard]] std::optional<position> wrap(position p) const noexcept {
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

        [[nodiscard]] position wrap_any(position p) const noexcept {
            while (p.x < 0) {
                p.x += m_num_rows;
            }
            while (p.x >= m_num_rows) {
                p.x -= m_num_rows;
            }
            while (p.y < 0) {
                p.y += m_num_cols;
            }
            while (p.y >= m_num_cols) {
                p.y -= m_num_cols;
            }
            return p;
        }

        [[nodiscard]] std::vector<position> neighbors(position p) const  {
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

        [[nodiscard]] std::vector<position> cardinal_neighbors(position p) const  {
            std::vector<position> retval;
            retval.reserve(std::size(CARDINAL_DIRECTIONS));
            for (auto v : CARDINAL_DIRECTIONS) {
                auto np = p + v;
                if (in(np)) {
                    retval.push_back(np);
                }
            }
            return retval;
        }

        [[nodiscard]] std::vector<position> neighbors(position p, bool cardinal_only) const  {
            return cardinal_only ? cardinal_neighbors(p) : neighbors(p);
        }

        [[nodiscard]] bool in(position p) const noexcept {
            return p.x >= 0 && p.x < m_num_rows &&
                p.y >= 0 && p.y < m_num_cols;
        }

        [[nodiscard]] std::size_t num_rows() const noexcept { return m_num_rows; }
        [[nodiscard]] std::size_t num_cols() const noexcept { return m_num_cols; }

        [[nodiscard]] const std::vector<T>& buffer() const { return m_data; }

        auto begin() noexcept { return m_data.begin(); }
        auto end() noexcept { return m_data.end(); }
        [[nodiscard]] auto begin() const noexcept { return m_data.begin(); }
        [[nodiscard]] auto end() const noexcept { return m_data.end(); }

        [[nodiscard]] auto size() const noexcept { return m_data.size(); }

        [[nodiscard]] grid<T> expand(int add_to_edge, T fill_val) const {
            grid<T> retval {num_rows() + 2*add_to_edge, num_cols() + 2*add_to_edge};
            velocity to_add{add_to_edge, add_to_edge};
            for (auto pos : list_positions()) {
                retval[pos + to_add] = at(pos);
            }
            for (int i = 0; i < add_to_edge; ++i) {
                for (auto& v : retval.row_span(i)) {
                    v = fill_val;
                }
                for (auto& v : retval.row_span(retval.num_rows() - 1 - i)) {
                    v = fill_val;
                }
                for (auto& v : retval.column_span(i)) {
                    v = fill_val;
                }
                for (auto& v : retval.column_span(retval.num_cols() - 1 - i)) {
                    v = fill_val;
                }
            }
            return retval;
        }

        [[nodiscard]] grid<T> rotate(bool clockwise = true) const {
            return clockwise ? rotate_cw() : rotate_ccw();
        }

        [[nodiscard]] grid<T> rotate_cw() const {
            grid<T> retval = *this;
            auto rot = [this](position p){ return position{p.y, static_cast<int>(num_cols())-p.x-1}; };
            for (const auto p : this->list_positions()) {
                retval[rot(p)] = this->at(p);
            }
            return retval;
        }

        [[nodiscard]] grid<T> rotate_ccw() const {
            grid<T> retval = *this;
            auto rot = [this](position p){ return position{static_cast<int>(num_rows())-p.y-1, p.x}; };
            for (const auto p : this->list_positions()) {
                retval[rot(p)] = this->at(p);
            }
            return retval;
        }

        [[nodiscard]] grid<T> flip(bool horizontal = true) const {
            return horizontal ? flip_horizontal() : flip_vertical();
        }

        [[nodiscard]] grid<T> flip_horizontal() const {
            grid<T> retval = *this;
            auto flip = [this](position p){ return position{p.x, static_cast<int>(num_cols()) - p.y - 1}; };
            for (const auto p : this->list_positions()) {
                retval[flip(p)] = this->at(p);
            }
            return retval;
        }

        [[nodiscard]] grid<T> flip_vertical() const {
            grid<T> retval = *this;
            auto flip = [this](position p){ return position{static_cast<int>(num_rows()) - p.x - 1, p.y}; };
            for (const auto p : this->list_positions()) {
                retval[flip(p)] = this->at(p);
            }
            return retval;
        }

        [[nodiscard]] grid<T> subgrid(position origin, position size) const {
            grid<T> retval {static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y)};
            for (int r = 0; r < size.x; ++r) {
                for (int c = 0; c < size.y; ++c) {
                    retval[r][c] = this->at(origin.x + r, origin.y + c);
                }
            }
            return retval;
        }

        template<typename U, std::invocable<T> F>
        [[nodiscard]] grid<U> map(F map_func) {
            return grid<U>{m_data | std::views::transform(map_func) | to<std::vector<U>>(), num_cols()};
        }

        friend class position_iterator;
        class position_iterator {
            const grid* m_grid;
            std::size_t m_idx;
            [[nodiscard]] bool is_end() const { return m_grid == nullptr || m_idx >= m_grid->size(); }
        public:
            using difference_type = decltype(std::declval<std::size_t>() - std::declval<std::size_t>());
            using value_type = position;
            using pointer = position*;
            using reference = position;
            using iterator_category = std::bidirectional_iterator_tag;

            position_iterator() noexcept : m_grid{nullptr}, m_idx{0} {}
            explicit position_iterator(const grid& g, std::size_t i = 0) noexcept : m_grid{&g}, m_idx{i} {}

            bool operator==(const position_iterator& rhs) const { return m_idx == rhs.m_idx || (is_end() && is_end() == rhs.is_end()); }
            bool operator!=(const position_iterator& rhs) const { return m_idx != rhs.m_idx && (!is_end() || is_end() != rhs.is_end()); }
            bool operator< (const position_iterator& rhs) const { return m_idx <  rhs.m_idx; }
            bool operator<=(const position_iterator& rhs) const { return m_idx <= rhs.m_idx; }
            bool operator> (const position_iterator& rhs) const { return m_idx >  rhs.m_idx; }
            bool operator>=(const position_iterator& rhs) const { return m_idx >= rhs.m_idx; }

            position operator*() const {
                if (m_grid == nullptr) {
                    throw std::runtime_error{"Cannot dereference."};
                }
                return {static_cast<int>(m_idx / m_grid->m_num_cols), static_cast<int>(m_idx % m_grid->m_num_cols)};
            }

            position_iterator& operator++() noexcept { ++m_idx; return *this; }
            position_iterator operator++(int) noexcept { auto retval = *this; ++m_idx; return retval; }

            position_iterator& operator--() noexcept { --m_idx; return *this; }
            position_iterator operator--(int) noexcept { auto retval = *this; --m_idx; return retval; }

            position_iterator& operator+=(int n) noexcept { m_idx += n; return *this; }
            position_iterator& operator-=(int n) noexcept { m_idx -= n; return *this; }

            position_iterator operator+(int n) const noexcept { auto retval = *this; retval += n; return retval; }
            position_iterator operator-(int n) const noexcept { auto retval = *this; retval -= n; return retval; }
            position operator[](int n) const { return *(*this + n); }

            difference_type operator-(const position_iterator& rhs) const noexcept { if (m_grid != rhs.m_grid) { return 0; } return m_idx - rhs.m_idx; }
        };

        class position_list {
            const grid* m_grid;
        public:
            explicit position_list(const grid& g) noexcept : m_grid{&g} {}

            [[nodiscard]] position_iterator begin() const noexcept { return position_iterator{*m_grid}; }
            [[nodiscard]] position_iterator end() const noexcept { return position_iterator{*m_grid, m_grid->size()}; }
            [[nodiscard]] std::size_t size() const noexcept { return m_grid->size(); }
        };

        [[nodiscard]] position_list list_positions() const noexcept { return position_list{*this}; }

        void erase_row(std::size_t row_idx) {
            if (row_idx < num_rows()) {
                m_data.erase(m_data.begin() + row_idx * m_num_cols, m_data.begin() + (row_idx + 1) * m_num_cols);
                --m_num_rows;
            }
        }

        class grid_row {
            T* m_begin;
            std::size_t m_size;
        public:
            static constexpr bool is_view = true;

            grid_row(T* begin, std::size_t size) noexcept : m_begin{begin}, m_size{size} {}
            grid_row(const grid_row&) = delete;
            grid_row(grid_row&&) = delete;

            T& operator[](std::size_t idx) noexcept  { return *(m_begin + idx); }
            [[nodiscard]] T operator[](std::size_t idx) const noexcept  { return *(m_begin + idx); }

            T& at(std::size_t idx) { if (idx >= m_size) { throw std::out_of_range{"Index out of range."}; } return *(m_begin + idx); }
            [[nodiscard]] T at(std::size_t idx) const { if (idx >= m_size) { throw std::out_of_range{"Index out of range."}; } return *(m_begin + idx); }

            auto begin() noexcept { return m_begin; }
            auto end() noexcept { return m_begin + m_size; }
            [[nodiscard]] auto begin() const noexcept { return m_begin; }
            [[nodiscard]] auto end() const noexcept { return m_begin + m_size; }

            [[nodiscard]] auto size() const noexcept { return m_size; }

            T* data() { return m_begin; }
            const T* data() const { return m_begin; }
        };

        grid_row row(std::size_t idx) noexcept  { return grid_row{m_data.data() + idx * m_num_cols, m_num_cols}; }
        [[nodiscard]] const grid_row row(std::size_t idx) const noexcept { return grid_row{const_cast<T*>(m_data.data() + idx * m_num_cols), m_num_cols}; }

        grid_row operator[](std::size_t idx) noexcept  { return row(idx); }
        [[nodiscard]] const grid_row operator[](std::size_t idx) const noexcept  { return row(idx); }


        T& at(std::size_t row, std::size_t col) {
            if (row >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (col >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[row * m_num_cols + col];
        }

        [[nodiscard]] T at(std::size_t row, std::size_t col) const {
            if (row >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (col >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[row * m_num_cols + col];
        }

        T& operator[](position p) noexcept { return m_data[p.x * m_num_cols + p.y]; }
        [[nodiscard]] T operator[](position p) const noexcept { return m_data[p.x * m_num_cols + p.y]; }

        T& at(position p) {
            if (p.x >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (p.y >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[p.x * m_num_cols + p.y];
        }

        [[nodiscard]] T at(position p) const {
            if (p.x >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (p.y >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[p.x * m_num_cols + p.y];
        }

        friend class grid_col;
        class grid_col {
            grid* m_grid;
            std::size_t m_col_num;
        public:
            static constexpr bool is_view = true;

            grid_col(grid* g, std::size_t col) noexcept : m_grid{g}, m_col_num{col} {}
            //grid_col(const grid_row&) = delete;
            //grid_col(grid_row&&) = delete;

            T& operator[](std::size_t idx) noexcept  { return *(m_grid->m_data.data() + m_col_num + idx * m_grid->m_num_cols); }
            [[nodiscard]] T operator[](std::size_t idx) const noexcept  { return *(m_grid->m_data.data() + m_col_num + idx * m_grid->m_num_cols); }

            T& at(std::size_t idx) { if (idx >= m_grid->m_num_cols) { throw std::out_of_range{"Index out of range."}; } return *(m_grid->m_data + m_col_num + idx * m_grid->m_num_cols); }
            [[nodiscard]] T at(std::size_t idx) const { if (idx >= m_grid->m_num_cols) { throw std::out_of_range{"Index out of range."}; } return *(m_grid->m_data + m_col_num + idx * m_grid->m_num_cols); }

            friend class iterator;
            class iterator {
                grid* m_grid;
                std::size_t m_col;
                std::size_t m_idx;
                [[nodiscard]] bool is_end() const { return m_grid == nullptr || m_idx >= m_grid->m_num_rows; }
            public:
                using difference_type = decltype(std::declval<std::size_t>() - std::declval<std::size_t>());
                using value_type = T;
                using pointer = T*;
                using reference = T&;
                using iterator_category = std::random_access_iterator_tag;

                iterator() noexcept : m_grid{nullptr}, m_col{0}, m_idx{0} {}
                explicit iterator(grid& g, std::size_t col, std::size_t i = 0) noexcept : m_grid{&g}, m_col{col}, m_idx{i} {}

                bool operator==(const iterator& rhs) const { return m_idx == rhs.m_idx || (is_end() && is_end() == rhs.is_end()); }
                bool operator!=(const iterator& rhs) const { return m_idx != rhs.m_idx && (!is_end() || is_end() != rhs.is_end()); }
                bool operator< (const iterator& rhs) const { return m_idx <  rhs.m_idx; }
                bool operator<=(const iterator& rhs) const { return m_idx <= rhs.m_idx; }
                bool operator> (const iterator& rhs) const { return m_idx >  rhs.m_idx; }
                bool operator>=(const iterator& rhs) const { return m_idx >= rhs.m_idx; }

                T& operator*() {
                    if (m_grid == nullptr) {
                        throw std::runtime_error{"Cannot dereference."};
                    }
                    return *(m_grid->m_data.data() + m_col + m_idx * m_grid->m_num_cols);
                }

                const T& operator*() const {
                    if (m_grid == nullptr) {
                        throw std::runtime_error{"Cannot dereference."};
                    }
                    return *(m_grid->m_data.data() + m_col + m_idx * m_grid->m_num_cols);
                }

                iterator& operator++() noexcept { ++m_idx; return *this; }
                iterator operator++(int) noexcept { auto retval = *this; ++m_idx; return retval; }

                iterator& operator--() noexcept { --m_idx; return *this; }
                iterator operator--(int) noexcept { auto retval = *this; --m_idx; return retval; }

                iterator& operator+=(int n) noexcept { m_idx += n; return *this; }
                iterator& operator-=(int n) noexcept { m_idx -= n; return *this; }

                iterator operator+(difference_type n) const noexcept { auto retval = *this; retval += n; return retval; }
                iterator operator-(difference_type n) const noexcept { auto retval = *this; retval -= n; return retval; }
                T& operator[](int n) const { return *(*this + n); }

                difference_type operator-(const iterator& rhs) const noexcept { if (m_grid != rhs.m_grid || m_col != rhs.m_col) { return 0; } return m_idx - rhs.m_idx; }
                difference_type operator-(const position_iterator& rhs) const noexcept { if (m_grid != rhs.m_grid) { return 0; } return m_idx - rhs.m_idx; }
            };

            auto begin() noexcept { return iterator{*m_grid, m_col_num, 0}; }
            auto end() noexcept { return iterator{*m_grid, m_col_num, m_grid->m_num_rows}; }
            [[nodiscard]] auto begin() const noexcept { return iterator{*m_grid, m_col_num, 0}; }
            [[nodiscard]] auto end() const noexcept { return iterator{*m_grid, m_col_num, m_grid->m_num_rows}; }

            [[nodiscard]] auto size() const noexcept { return m_grid->m_num_rows; }
        };

        [[nodiscard]] grid_col column(std::size_t col) noexcept { return {this, col}; }
        [[nodiscard]] const grid_col column(std::size_t col) const noexcept { return {const_cast<grid*>(this), col}; }

        [[nodiscard]] stride_span<T> row_span(std::size_t row) noexcept { return {m_data, static_cast<typename stride_span<T>::length>(num_cols()), row * num_cols()}; }
        [[nodiscard]] stride_span<const T> row_span(std::size_t row) const noexcept { return {m_data, static_cast<typename stride_span<const T>::length>(num_cols()), row * num_cols()}; }

        [[nodiscard]] stride_span<T> column_span(std::size_t col) noexcept { return {m_data, col, static_cast<std::ptrdiff_t>(m_num_cols)}; }
        [[nodiscard]] stride_span<const T> column_span(std::size_t col) const noexcept { return {m_data, col, static_cast<std::ptrdiff_t>(m_num_cols)}; }
        [[nodiscard]] stride_span<const T> const_column_span(std::size_t col) noexcept { return {m_data, col, static_cast<std::ptrdiff_t>(m_num_cols)}; }
        [[nodiscard]] stride_span<const T> const_column_span(std::size_t col) const noexcept { return {m_data, col, static_cast<std::ptrdiff_t>(m_num_cols)}; }

        [[nodiscard]] grid expand_2x(const T& fill_item) const {
            grid retval {num_rows() * 2 + 1, num_cols() * 2 + 1};
            std::fill(retval.begin(), retval.end(), fill_item);
            for (int x = 0; x < num_rows(); ++x) {
                for (int y = 0; y < num_cols(); ++y) {
                    retval[position{2 * x + 1, 2 * y + 1}] = (*this)[position{x, y}];
                }
            }
            return retval;
        }

        [[nodiscard]] grid compress_2x() const {
            grid retval {num_rows() / 2, num_cols() / 2};
            for (int x = 0; x < retval.num_rows(); ++x) {
                for (int y = 0; y < retval.num_cols(); ++y) {
                    retval[position{x, y}] = (*this)[position{2 * x + 1, 2 * y + 1}];
                }
            }
            return retval;
        }

        [[nodiscard]] grid transpose() const {
            grid retval {num_cols(), num_rows()};
            for (int x = 0; x < retval.num_rows(); ++x) {
                for (int y = 0; y < retval.num_cols(); ++y) {
                    retval[position{x, y}] = (*this)[position{y, x}];
                }
            }
            return retval;
        }

        void display(std::ostream& os, std::optional<position> marked = std::nullopt, int col_width = 8) const {
            std::size_t idx = 0;
            os << std::setw(6) << std::left << (idx / m_num_cols);
            for (auto p : list_positions()) {
                os << std::setw(col_width) << std::right;
                if (marked && *marked == p) {
                    os << 'X';
                }
                else {
                    os << at(p);
                }
                ++idx;
                if (idx % m_num_cols == 0) {
                    os << '\n';
                    if (idx < m_data.size()) {
                        os << std::setw(6) << std::left << (idx / m_num_cols);
                    }
                }
            }
        }
    };

    grid<char> to_grid(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end);
    grid<char> to_grid(const std::vector<std::string>& lines);

    template <>
    inline void grid<char>::display(std::ostream &os, std::optional<position> marked, int) const {
        std::size_t idx = 0;
        auto hdr_height = static_cast<int>(std::ceil(log10(m_num_cols)));
        for (std::size_t row = 0; row < hdr_height; ++row) {
            os << std::setw(6) << std::left << ' ';
            const auto div = static_cast<int64_t>(std::pow(10, hdr_height - row - 1));
            const auto mod = div * 10;
            for (std::size_t col = 0; col < m_num_cols; ++col) {
                auto digit = (col % mod) / div;
                os << digit;
            }
            os << '\n';
        }
        os << std::setw(6) << std::left << (idx / m_num_cols);
        for (auto p : list_positions()) {
            if (marked && *marked == p) {
                os << 'x';
            }
            else {
                os << at(p);
            }
            ++idx;
            if (idx % m_num_cols == 0) {
                os << '\n';
                if (idx < m_data.size()) {
                    os << std::setw(6) << std::left << (idx / m_num_cols);
                }
            }
        }
    }

    template<typename T>
    inline typename grid<T>::position_iterator operator+(int n, typename grid<T>::position_iterator i) noexcept { return i + n; }

    std::ostream& operator<<(std::ostream& os, position p);
    std::ostream& operator<<(std::ostream& os, velocity v);

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const grid<T>& g) {
        g.display(os);
        return os;
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_GRID_H
