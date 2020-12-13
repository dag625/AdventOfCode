//
// Created by Daniel Garcia on 12/1/20.
//

#ifndef ADVENTOFCODE2020_UTILITIES_H
#define ADVENTOFCODE2020_UTILITIES_H

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>

namespace aoc {

    std::string ltrim(const std::string& s);
    std::string rtrim(const std::string& s);
    std::string trim(const std::string& s);

    std::vector<std::string_view> split(const std::string& s, char c);

    std::vector<std::string> read_file_lines(const std::filesystem::path& file);

} /* namespace aoc */

#endif //ADVENTOFCODE2020_UTILITIES_H
