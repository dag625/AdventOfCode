//
// Created by Daniel Garcia on 4/28/21.
//

#ifndef ADVENTOFCODE_PARSE_H
#define ADVENTOFCODE_PARSE_H

#include <string_view>
#include <charconv>
#include <system_error>

namespace aoc {

    template <typename T>
    T parse(std::string_view s, int base = 10) {
        if (s.empty()) {
            throw std::runtime_error{"Cannot parse empty string."};
        }
        T val{};
        const auto res = std::from_chars(s.data(), s.data() + s.size(), val, base);
        if (const auto ec = std::make_error_code(res.ec); ec) {
            throw std::system_error{ec};
        }
        return val;
    }

    //Convenience functions:
    inline int parse32(std::string_view s) { return parse<int>(s); }
    inline int64_t parse64(std::string_view s) { return parse<int64_t>(s); }
    inline uint32_t parseu32(std::string_view s) { return parse<uint32_t>(s); }
    inline uint64_t parseu64(std::string_view s) { return parse<uint64_t>(s); }

    template <typename T>
    T parse_allow_plus(std::string_view s, int base = 10) {
        if (s.starts_with('+')) {
            s.remove_prefix(1);
        }
        return parse<T>(s, base);
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_PARSE_H
