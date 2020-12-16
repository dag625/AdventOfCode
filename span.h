//
// Created by Daniel Garcia on 12/15/20.
//

#ifndef ADVENTOFCODE_SPAN_H
#define ADVENTOFCODE_SPAN_H

#include <iterator>

namespace aoc {

    template <typename T>
    class span {
        T* m_begin = nullptr;
        T* m_end = nullptr;
    public:
        span() = default;

        template <typename Container>
        span(Container& c) : m_begin{std::data(c)}, m_end{std::data(c) + std::size(c)} {}

        [[nodiscard]] bool empty() const { return m_begin == nullptr || m_end - m_begin == 0; }
        [[nodiscard]] std::size_t size() const { return static_cast<std::size_t>(m_end - m_begin); }
        [[nodiscard]] T* data() { return m_begin; }
        [[nodiscard]] const T* data() const { return m_begin; }

        [[nodiscard]] T* begin() { return m_begin; }
        [[nodiscard]] T* end() { return m_end; }
        [[nodiscard]] const T* begin() const { return m_begin; }
        [[nodiscard]] const T* end() const { return m_end; }
        [[nodiscard]] const T* cbegin() { return m_begin; }
        [[nodiscard]] const T* cend() { return m_end; }
    };

} /* namespace aoc */

#endif //ADVENTOFCODE_SPAN_H
