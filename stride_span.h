//
// Created by Daniel Garcia on 12/20/20.
//

#ifndef ADVENTOFCODE_STRIDE_SPAN_H
#define ADVENTOFCODE_STRIDE_SPAN_H

#include <iterator>

namespace aoc {

    template <typename T>
    class stride_span;

    template <typename T>
    class stride_span_iterator {
        stride_span<T>* m_span = nullptr;
        std::size_t m_index = 0;
        [[nodiscard]] bool is_end() const noexcept;

        friend class stride_span<T>;
        stride_span_iterator(stride_span<T>* s, std::size_t i) : m_span{s}, m_index{i} {}

    public:
        using difference_type = decltype(std::declval<T*>() - std::declval<T*>());
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        bool operator==(const stride_span_iterator& rhs) const { return m_index == rhs.m_index || (is_end() && is_end() == rhs.is_end()); }
        bool operator!=(const stride_span_iterator& rhs) const { return m_index != rhs.m_index && (!is_end() || is_end() != rhs.is_end()); }
        bool operator< (const stride_span_iterator& rhs) const { return m_index <  rhs.m_index; }
        bool operator<=(const stride_span_iterator& rhs) const { return m_index <= rhs.m_index; }
        bool operator> (const stride_span_iterator& rhs) const { return m_index >  rhs.m_index; }
        bool operator>=(const stride_span_iterator& rhs) const { return m_index >= rhs.m_index; }

        T& operator*();
        const T& operator*() const;

        stride_span_iterator& operator++() noexcept { ++m_index; return *this; }
        stride_span_iterator operator++(int) noexcept { auto retval = *this; ++m_index; return retval; }

        stride_span_iterator& operator--() noexcept { --m_index; return *this; }
        stride_span_iterator operator--(int) noexcept { auto retval = *this; --m_index; return retval; }

        stride_span_iterator& operator+=(int n) noexcept { m_index += n; return *this; }
        stride_span_iterator& operator-=(int n) noexcept { m_index -= n; return *this; }

        stride_span_iterator operator+(int n) const noexcept { auto retval = *this; retval += n; return retval; }
        stride_span_iterator operator-(int n) const noexcept { auto retval = *this; retval -= n; return retval; }
        T& operator[](int n) { return *(*this + n); }
        const T& operator[](int n) const { return *(*this + n); }

        difference_type operator-(const stride_span_iterator& rhs) const noexcept { if (m_span != rhs.m_span) { return 0; } return m_index - rhs.m_index; }
    };

    template <typename T>
    stride_span_iterator<T> operator+(int n, stride_span_iterator<T> it) {
        return it + n;
    }

    template <typename T>
    class const_stride_span_iterator {
        const stride_span<T>* m_span = nullptr;
        std::size_t m_index = 0;
        [[nodiscard]] bool is_end() const noexcept;

        friend class stride_span<T>;
        const_stride_span_iterator(const stride_span<T>* s, std::size_t i) : m_span{s}, m_index{i} {}

    public:
        using difference_type = decltype(std::declval<T*>() - std::declval<T*>());
        using value_type = const T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

        bool operator==(const const_stride_span_iterator& rhs) const { return m_index == rhs.m_index || (is_end() && is_end() == rhs.is_end()); }
        bool operator!=(const const_stride_span_iterator& rhs) const { return m_index != rhs.m_index && (!is_end() || is_end() != rhs.is_end()); }
        bool operator< (const const_stride_span_iterator& rhs) const { return m_index <  rhs.m_index; }
        bool operator<=(const const_stride_span_iterator& rhs) const { return m_index <= rhs.m_index; }
        bool operator> (const const_stride_span_iterator& rhs) const { return m_index >  rhs.m_index; }
        bool operator>=(const const_stride_span_iterator& rhs) const { return m_index >= rhs.m_index; }

        const T& operator*() const;

        const_stride_span_iterator& operator++() noexcept { ++m_index; return *this; }
        const_stride_span_iterator operator++(int) noexcept { auto retval = *this; ++m_index; return retval; }

        const_stride_span_iterator& operator--() noexcept { --m_index; return *this; }
        const_stride_span_iterator operator--(int) noexcept { auto retval = *this; --m_index; return retval; }

        const_stride_span_iterator& operator+=(int n) noexcept { m_index += n; return *this; }
        const_stride_span_iterator& operator-=(int n) noexcept { m_index -= n; return *this; }

