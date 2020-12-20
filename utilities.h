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

} /* namespace aoc */

#endif //ADVENTOFCODE2020_UTILITIES_H
