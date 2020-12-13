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

namespace aoc {

    class grid;

    struct velocity {
        int dx = 0;
        int dy = 0;
    };

    struct position {
        int x = 0;
        int y = 0;

        position& operator+=(velocity v) noexcept {
            x += v.dx;
            y += v.dy;
            return *this;
        }

        position& operator-=(velocity v) noexcept {
            x -= v.dx;
            y -= v.dy;
            return *this;
        }
    };

    inline position operator+(position a, position b) noexcept {
        return {a.x + b.x, a.y + b.y};
    }

    inline position operator-(position a, position b) noexcept {
        return {a.x - b.x, a.y - b.y};
    }

    inline position operator+(position p, velocity v) noexcept {
        return p += v;
    }

    inline position operator-(position p, velocity v) noexcept {
        return p -= v;
    }

    inline bool operator==(position a, position b) noexcept {
        return a.x == b.x && a.y == b.y;
    }

    inline bool operator!=(position a, position b) noexcept {
        return a.x != b.x || a.y != b.y;
    }

    inline position top_left() noexcept {
        return {0, 0};
    }

    constexpr velocity STANDARD_DIRECTIONS[] = {{1,-1}, {1,0}, {1,1}, {0,-1}, {0,1}, {-1,-1}, {-1,0}, {-1,1}};

    class grid {
        std::vector<char> m_data;
        std::size_t m_num_cols;
        std::size_t m_num_rows;
    public:
        explicit grid(const std::vector<std::string>& lines);
        grid(std::vector<char> data, std::size_t row_len);

        [[nodiscard]] std::optional<position> wrap(position p) const noexcept;
        [[nodiscard]] std::vector<position> neighbors(position p) const;

        [[nodiscard]] bool in(position p) const noexcept {
            return p.x >= 0 && p.x < m_num_rows &&
                p.y >= 0 && p.y < m_num_cols;
        }

        [[nodiscard]] std::size_t num_rows() const noexcept { return m_num_rows; }
        [[nodiscard]] std::size_t num_cols() const noexcept { return m_num_cols; }

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

        class grid_row {
            char* m_begin;
            std::size_t m_size;
        public:
            grid_row(char* begin, std::size_t size) noexcept : m_begin{begin}, m_size{size} {}
            grid_row(const grid_row&) = delete;
            grid_row(grid_row&&) = delete;

            char& operator[](std::size_t idx) noexcept  { return *(m_begin + idx); }
            [[nodiscard]] char operator[](std::size_t idx) const noexcept  { return *(m_begin + idx); }

            char& at(std::size_t idx) { if (idx >= m_size) { throw std::out_of_range{"Index out of range."}; } return *(m_begin + idx); }
            [[nodiscard]] char at(std::size_t idx) const { if (idx >= m_size) { throw std::out_of_range{"Index out of range."}; } return *(m_begin + idx); }

            auto begin() noexcept { return m_begin; }
            auto end() noexcept { return m_begin + m_size; }
            [[nodiscard]] auto begin() const noexcept { return m_begin; }
            [[nodiscard]] auto end() const noexcept { return m_begin + m_size; }

            [[nodiscard]] auto size() const noexcept { return m_size; }
        };

        grid_row operator[](std::size_t idx) noexcept  { return grid_row{m_data.data() + idx * m_num_cols, m_num_cols}; }
        [[nodiscard]] const grid_row operator[](std::size_t idx) const noexcept  { return grid_row{const_cast<char*>(m_data.data() + idx * m_num_cols), m_num_cols}; }

        char& at(std::size_t row, std::size_t col) {
            if (row >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (col >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[row * m_num_cols + col];
        }

        [[nodiscard]] char at(std::size_t row, std::size_t col) const {
            if (row >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (col >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[row * m_num_cols + col];
        }

        char& operator[](position p) noexcept { return m_data[p.x * m_num_cols + p.y]; }
        [[nodiscard]] char operator[](position p) const noexcept { return m_data[p.x * m_num_cols + p.y]; }

        char& at(position p) {
            if (p.x >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (p.y >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[p.x * m_num_cols + p.y];
        }

        [[nodiscard]] char at(position p) const {
            if (p.x >= m_num_rows) {
                throw std::out_of_range{"Row index out of range."};
            }
            else if (p.y >= m_num_cols) {
                throw std::out_of_range{"Column index out of range."};
            }
            return m_data[p.x * m_num_cols + p.y];
        }

        void display(std::ostream& os, std::optional<position> marked = std::nullopt) const;
    };

    inline grid::position_iterator operator+(int n, grid::position_iterator i) noexcept { return i + n; }

    std::ostream& operator<<(std::ostream& os, const grid& g);

} /* namespace aoc */

#endif //ADVENTOFCODE_GRID_H