        const_stride_span_iterator operator+(int n) const noexcept { auto retval = *this; retval += n; return *this; }
        const_stride_span_iterator operator-(int n) const noexcept { auto retval = *this; retval -= n; return *this; }
        const T& operator[](int n) const { return *(*this + n); }

        difference_type operator-(const const_stride_span_iterator& rhs) const noexcept { if (m_span != rhs.m_span) { return 0; } return m_index - rhs.m_index; }
    };

    template <typename T>
    class stride_span {
        T* m_begin;
        T* m_end;
        std::size_t m_offset = 0;
        std::ptrdiff_t m_stride = 1;
        friend class stride_span_iterator<T>;
        friend class const_stride_span_iterator<T>;

        stride_span(T* b, T* e, std::size_t off, std::ptrdiff_t str) : m_begin{b}, m_end{e}, m_offset{off}, m_stride{str} {}
    public:
        stride_span() = default;

        enum class length : std::size_t {};

        template <typename Container>
        stride_span(Container& c, std::size_t offset = 0, std::ptrdiff_t stride = 1) noexcept :
            m_begin{std::data(c)}, m_end{std::data(c) + std::size(c)}, m_offset{offset}, m_stride{stride} {}

        template <typename Container>
        stride_span(Container& c, length len, std::size_t offset = 0, std::ptrdiff_t stride = 1) noexcept :
                m_begin{std::data(c)}, m_end{std::data(c) + static_cast<std::size_t>(len) * stride}, m_offset{offset}, m_stride{stride} {}

        template <typename Container, typename = std::enable_if_t<Container::is_view, void>>
        stride_span(Container&& c, std::size_t offset = 0, std::ptrdiff_t stride = 1) noexcept :
            m_begin{std::data(c)}, m_end{std::data(c) + std::size(c)}, m_offset{offset}, m_stride{stride} {}

        [[nodiscard]] bool empty() const {
            return m_begin == nullptr ||
                   m_stride >= 0 ? (m_end - m_begin < m_stride) : (m_begin - m_end < -m_stride);
        }
        [[nodiscard]] std::size_t size() const {
            return m_stride >= 0 ?
                   static_cast<std::size_t>((std::distance(m_begin, m_end) + m_stride - 1) / m_stride) :
                   static_cast<std::size_t>((std::distance(m_end, m_begin) - m_stride - 1) / -m_stride);
        }

        stride_span<T> reverse() const {
            return stride_span{m_end, m_begin, -m_stride + m_offset, -m_stride};
        }

        T& operator[](std::size_t index) { return *(m_begin + m_offset + m_stride * index); }
        const T& operator[](std::size_t index) const { return *(m_begin + m_offset + m_stride * index); }

        stride_span<T> sub_span(std::size_t offset = 0, std::size_t length = 0) const {
            const auto sz = size();
            if (offset >= sz) {
                throw std::runtime_error{"Offset past end of span."};
            }
            else if (offset + length >= sz || length == 0) {
                length = sz - offset;
            }
            return stride_span{ m_begin + m_stride * offset, m_begin + m_stride * (offset + length), m_offset, m_stride };
        }

        [[nodiscard]] stride_span_iterator<T> begin() { return {this, 0}; }
        [[nodiscard]] stride_span_iterator<T> end() { return {this, size()}; }
        [[nodiscard]] const_stride_span_iterator<T> begin() const { return {this, 0}; }
        [[nodiscard]] const_stride_span_iterator<T> end() const { return {this, size()}; }
        [[nodiscard]] const_stride_span_iterator<T> cbegin() { return {this, 0}; }
        [[nodiscard]] const_stride_span_iterator<T> cend() { return {this, size()}; }
    };

    template <typename T>
    bool stride_span_iterator<T>::is_end() const noexcept { return m_span == nullptr || m_index >= m_span->size(); }

    template <typename T>
    T& stride_span_iterator<T>::operator*() {
        return *(m_span->m_begin + m_span->m_offset + m_index * m_span->m_stride);
    }

    template <typename T>
    const T& stride_span_iterator<T>::operator*() const {
        return *(m_span->m_begin + m_span->m_offset + m_index * m_span->m_stride);
    }

    template <typename T>
    bool const_stride_span_iterator<T>::is_end() const noexcept { return m_span == nullptr || m_index >= m_span->size(); }

    template <typename T>
    const T& const_stride_span_iterator<T>::operator*() const {
        return *(m_span->m_begin + m_span->m_offset + m_index * m_span->m_stride);
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_STRIDE_SPAN_H
