//
// Created by Daniel Garcia on 10/31/2025.
//

#ifndef ADVENTOFCODE_CIRCULAR_LIST_H
#define ADVENTOFCODE_CIRCULAR_LIST_H

#include <iterator>
#include <memory>
#include <vector>
#include <ranges>
#include <stdexcept>

namespace aoc {

    template <typename T>
    class circular_list_iterator;

    template <typename T>
    class circular_list_const_iterator;

    template <typename T>
    class circular_list {
    public:
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using iterator = circular_list_iterator<T>;
        using const_iterator = circular_list_const_iterator<T>;
        using difference_type = std::ptrdiff_t;//This should be the same as the difference_type for both iterators.
        using size_type = std::size_t;

    private:
        friend class circular_list_iterator<T>;
        friend class circular_list_const_iterator<T>;

        struct node {
            T data;
            node* next = nullptr;
            node* prev = nullptr;

            node() = default;
            node(const node&) requires std::is_copy_constructible_v<T> = default;
            node(node&&) noexcept requires std::is_move_constructible_v<T> = default;

            template <typename ...Args>
            node(Args&&... a) : data{std::forward<Args>(a)...} {}

            node& operator=(const node&) requires std::is_copy_assignable_v<T> = default;
            node& operator=(node&&) noexcept requires std::is_move_assignable_v<T> = default;
        };
        struct end_tag{};

        node* m_head = nullptr;
        size_type m_size = 0;

        iterator insert(const_iterator pos, std::unique_ptr<node>& first_node,
            std::unique_ptr<node>& last_node, size_type count);

    public:
        circular_list() = default;
        circular_list(const circular_list& other) : m_size{other.m_size} {
            if (m_size > 0) {
                //This isn't efficient (e.g., two loops), but it should be exception safe.
                std::vector<std::unique_ptr<node>> tmp;
                node* last = nullptr;
                for (const auto& on : other) {
                    tmp.push_back(std::make_unique<node>(on));
                    if (last) [[likely]] {
                        last->next = tmp.back().get();
                        tmp.back()->prev = last;
                    }
                    last = tmp.back().get();
                }
                m_head = tmp.front().get();
                tmp.front()->prev = tmp.back().get();
                tmp.back()->next = tmp.front().get();
                for (auto& p : tmp) {
                    p.release();
                }
            }
        }
        circular_list(circular_list&& other) noexcept : m_head{other.m_head}, m_size {other.m_size} {
            other.m_head = nullptr;
            other.m_size = 0;
        }
        template<std::input_or_output_iterator Iter>
        circular_list(Iter first, Iter last) {
            if (first != last) {
                //This isn't efficient (e.g., two loops), but it should be exception safe.
                std::vector<std::unique_ptr<node>> tmp;
                node* prev = nullptr;
                size_type size = 0;
                for (auto it = first; it != last; ++it) {
                    ++size;
                    tmp.push_back(std::make_unique<node>(std::move(*it)));
                    if (prev) [[likely]] {
                        prev->next = tmp.back().get();
                        tmp.back()->prev = prev;
                    }
                    prev = tmp.back().get();
                }
                m_head = tmp.front().get();
                m_size = size;
                tmp.front()->prev = tmp.back().get();
                tmp.back()->next = tmp.front().get();
                for (auto& p : tmp) {
                    p.release();
                }
            }
        }
        circular_list(std::initializer_list<T> list) : m_size{list.size()} {
            if (m_size > 0) {
                //This isn't efficient (e.g., two loops), but it should be exception safe.
                std::vector<std::unique_ptr<node>> tmp;
                node* last = nullptr;
                for (auto& li : list) {
                    tmp.push_back(std::make_unique<node>(std::move(li)));
                    if (last) [[likely]] {
                        last->next = tmp.back().get();
                        tmp.back()->prev = last;
                    }
                    last = tmp.back().get();
                }
                m_head = tmp.front().get();
                tmp.front()->prev = tmp.back().get();
                tmp.back()->next = tmp.front().get();
                for (auto& p : tmp) {
                    p.release();
                }
            }
        }
        ~circular_list() {
            clear();
        }

        circular_list& operator=(circular_list other) noexcept {
            swap(other);
            return *this;
        }

