//
// Created by Daniel Garcia on 12/25/20.
//

#ifndef ADVENTOFCODE_UTILITY_TEMPLATES_H
#define ADVENTOFCODE_UTILITY_TEMPLATES_H

#include <functional>

namespace aoc {

    template <typename T> struct base_type { using type = T; };
    template <typename T> struct base_type<const T> { using type = typename base_type<T>::type; };
    template <typename T> struct base_type<volatile T> { using type = typename base_type<T>::type; };
    template <typename T> struct base_type<T&> { using type = typename base_type<T>::type; };
    template <typename T> struct base_type<T*> { using type = typename base_type<T>::type; };
    template <typename T> using base_type_t = typename base_type<T>::type;

    class hash_combiner {
        std::size_t hash;

        explicit hash_combiner(std::size_t h) : hash{h} {}
    public:
        hash_combiner(const hash_combiner&) = default;
        hash_combiner(hash_combiner&&) noexcept = default;
        template <typename T, typename = std::enable_if_t<!std::is_same_v<base_type_t<T>, hash_combiner>, void>>
        explicit hash_combiner(T&& v) : hash{std::hash<base_type_t<T>>{}(std::forward<T>(v))} {}

        hash_combiner operator+(hash_combiner rhs) const {
            hash_combiner retval {hash};
            retval.hash ^= rhs.hash + 0x9e3779b9 + (retval.hash << 6) + (retval.hash >> 2);
            return retval;
        }

        [[nodiscard]] std::size_t value() const { return hash; }
    };

    template <typename... Args>
    std::size_t multi_hash(Args&&... args) {
        return ((... + hash_combiner{args})).value();
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_UTILITY_TEMPLATES_H
