//
// Created by Daniel Garcia on 12/12/2025.
//

#ifndef LATESTER_MATRIX_H
#define LATESTER_MATRIX_H

#include <iostream>
#include <utility>
#include <vector>
#include <mdspan>
#include <format>
#include <numeric>
#include <stdexcept>

namespace aoc {

    struct matrix_exception : public std::runtime_error {
        matrix_exception(const std::string& msg) : std::runtime_error{msg} {}
    };

    class vector;

    class matrix {
    public:
        using data_t = std::mdspan<double, std::dextents<size_t, 2>>;
        using row_t = std::mdspan<double, std::dextents<size_t, 1>, std::layout_stride>;
        using const_row_t = std::mdspan<const double, std::dextents<size_t, 1>, std::layout_stride>;
        using col_t = std::mdspan<double, std::dextents<size_t, 1>, std::layout_stride>;
        using const_col_t = std::mdspan<const double, std::dextents<size_t, 1>, std::layout_stride>;
    private:
        std::vector<double> m_buffer;
        data_t m_mat;

        void check_same_size(const matrix& other) const;
    public:
        matrix() = default;
        matrix(size_t square_size) : m_buffer(square_size * square_size, 0.0), m_mat{m_buffer.data(), square_size, square_size} {}
        matrix(size_t num_rows, size_t num_cols) : m_buffer(num_rows * num_cols, 0.0), m_mat{m_buffer.data(), num_rows, num_cols} {}
        matrix(std::initializer_list<std::initializer_list<double>> list);

        bool operator==(const matrix& rhs) const {
            return num_rows() == rhs.num_rows() && std::ranges::equal(m_buffer, rhs.m_buffer);
        }

        [[nodiscard]] size_t num_rows() const { return m_mat.extent(0); }
        [[nodiscard]] size_t num_cols() const { return m_mat.extent(1); }
        [[nodiscard]] bool is_same_size(const matrix& other) const { return num_rows() == other.num_rows() && num_cols() == other.num_cols(); }

        std::vector<double>& data() { return m_buffer; }
        [[nodiscard]] const std::vector<double>& data() const { return m_buffer; }

        data_t& span() { return m_mat; }
        [[nodiscard]] const data_t& span() const { return m_mat; }

        [[nodiscard]] matrix submatrix(size_t num_rows, size_t num_cols) const;

        [[nodiscard]] row_t row_span(size_t row);
        [[nodiscard]] const_row_t row_cspan(size_t row);
        [[nodiscard]] const_row_t row_span(size_t row) const;
        [[nodiscard]] const_row_t row_cspan(size_t row) const;
        [[nodiscard]] std::vector<double> row_data(size_t row) const;
        [[nodiscard]] vector row_vector(size_t row) const;

        [[nodiscard]] col_t col_span(size_t col);
        [[nodiscard]] const_col_t col_cspan(size_t col);
        [[nodiscard]] const_col_t col_span(size_t col) const;
        [[nodiscard]] const_col_t col_cspan(size_t col) const;
        [[nodiscard]] std::vector<double> col_data(size_t col) const;
        [[nodiscard]] vector col_vector(size_t row) const;

        double& operator[](size_t row, size_t col) { return m_mat[row, col]; }
        const double& operator[](size_t row, size_t col) const { return m_mat[row, col]; }

        void round_all();
        void set_zeros(double tolerance);

        static matrix identity(size_t size);

        [[nodiscard]] matrix& to_transpose();
        [[nodiscard]] matrix transpose() const;

        [[nodiscard]] matrix adjoin(const matrix& rhs) const;
        [[nodiscard]] matrix adjoin_below(const matrix& rhs) const;

        [[nodiscard]] matrix& to_inverse();
        [[nodiscard]] matrix inverse() const;

        [[nodiscard]] matrix& to_row_echelon(double tolerance = 0.0001);
        [[nodiscard]] matrix row_echelon(double tolerance = 0.0001) const;
        [[nodiscard]] matrix& to_reduced_row_echelon(double tolerance = 0.0001);
        [[nodiscard]] matrix reduced_row_echelon(double tolerance = 0.0001) const;

        void swap_rows(size_t r1, size_t r2) noexcept {
            using std::swap;
            for (size_t c = 0; c < num_cols(); ++c) {
                swap(m_mat[r1, c], m_mat[r2, c]);
            }
        }

        void swap_cols(size_t c1, size_t c2) noexcept {
            using std::swap;
            for (size_t r = 0; r < num_rows(); ++r) {
                swap(m_mat[r, c1], m_mat[r, c2]);
            }
        }