        void swap(circular_list& other) noexcept {
            using std::swap;
            swap(m_head, other.m_head);
            swap(m_size, other.m_size);
        }

        friend void swap(circular_list& a, circular_list& b) noexcept {
            a.swap(b);
        }

        //Assignment
        void assign(size_type count, const T& value) {
            circular_list tmp;
            for (size_type i = 0; i < count; ++i) {
                tmp.push_back(value);
            }
            swap(tmp);
        }

        template<std::input_iterator Iter>
        void assign(Iter first, Iter last) {
            circular_list tmp{first, last};
            swap(tmp);
        }

        void assign(std::initializer_list<T> ilist) {
            assign(ilist.begin(), ilist.end());
        }

        template<typename R>
        void assign_range(R&& rg) requires std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T> {
            assign(std::begin(rg), std::end(rg));
        }

        //Iterators
        [[nodiscard]] iterator begin() { return iterator{m_head, this}; }
        [[nodiscard]] iterator end() { return iterator{end_tag{}, m_head, this}; }
        [[nodiscard]] const_iterator begin() const { return const_iterator{m_head, this}; }
        [[nodiscard]] const_iterator end() const { return const_iterator{end_tag{}, m_head, this}; }
        [[nodiscard]] const_iterator cbegin() { return const_iterator{m_head, this}; }
        [[nodiscard]] const_iterator cend() { return const_iterator{end_tag{}, m_head, this}; }
        //Reverse Iterators (Hopefully)
        [[nodiscard]] std::reverse_iterator<iterator> rbegin() { return std::reverse_iterator<iterator>{iterator{m_head, this}}; }
        [[nodiscard]] std::reverse_iterator<iterator> rend() { return std::reverse_iterator<iterator>{iterator{end_tag{}, m_head, this}}; }
        [[nodiscard]] std::reverse_iterator<const_iterator> rbegin() const { return std::reverse_iterator<const_iterator>{const_iterator{m_head, this}}; }
        [[nodiscard]] std::reverse_iterator<const_iterator> rend() const { return std::reverse_iterator<const_iterator>{const_iterator{end_tag{}, m_head, this}}; }
        [[nodiscard]] std::reverse_iterator<const_iterator> crbegin() { return std::reverse_iterator<const_iterator>{const_iterator{m_head, this}}; }
        [[nodiscard]] std::reverse_iterator<const_iterator> crend() { return std::reverse_iterator<const_iterator>{const_iterator{end_tag{}, m_head, this}}; }

        //Access
        reference front() { return m_head->data; }
        const_reference front() const { return m_head->data; }
        reference back() { return m_head->prev->data; }
        const_reference back() const { return m_head->prev->data; }

        //Capacity
        [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
        [[nodiscard]] size_type size() const noexcept { return m_size; }
        [[nodiscard]] size_type max_size() const noexcept { return std::numeric_limits<difference_type>::max() / sizeof(node); }
        void resize(const size_type count) {
            resize(count, T{});
        }
        void resize(const size_type count, const value_type& value) {
            if (count == m_size) { return; }
            if (count > m_size) {
                for (size_type i = m_size; i < count; ++i) {
                    push_back(value);
                }
            }
            else {
                for (size_type i = m_size; i < count; ++i) {
                    pop_back();
                }
            }
        }

        //Modifiers
        void clear() {
            for (std::size_t i = 0; i < m_size; ++i) {
                auto* del = m_head;
                m_head = del->next;
                delete del;
            }
            m_head = nullptr;
            m_size = 0;
        }
        iterator insert(const_iterator pos, const T& value);
        iterator insert(const_iterator pos, T&& value);
        iterator insert(const_iterator pos,
                         size_type count, const T& value);
        template<std::input_iterator Iter>
        iterator insert(const_iterator pos, Iter first, Iter last);
        iterator insert(const_iterator pos, std::initializer_list<T> ilist);
        template<typename R>
        iterator insert_range(const_iterator pos, R&& rg) requires std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;
        template<class... Args>
        iterator emplace(const_iterator pos, Args&&... args);
        iterator erase(const_iterator pos);
        iterator erase(const_iterator first, const_iterator last);
        void push_front(const T& value) { insert(cbegin(), value); }
        void push_front(T&& value) { insert(cbegin(), std::move(value)); }
        template<typename R>
        void prepend_range(R&& rg) requires std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T> { insert_range(cbegin(), std::forward<R>(rg)); }
        template< class... Args >
        reference emplace_front(Args&&... args) { emplace(begin(), std::forward<Args>(args)...); return front(); }
        void pop_front() { erase(begin()); }
        void push_back(const T& value) { insert(cend(), value); }
        void push_back(T&& value) { insert(cend(), std::move(value)); }
        template<typename R>
        void append_range(R&& rg) requires std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T> { insert_range(cend(), std::forward<R>(rg)); }
        template< class... Args >
        reference emplace_back(Args&&... args) { emplace(cend(), std::forward<Args>(args)...); return front(); }
        void pop_back() { erase(end()); }
    };

