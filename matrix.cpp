//
// Created by Daniel Garcia on 12/12/2025.
//

#include "matrix.h"

namespace aoc {

    matrix::matrix(std::initializer_list<std::initializer_list<double>> list) {
        const auto num_rows = list.size();
        if (num_rows > 0) {
            const auto num_cols = list.begin()->size();
            if (!std::all_of(list.begin(), list.end(), [num_cols](const auto& l) { return l.size() == num_cols; })) {
                throw matrix_exception{std::format("Inconsistent number of columns not equal to 1st column size {}.", num_cols)};
            }
            m_buffer.reserve(num_rows * num_cols);
            for (const auto& l : list) {
                m_buffer.append_range(l);
            }
            m_mat = data_t{m_buffer.data(), num_rows, num_cols};
        }
    }

    void matrix::check_same_size(const matrix& other) const {
        if (!is_same_size(other)) {
            throw matrix_exception{std::format("Matricies have different sizes [{},{}] vs [{},{}].",
                num_rows(), num_cols(), other.num_rows(), other.num_cols())};
        }
    }

    matrix matrix::submatrix(const size_t rows, const size_t cols) const {
        if (rows > num_rows()) {
            throw matrix_exception{std::format("Cannot create a submatrix because the requested number of rows {} is greater than the matrix's number of rows {}.", rows, num_rows())};
        }
        if (cols > num_cols()) {
            throw matrix_exception{std::format("Cannot create a submatrix because the requested number of columns {} is greater than the matrix's number of columns {}.", cols, num_cols())};
        }
        matrix retval {rows, cols};
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                retval.m_mat[r, c] = m_mat[r, c];
            }
        }
        return retval;
    }

    matrix::row_t matrix::row_span(size_t row) {
        std::dextents<size_t, 1> shape{num_cols()};
        std::array<size_t, 1> strides{m_mat.mapping().stride(1)};
        return {m_buffer.data() + row * num_cols(), std::layout_stride::mapping{shape, strides}};
    }
    matrix::const_row_t matrix::row_cspan(size_t row) {
        std::dextents<size_t, 1> shape{num_cols()};
        std::array<size_t, 1> strides{m_mat.mapping().stride(1)};
        return {m_buffer.data() + row * num_cols(), std::layout_stride::mapping{shape, strides}};
    }
    matrix::const_row_t matrix::row_span(size_t row) const {
        std::dextents<size_t, 1> shape{num_cols()};
        std::array<size_t, 1> strides{m_mat.mapping().stride(1)};
        return {m_buffer.data() + row * num_cols(), std::layout_stride::mapping{shape, strides}};
    }
    matrix::const_row_t matrix::row_cspan(size_t row) const {
        return row_span(row);
    }
    std::vector<double> matrix::row_data(size_t row) const {
        const auto span = row_span(row);
        std::vector<double> retval;
        retval.reserve(span.size());
        for (size_t i = 0; i < span.size(); ++i) {
            retval.emplace_back(span[i]);
        }
        return retval;
    }
    vector matrix::row_vector(size_t row) const {
        const auto span = row_span(row);
        vector retval (span.size());
        for (size_t i = 0; i < span.size(); ++i) {
            retval[i] = span[i];
        }
        return retval;
    }

    matrix::col_t matrix::col_span(size_t col) {
        std::dextents<size_t, 1> shape{num_rows()};
        std::array<size_t, 1> strides{m_mat.mapping().stride(0)};
        return {m_buffer.data() + col, std::layout_stride::mapping{shape, strides}};
    }
    matrix::const_col_t matrix::col_cspan(size_t col) {
        std::dextents<size_t, 1> shape{num_rows()};
        std::array<size_t, 1> strides{m_mat.mapping().stride(0)};
        return {m_buffer.data() + col, std::layout_stride::mapping{shape, strides}};
    }
    matrix::const_col_t matrix::col_span(size_t col) const {
        std::dextents<size_t, 1> shape{num_rows()};
        std::array<size_t, 1> strides{m_mat.mapping().stride(0)};
        return {m_buffer.data() + col, std::layout_stride::mapping{shape, strides}};
    }
    matrix::const_col_t matrix::col_cspan(size_t col) const {
        return col_span(col);
    }
    std::vector<double> matrix::col_data(size_t col) const {
        const auto span = col_span(col);
        std::vector<double> retval;
        retval.reserve(span.size());
        for (size_t i = 0; i < span.size(); ++i) {
            retval.emplace_back(span[i]);
        }
        return retval;
    }
    vector matrix::col_vector(size_t row) const {
        const auto span = col_span(row);
        vector retval (span.size());
        for (size_t i = 0; i < span.size(); ++i) {
            retval[i] = span[i];
        }
        return retval;
    }

    void matrix::round_all() {
        for (auto& v : m_buffer) {
            v = std::round(v);
        }
    }

    void matrix::set_zeros(double tolerance) {
        for (auto& v : m_buffer) {
            if (std::abs(v) < tolerance) { v = 0.0; }
        }
    }

    matrix matrix::identity(size_t size) {
        matrix retval{size};
        for (size_t i = 0; i < size; ++i) {
            retval[i,i] = 1;
        }
        return retval;
    }

    matrix& matrix::to_transpose() {
        using std::swap;
        for (size_t r = 0; r < num_rows(); ++r) {
            for (size_t c = 0; c < num_cols(); ++c) {
                swap(m_mat[r, c], m_mat[c, r]);
            }
        }
        return *this;
    }
    matrix matrix::transpose() const {
        auto retval = *this;
        return retval.to_transpose();
    }

    matrix matrix::adjoin(const matrix& rhs) const {
        if (num_rows() != rhs.num_rows()) {
            throw std::logic_error{std::format("Cannot adjoin two matrices with {} and {} rows.  The row count must be the same.", num_rows(), rhs.num_rows())};
        }
        const auto rows = num_rows();
        const auto cols_offset = num_cols();
        matrix retval {rows, num_cols() + rhs.num_cols()};
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < num_cols(); ++c) {
                retval[r, c] = m_mat[r, c];
            }
        }
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < rhs.num_cols(); ++c) {
                retval[r, cols_offset + c] = rhs.m_mat[r, c];
            }
        }
        return retval;
    }

    matrix matrix::adjoin_below(const matrix& rhs) const {
        if (num_cols() != rhs.num_cols()) {
            throw std::logic_error{std::format("Cannot adjoin below two matrices with {} and {} columns.  The column count must be the same.", num_cols(), rhs.num_cols())};
        }
        const auto cols = num_cols();
        const auto rows_offset = num_rows();
        matrix retval {num_rows() + rhs.num_rows(), cols};
        for (size_t r = 0; r < num_rows(); ++r) {
            for (size_t c = 0; c < cols; ++c) {
                retval[r, c] = m_mat[r, c];
            }
        }
        for (size_t r = 0; r < rhs.num_rows(); ++r) {
            for (size_t c = 0; c < cols; ++c) {
                retval[rows_offset + r, c] = rhs.m_mat[r, c];
            }
        }
        return retval;
    }

    matrix& matrix::to_inverse() {
        if (num_rows() != num_cols()) {
            throw std::logic_error{std::format("Cannot invert matrix of size [{},{}] because the inverse is only defined for square matrices.", num_rows(), num_cols())};
        }
        using std::swap;
        auto tmp = adjoin(identity(num_rows()));
        //todo to reduced row echelon
        //todo from latter num_cols()
        swap(m_buffer, tmp.m_buffer);
        swap(m_mat, tmp.m_mat);
        return *this;
    }
    matrix matrix::inverse() const {
        auto retval = *this;
        return retval.to_inverse();
    }

    matrix& matrix::to_row_echelon(double tolerance) {
        ::aoc::to_row_echelon(*this, tolerance);
        return *this;
    }
    matrix matrix::row_echelon(double tolerance) const {
        auto retval = *this;
        return retval.to_row_echelon(tolerance);
    }
    matrix& matrix::to_reduced_row_echelon(double tolerance) {
        ::aoc::to_reduced_row_echelon(*this, tolerance);
        return *this;
    }
    matrix matrix::reduced_row_echelon(double tolerance) const {
        auto retval = *this;
        return retval.to_reduced_row_echelon(tolerance);
    }

    matrix matrix::operator+(const matrix& rhs) const {
        check_same_size(rhs);
        matrix retval {num_rows(), num_cols()};
        for (int i = 0; i < m_buffer.size(); ++i) {
            retval.m_buffer[i] = m_buffer[i] + rhs.m_buffer[i];
        }
        return retval;
    }

    matrix matrix::operator-(const matrix& rhs) const {
        check_same_size(rhs);
        matrix retval {num_rows(), num_cols()};
        for (int i = 0; i < m_buffer.size(); ++i) {
            retval.m_buffer[i] = m_buffer[i] - rhs.m_buffer[i];
        }
        return retval;
    }

    matrix matrix::operator*(const matrix& rhs) const {
        if (num_cols() != rhs.num_rows()) {
            throw matrix_exception{std::format("Cannot multiple matrix sized [{},{}] by one sized [{},{}].",
                num_rows(), num_cols(), rhs.num_rows(), rhs.num_cols())};
        }
        matrix retval {num_rows(), rhs.num_cols()};
        for (size_t r = 0; r < num_rows(); ++r) {
            for (size_t c = 0; c < rhs.num_cols(); ++c) {
                for (size_t n = 0; n < rhs.num_rows(); ++n) {
                    retval.m_mat[r, c] += m_mat[r, n] * rhs.m_mat[n, c];
                }
            }
        }
        return retval;
    }

    std::string matrix::to_string(int width, int precision) const {
        std::string retval;
        for (size_t r = 0; r < num_rows(); ++r) {
            std::string_view sp;
            for (size_t c = 0; c < num_cols(); ++c) {
                retval += std::format("{}{:{}.{}f}", sp, m_mat[r, c], width, precision);
                sp = " ";
            }
            retval.push_back('\n');
        }
        return retval;
    }

    // ------------------------------ Vector ------------------------------

    vector vector::append(double v) const {
        auto retval = *this;
        retval.m_buffer.push_back(v);
        retval.m_vec = data_t{retval.m_buffer.data(), retval.m_buffer.size()};
        return retval;
    }

    vector vector::subvector(size_t size) const {
        return {m_buffer.begin(), m_buffer.begin() + static_cast<std::vector<double>::difference_type>(std::min(size, m_buffer.size()))};
    }

    matrix vector::to_matrix() const {
        matrix retval {m_buffer.size(), 1};
        std::copy(m_buffer.begin(), m_buffer.end(), retval.data().begin());
        return retval;
    }

    matrix vector::to_transpose_matrix() const {
        matrix retval {1, m_buffer.size()};
        std::copy(m_buffer.begin(), m_buffer.end(), retval.data().begin());
        return retval;
    }

    double vector::dot(const vector& rhs) const {
        check_same_size(rhs);
        double retval = 0.0;
        for (size_t i = 0; i < m_buffer.size(); ++i) {
            retval += m_buffer[i] * rhs.m_buffer[i];
        }
        return retval;
    }

    vector vector::cross(const vector& rhs) const {
        check_same_size(rhs);
        if (size() != 3) {
            throw matrix_exception{std::format("Vector cross product is only valid in 3 dimensions, but the vectors have size {}.", size())};
        }
        vector retval(3);
        retval[0] = m_buffer[1] * rhs.m_buffer[2] - m_buffer[2] * rhs.m_buffer[1];
        retval[1] = m_buffer[0] * rhs.m_buffer[2] - m_buffer[2] * rhs.m_buffer[0];
        retval[2] = m_buffer[0] * rhs.m_buffer[1] - m_buffer[1] * rhs.m_buffer[0];
        return retval;
    }

    std::string vector::to_string(int width, int precision) const {
        std::string retval;
        std::string_view sp;
        for (size_t idx = 0; idx < size(); ++idx) {
            retval += std::format("{}{:{}.{}f}", sp, m_vec[idx], width, precision);
            sp = " ";
        }
        return retval;
    }

    // -------------------------- Free Functions --------------------------

    size_t max_row_idx(const matrix& mat, const size_t begin_row, const size_t end_row, const size_t col) {
        size_t retval = begin_row;
        double max = 0.0;
        for (size_t r = begin_row; r < end_row; ++r) {
            const auto abs = std::abs(mat[r, col]);
            if (abs > max) {
                max = abs;
                retval = r;
            }
        }
        return retval;
    }

    void to_row_echelon(matrix& mat, double tolerance) {
        size_t h = 0; //Row
        size_t k = 0; //Column
        const auto M = mat.num_rows();
        const auto N = mat.num_cols();
        while (h < M && k < N) {
            const auto i_max = max_row_idx(mat, h, M, k);
            if (mat[i_max, k] == 0.0) {
                ++k;
                continue;
            }

            mat.swap_rows(h, i_max);
            for (auto i = h + 1; i < M; ++i) {
                const auto f = mat[i, k] / mat[h, k];
                mat[i, k] = 0;
                for (auto j = k + 1; j < N; ++j) {
                    mat[i, j] = mat[i, j] - mat[h, j] * f;
                }
            }
            mat.set_zeros(tolerance);

            ++h;
            ++k;
        }
    }

    void to_reduced_row_echelon(matrix& mat, double tolerance) {
        size_t h = 0; //Row
        size_t k = 0; //Column
        const auto M = mat.num_rows();
        const auto N = mat.num_cols();
        while (h < M && k < N) {
            //std::println(std::cout, "{}", mat.to_string(6, 3));std::cout.flush();
            const auto i_max = max_row_idx(mat, h, M, k);
            if (mat[i_max, k] == 0.0) {
                ++k;
                continue;
            }

            mat.swap_rows(h, i_max);
            const auto f_c = 1.0 / mat[h, k];
            mat[h, k] = 1.0;
            for (auto j = k + 1; j < N; ++j) {
                mat[h, j] *= f_c;
            }
            for (auto i = 0; i < M; ++i) {
                if (i == h) { continue; }
                const auto f = mat[i, k] / mat[h, k];
                mat[i, k] = 0;
                for (auto j = k + 1; j < N; ++j) {
                    mat[i, j] = mat[i, j] - mat[h, j] * f;
                }
            }
            mat.set_zeros(tolerance);

            ++h;
            ++k;
        }
    }

} /* namespace aoc */