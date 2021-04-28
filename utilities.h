//
// Created by Daniel Garcia on 12/1/20.
//

#ifndef ADVENTOFCODE2020_UTILITIES_H
#define ADVENTOFCODE2020_UTILITIES_H

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <optional>
#include <sstream>

namespace aoc {

    std::string ltrim(const std::string& s);
    std::string rtrim(const std::string& s);
    std::string trim(const std::string& s);

    std::string_view ltrim(std::string_view s);
    std::string_view rtrim(std::string_view s);
    std::string_view trim(std::string_view s);

    std::vector<std::string_view> split(std::string_view s, char c);
    std::vector<std::string_view> split(std::string_view s, std::string_view spl);
    std::vector<std::string_view> split_no_empty(std::string_view s, char c);
    std::vector<std::string_view> split_no_empty(std::string_view s, std::string_view spl);

    std::optional<std::string_view> starts_with(std::string_view str, std::string_view to_find);

    std::vector<std::string> read_file_lines(const std::filesystem::path& file);
    std::string read_file(const std::filesystem::path& file);

    template <typename T>
    T default_to_string(T&& t) {
        return std::forward<T>(t);
    }

    template <typename Iter, typename F>
    std::string join(Iter begin, const Iter end, std::string_view join_str, F func = default_to_string<decltype(*std::declval<Iter>())>) {
        std::stringstream js;
        bool first = true;
        for (; begin != end; ++begin) {
            if (!first) {
                js << join_str;
            }
            first = false;
            js << func(*begin);
        }
        return js.str();
    }

    template <typename Iter, typename F>
    std::string join(Iter begin, const Iter end, char join_ch, F func = default_to_string<decltype(*std::declval<Iter>())>) {
        std::stringstream js;
        bool first = true;
        for (; begin != end; ++begin) {
            if (!first) {
                js << join_ch;
            }
            first = false;
            js << func(*begin);
        }
        return js.str();
    }

    template <typename Container, typename F = decltype(default_to_string<decltype(*std::declval<Container>().begin())>)>
    std::string join(Container c, std::string_view join_str, F func = default_to_string<decltype(*std::declval<Container>().begin())>) {
        return join(std::begin(c), std::end(c), join_str, func);
    }

    template <typename Container, typename F = decltype(default_to_string<decltype(*std::declval<Container>().begin())>)>
    std::string join(Container c, char join_ch, F func = default_to_string<decltype(*std::declval<Container>().begin())>) {
        return join(std::begin(c), std::end(c), join_ch, func);
    }

} /* namespace aoc */

#endif //ADVENTOFCODE2020_UTILITIES_H