    template <typename T>
    class circular_list_iterator {
    protected:
        using node_t = circular_list<T>::node;

        node_t* m_ptr = nullptr;
        const circular_list<T>* m_parent = nullptr;
        bool m_passed_end = false;

        explicit circular_list_iterator(node_t* ptr, const circular_list<T>* parent) :
                m_ptr{ptr}, m_parent{parent} {}
        explicit circular_list_iterator(circular_list<T>::end_tag, node_t* ptr, const circular_list<T>* parent) :
                m_ptr{ptr}, m_parent{parent}, m_passed_end{true} {}

        friend class circular_list<T>;
    public:
        //Standard types
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_cvref_t<T>;
        using pointer = T*;
        using reference	= T&;
        using iterator_category	= std::bidirectional_iterator_tag;
        using iterator_concept = std::bidirectional_iterator_tag;

        //Custom types
        using const_reference = const T&;
        using const_pointer = const T*;

        circular_list_iterator() noexcept = default;

        void swap(circular_list_iterator& other) noexcept {
            using std::swap;
            swap(m_ptr, other.m_ptr);
            swap(m_parent, other.m_parent);
            swap(m_passed_end, other.m_passed_end);
        }

        friend void swap(circular_list_iterator& a, circular_list_iterator& b) noexcept {
            a.swap(b);
        }

        bool operator==(const circular_list_iterator& other) const noexcept {
            return m_ptr == other.m_ptr &&
                m_parent == other.m_parent &&
                m_passed_end == other.m_passed_end;
        }

        //Dereference
        reference operator*() noexcept { return m_ptr->data; }
        reference operator*() const noexcept { return m_ptr->data; }
        pointer operator->() noexcept { return &m_ptr->data; }
        const_pointer operator->() const noexcept { return &m_ptr->data; }

        //Increment and decrement
        circular_list_iterator& operator++() noexcept {
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->next == m_parent->m_head;
            m_ptr = m_ptr->next;
            return *this;
        }
        circular_list_iterator operator++(int) noexcept {
            auto save = *this;
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->next == m_parent->m_head;
            m_ptr = m_ptr->next;
            return save;
        }

        circular_list_iterator& operator--() noexcept {
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->prev == m_parent->m_head;
            m_ptr = m_ptr->prev;
            return *this;
        }
        circular_list_iterator operator--(int) noexcept {
            auto save = *this;
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->prev == m_parent->m_head;
            m_ptr = m_ptr->prev;
            return save;
        }

        //Not required for bidirectional iterator, but nice to have.
        circular_list_iterator& operator+=(const difference_type dist) noexcept {
            for (difference_type i = 0; i < dist; ++i) { ++*this; }
            return *this;
        }
        circular_list_iterator& operator-=(const difference_type dist) noexcept {
            for (difference_type i = 0; i < dist; ++i) { --*this; }
            return *this;
        }
    };

    template <typename T>
    circular_list_iterator<T> operator+(circular_list_iterator<T> it, const typename circular_list_iterator<T>::difference_type dist) noexcept {
        it += dist;
        return it;
    }

    template <typename T>
    circular_list_iterator<T> operator-(circular_list_iterator<T> it, const typename circular_list_iterator<T>::difference_type dist) noexcept {
        it -= dist;
        return it;
    }

    template <typename T>
    class circular_list_const_iterator {
    protected:
        using node_t = circular_list<T>::node;

        node_t* m_ptr = nullptr;
        const circular_list<T>* m_parent = nullptr;
        bool m_passed_end = false;