        matrix operator+(const matrix& rhs) const;
        matrix operator-(const matrix& rhs) const;
        matrix operator*(const matrix& rhs) const;

        matrix operator+(const double rhs) const {
            matrix retval {num_rows(), num_cols()};
            for (int i = 0; i < m_buffer.size(); ++i) {
                retval.m_buffer[i] = m_buffer[i] + rhs;
            }
            return retval;
        }

        matrix operator-(const double rhs) const {
            matrix retval {num_rows(), num_cols()};
            for (int i = 0; i < m_buffer.size(); ++i) {
                retval.m_buffer[i] = m_buffer[i] - rhs;
            }
            return retval;
        }

        matrix operator*(const double rhs) const {
            matrix retval {num_rows(), num_cols()};
            for (int i = 0; i < m_buffer.size(); ++i) {
                retval.m_buffer[i] = m_buffer[i] * rhs;
            }
            return retval;
        }

        matrix operator/(const double rhs) const {
            matrix retval {num_rows(), num_cols()};
            for (int i = 0; i < m_buffer.size(); ++i) {
                retval.m_buffer[i] = m_buffer[i] / rhs;
            }
            return retval;
        }

        matrix& operator+=(const double rhs) {
            for (double& v : m_buffer) {
                v += rhs;
            }
            return *this;
        }

        matrix& operator-=(const double rhs) {
            for (double& v : m_buffer) {
                v -= rhs;
            }
            return *this;
        }

        matrix& operator*=(const double rhs) {
            for (double& v : m_buffer) {
                v *= rhs;
            }
            return *this;
        }

        matrix& operator/=(const double rhs) {
            for (double& v : m_buffer) {
                v /= rhs;
            }
            return *this;
        }

        [[nodiscard]] std::string to_string(int width = 8, int precision = 0) const;
    };

    class vector {
    public:
        using data_t = std::mdspan<double, std::dextents<size_t, 1>>;
    private:
        std::vector<double> m_buffer;
        data_t m_vec;

        void check_same_size(const vector& other) const {
            if (size() != other.size()) {
                throw matrix_exception{std::format("Vector have different sizes {} vs {}.",
                    size(), other.size())};
            }
        }
    public:
        vector() = default;
        vector(size_t size) : m_buffer(size, 0.0), m_vec{m_buffer.data(), size} {}
        vector(std::initializer_list<double> list) : m_buffer(list), m_vec {m_buffer.data(), m_buffer.size()} {}
        vector(std::vector<double> buf) : m_buffer(std::move(buf)), m_vec {m_buffer.data(), m_buffer.size()} {}
        template <typename Iter>
        vector(Iter begin, Iter end) : m_buffer{std::move(begin), std::move(end)}, m_vec {m_buffer.data(), m_buffer.size()} {}

        struct convert_tag{};

        template <typename Iter>
        vector(convert_tag, const Iter begin, const Iter end) {
            for (auto current = begin; current != end; ++current) {
                m_buffer.push_back(static_cast<double>(*current));
            }
            m_vec = data_t{m_buffer.data(), m_buffer.size()};
        }

        bool operator==(const vector& rhs) const {
            return std::ranges::equal(m_buffer, rhs.m_buffer);
        }

        [[nodiscard]] size_t size() const { return m_vec.extent(0); }

        std::vector<double>& data() { return m_buffer; }
        [[nodiscard]] const std::vector<double>& data() const { return m_buffer; }

        data_t& span() { return m_vec; }
        [[nodiscard]] const data_t& span() const { return m_vec; }

        double& operator[](size_t i) { return m_vec[i]; }
        const double& operator[](size_t i) const { return m_vec[i]; }

        [[nodiscard]] vector append(double v) const;
        [[nodiscard]] vector subvector(size_t size) const;
        [[nodiscard]] matrix to_matrix() const;
        [[nodiscard]] matrix to_transpose_matrix() const;
        [[nodiscard]] double dot(const vector& rhs) const;
        [[nodiscard]] vector cross(const vector& rhs) const;

        [[nodiscard]] std::string to_string(int width = 8, int precision = 0) const;
    };

    inline vector operator*(const matrix& lhs, const vector& rhs) {
        const auto res_mat = lhs * rhs.to_matrix();
        return {res_mat.data().begin(), res_mat.data().end()};
    }

    void to_row_echelon(matrix& mat, double tolerance = 0.0001);
    void to_reduced_row_echelon(matrix& mat, double tolerance = 0.0001);

} // aoc

#endif //LATESTER_MATRIX_H