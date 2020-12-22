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

#include "point.h"
#include "stride_span.h"

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
        grid(std::vector<T> data, std::size_t row_len) :
                m_data{check_data(std::move(data), row_len)}, m_num_cols{row_len}, m_num_rows{m_data.size() / m_num_cols} {}

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

            position_iterator operator+(int n) const noexcept { auto retval = *this; retval += n; return *this; }
            position_iterator operator-(int n) const noexcept { auto retval = *this; retval -= n; return *this; }
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

        grid_row operator[](std::size_t idx) noexcept  { return grid_row{m_data.data() + idx * m_num_cols, m_num_cols}; }
        [[nodiscard]] const grid_row operator[](std::size_t idx) const noexcept  { return grid_row{const_cast<char*>(m_data.data() + idx * m_num_cols), m_num_cols}; }

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

            T& operator[](std::size_t idx) noexcept  { return *(m_grid->m_data + m_col_num + idx * m_grid->m_num_cols); }
            [[nodiscard]] T operator[](std::size_t idx) const noexcept  { return *(m_grid->m_data + m_col_num + idx * m_grid->m_num_cols); }

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

                iterator operator+(int n) const noexcept { auto retval = *this; retval += n; return *this; }
                iterator operator-(int n) const noexcept { auto retval = *this; retval -= n; return *this; }
                T& operator[](int n) const { return *(*this + n); }

                difference_type operator-(const position_iterator& rhs) const noexcept { if (m_grid != rhs.m_grid) { return 0; } return m_idx - rhs.m_idx; }
            };

            auto begin() noexcept { return iterator{*m_grid, m_col_num, 0}; }
            auto end() noexcept { return iterator{*m_grid, m_col_num, m_grid->m_num_rows}; }
            [[nodiscard]] auto begin() const noexcept { return iterator{*m_grid, m_col_num, 0}; }
            [[nodiscard]] auto end() const noexcept { return iterator{*m_grid, m_col_num, m_grid->m_num_rows}; }

            [[nodiscard]] auto size() const noexcept { return m_grid->m_num_rows; }
        };

        [[nodiscard]] grid_col column(std::size_t col) noexcept { return {this, col}; }
        [[nodiscard]] const grid_col column(std::size_t col) const noexcept { return {this, col}; }

        [[nodiscard]] stride_span<T> column_span(std::size_t col) noexcept { return {m_data, col, static_cast<std::ptrdiff_t>(m_num_cols)}; }
        [[nodiscard]] stride_span<const T> column_span(std::size_t col) const noexcept { return {m_data, col, static_cast<std::ptrdiff_t>(m_num_cols)}; }

        void display(std::ostream& os, std::optional<position> marked = std::nullopt) const {
            std::size_t idx = 0;
            os << std::setw(6) << std::left << (idx / m_num_cols);
            for (auto p : list_positions()) {
                os << std::setw(8) << std::right;
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
    inline void grid<char>::display(std::ostream &os, std::optional<position> marked) const {
        std::size_t idx = 0;
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