        explicit circular_list_const_iterator(node_t* ptr, const circular_list<T>* parent) :
                m_ptr{ptr}, m_parent{parent} {}
        explicit circular_list_const_iterator(circular_list<T>::end_tag, node_t* ptr, const circular_list<T>* parent) :
                m_ptr{ptr}, m_parent{parent}, m_passed_end{true} {}
        explicit circular_list_const_iterator(const circular_list_iterator<T>& non_const) :
                m_ptr{non_const.m_ptr}, m_parent{non_const.m_parent}, m_passed_end {non_const.m_passed_end} {}

        friend class circular_list<T>;
    public:
        //Standard types
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_cvref_t<T>;
        using pointer = const T*;
        using reference	= const T&;
        using iterator_category	= std::bidirectional_iterator_tag;
        using iterator_concept = std::bidirectional_iterator_tag;

        //Custom types
        using const_reference = const T&;
        using const_pointer = const T*;

        circular_list_const_iterator() noexcept = default;

        void swap(circular_list_const_iterator& other) noexcept {
            using std::swap;
            swap(m_ptr, other.m_ptr);
            swap(m_parent, other.m_parent);
            swap(m_passed_end, other.m_passed_end);
        }

        friend void swap(circular_list_const_iterator& a, circular_list_const_iterator& b) noexcept {
            a.swap(b);
        }

        bool operator==(const circular_list_const_iterator& other) const noexcept {
            return m_ptr == other.m_ptr &&
                m_parent == other.m_parent &&
                m_passed_end == other.m_passed_end;
        }

        //Dereference
        reference operator*() noexcept { return m_ptr->data; }
        const_reference operator*() const noexcept { return m_ptr->data; }
        pointer operator->() noexcept { return &m_ptr->data; }
        const_pointer operator->() const noexcept { return &m_ptr->data; }

        //Increment and decrement
        circular_list_const_iterator& operator++() noexcept {
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->next == m_parent->m_head;
            m_ptr = m_ptr->next;
            return *this;
        }
        circular_list_const_iterator operator++(int) noexcept {
            auto save = *this;
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->next == m_parent->m_head;
            m_ptr = m_ptr->next;
            return save;
        }

        circular_list_const_iterator& operator--() noexcept {
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->prev == m_parent->m_head;
            m_ptr = m_ptr->prev;
            return *this;
        }
        circular_list_const_iterator operator--(int) noexcept {
            auto save = *this;
            m_passed_end = m_ptr != m_parent->m_head && m_ptr->prev == m_parent->m_head;
            m_ptr = m_ptr->prev;
            return save;
        }

        //Not required for bidirectional iterator, but nice to have.
        circular_list_const_iterator& operator+=(const difference_type dist) noexcept {
            for (difference_type i = 0; i < dist; ++i) { ++*this; }
            return *this;
        }
        circular_list_const_iterator& operator-=(const difference_type dist) noexcept {
            for (difference_type i = 0; i < dist; ++i) { --*this; }
            return *this;
        }
    };

    template <typename T>
    circular_list_const_iterator<T> operator+(circular_list_const_iterator<T> it, const typename circular_list_const_iterator<T>::difference_type dist) noexcept {
        it += dist;
        return it;
    }

    template <typename T>
    circular_list_const_iterator<T> operator-(circular_list_const_iterator<T> it, const typename circular_list_const_iterator<T>::difference_type dist) noexcept {
        it -= dist;
        return it;
    }

    //************************ circular_list Methods ************************

    template <typename T>
    circular_list<T>::iterator circular_list<T>::insert(
            const_iterator pos,
            std::unique_ptr<node>& first_node,
            std::unique_ptr<node>& last_node,
            const size_type count)
    {
        auto* first = first_node.get();
        auto* last = last_node.get();
        if (pos.m_ptr == nullptr) {
            if (m_size == 0) {
                last->next = first;
                first->prev = last;
                m_head = first_node.release();
                last_node.release();
                m_size += count;
            }
            else {
                throw std::logic_error{"invalid iterator"};
            }
        }
        else {
            last->next = pos.m_ptr;
            first->prev = pos.m_ptr->prev;
            pos.m_ptr->prev->next = first;
            pos.m_ptr->prev = last;

            if (m_head == pos.m_ptr && !pos.m_passed_end) {
                m_head = first;
            }
            first_node.release();
            last_node.release();
            m_size += count;
        }
        return iterator{first, this};
    }

