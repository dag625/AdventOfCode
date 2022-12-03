//
// Created by Daniel Garcia on 12/2/2022.
//

#ifndef ADVENTOFCODE_HEAP_ARRAY_H
#define ADVENTOFCODE_HEAP_ARRAY_H

#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <array>

namespace aoc {

    /**
     * Intended to be a drop in replacement for std::array where the array is allocated on the heap
     * instead of the stack.  This is not exactly a drop in replacement since std::array can be created
     * uninitialized and such.
     * @tparam T The type of object of the elements of the array.
     * @tparam Size The size of the array.
     */
    template <class T, size_t Size>
    class heap_array {
        T* m_elements;

    public:
        using value_type      = T;
        using size_type       = size_t;
        using difference_type = std::ptrdiff_t;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;

        using iterator       = T*;
        using const_iterator = const T*;

        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr heap_array() : m_elements{new T[Size]} {}
        constexpr heap_array(std::initializer_list<T> vals) : m_elements{new T[Size]{vals}} {}
        constexpr heap_array(const heap_array& other) : m_elements{new T[Size]} { std::copy_n(other.begin(), Size, begin()); }
        constexpr heap_array(const std::array<T, Size>& other) : m_elements{new T[Size]} { std::copy_n(other.begin(), Size, begin()); }
        constexpr heap_array(heap_array&& other) noexcept : m_elements{new T[Size]} { std::swap(m_elements, other.m_elements); }
        constexpr ~heap_array() { delete[] m_elements; }

        constexpr heap_array& operator=(heap_array other) noexcept
        {
            swap(other);
            return *this;
        }
        constexpr heap_array& operator=(const std::array<T, Size>& other) noexcept
        {
            std::copy_n(other.begin(), Size, begin());
            return *this;
        }

        constexpr void fill(const T& value) {
            std::fill_n(m_elements, Size, value);
        }

        constexpr void swap(heap_array& other) noexcept {
            std::swap(m_elements, other.m_elements);
        }

        [[nodiscard]] constexpr iterator begin() noexcept {
            return m_elements;
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept {
            return m_elements;
        }

        [[nodiscard]] constexpr iterator end() noexcept {
            return m_elements + Size;
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept {
            return m_elements + Size;
        }

        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        [[nodiscard]] constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
            return begin();
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept {
            return end();
        }

        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
            return rend();
        }

        [[nodiscard]] constexpr size_type size() const noexcept {
            return Size;
        }

        [[nodiscard]] constexpr size_type max_size() const noexcept {
            return Size;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return false;
        }

        [[nodiscard]] constexpr reference at(size_type pos) {
            if (Size <= pos) {
                throw_out_of_range();
            }

            return m_elements[pos];
        }

        [[nodiscard]] constexpr const_reference at(size_type pos) const {
            if (Size <= pos) {
                throw_out_of_range();
            }

            return m_elements[pos];
        }

        [[nodiscard]] constexpr reference operator[](_In_range_(0, Size - 1) size_type pos) noexcept {
            return m_elements[pos];
        }

        [[nodiscard]] constexpr const_reference operator[](_In_range_(0, Size - 1) size_type pos) const noexcept {
            return m_elements[pos];
        }

        [[nodiscard]] constexpr reference front() noexcept {
            return m_elements[0];
        }

        [[nodiscard]] constexpr const_reference front() const noexcept {
            return m_elements[0];
        }

        [[nodiscard]] constexpr reference back() noexcept {
            return m_elements[Size - 1];
        }

        [[nodiscard]] constexpr const_reference back() const noexcept {
            return m_elements[Size - 1];
        }

        [[nodiscard]] constexpr T* data() noexcept {
            return m_elements;
        }

        [[nodiscard]] constexpr const T* data() const noexcept {
            return m_elements;
        }

    private:
        [[noreturn]] void throw_out_of_range() const {
            throw std::out_of_range{"Invalid heap_array<T, N> subscript"};
        }
    };

} /* namespace aoc */

#endif //ADVENTOFCODE_HEAP_ARRAY_H