    template <typename T>
    circular_list<T>::iterator circular_list<T>::insert(const_iterator pos, const T& value) {
        if (pos.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        auto new_node = std::make_unique<node>(value);
        return insert(pos, new_node, new_node, 1);
    }

    template <typename T>
    circular_list<T>::iterator circular_list<T>::insert(const_iterator pos, T&& value) {
        if (pos.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        auto new_node = std::make_unique<node>(std::forward<T>(value));
        return insert(pos, new_node, new_node, 1);
    }

    template <typename T>
    circular_list<T>::iterator circular_list<T>::insert(
        const_iterator pos, size_type count, const T& value)
    {
        if (pos.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        if (count == 0) {
            return pos;
        }
        std::vector<std::unique_ptr<node>> new_nodes;
        for (size_type i = 0; i < count; ++i) {
            auto nn = std::make_unique<node>(value);
            if (i != 0) [[likely]] {
                nn->prev = new_nodes.back().get();
                new_nodes.back()->next = nn.get();
            }
            new_nodes.push_back(std::move(nn));
        }
        return insert(pos, new_nodes.front(), new_nodes.back(), count);
    }

    template <typename T>
    template <std::input_iterator Iter>
    circular_list<T>::iterator circular_list<T>::insert(const_iterator pos, Iter first, Iter last) {
        if (pos.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        const auto count = std::distance(first, last);
        if (count == 0) {
            return pos;
        }
        std::vector<std::unique_ptr<node>> new_nodes;
        for (auto current = first; current != last; ++current) {
            auto nn = std::make_unique<node>(*current);
            if (current != first) [[likely]] {
                nn->prev = new_nodes.back().get();
                new_nodes.back()->next = nn.get();
            }
            new_nodes.push_back(std::move(nn));
        }
        return insert(pos, new_nodes.front(), new_nodes.back(), count);
    }

    template <typename T>
    circular_list<T>::iterator circular_list<T>::insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template <typename T>
    template<typename R>
    circular_list<T>::iterator circular_list<T>::insert_range(const_iterator pos, R&& rg) requires std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T> {
        return insert(pos, rg.begin(), rg.end());
    }

    template <typename T>
    template<class... Args>
    circular_list<T>::iterator circular_list<T>::emplace(const_iterator pos, Args&&... args) {
        if (pos.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        auto new_node = std::make_unique<node>(std::forward<T>(args)...);
        return insert(pos, new_node, new_node, 1);
    }

    template <typename T>
    circular_list<T>::iterator circular_list<T>::erase(const_iterator pos) {
        if (pos.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        if (m_size == 0) {
            throw std::logic_error{"cannot erase from empty list"};
        }
        std::unique_ptr<node> del {pos.m_ptr};
        if (m_size == 1) {
            m_head = nullptr;
            m_size = 0;
            return end();
        }
        else {
            auto* next = del->next;
            del->prev->next = del->next;
            del->next->prev = del->prev;
            if (m_head == del) {
                m_head = next;
            }
            --m_size;
            return {next, this};
        }
    }

    template <typename T>
    circular_list<T>::iterator circular_list<T>::erase(const_iterator first, const_iterator last) {
        if (first.m_parent != this || last.m_parent != this) {
            throw std::logic_error{"iterator mismatch"};
        }
        if (m_size == 0) {
            throw std::logic_error{"cannot erase from empty list"};
        }
        if (first == last) {
            return last;
        }
        std::vector<std::unique_ptr<node>> dels;
        dels.reserve(m_size);
        node *prev = first.m_ptr->prev, *next = last.m_ptr->next;
        bool contained_head = false;
        size_type num_deleted = 0;
        for (auto current = first; current != last; ++current) {
            contained_head = current.m_ptr == m_head || contained_head;
            dels.emplace_back(std::move(current.m_ptr));
            ++num_deleted;
        }
        m_size -= num_deleted;
        if (m_size == 0) {
            m_head = nullptr;
            return end();
        }
        else if (contained_head) {
            m_head = next;
        }
        prev->next = next;
        next->prev = prev;
        return {next, this};
    }



} /* namespace aoc */

#endif //ADVENTOFCODE_CIRCULAR_LIST_H