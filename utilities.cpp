//
// Created by Daniel Garcia on 12/1/20.
//

#include "utilities.h"

#include <fstream>

namespace fs = std::filesystem;

namespace aoc {

    namespace {

        const char *whitespace = " \t\n";

    }

    std::string ltrim(const std::string& s) {
        auto pos = s.find_first_not_of(whitespace);
        if (pos != std::string::npos) {
            return s.substr(pos);
        }
        else {
            return s;
        }
    }

    std::string rtrim(const std::string& s) {
        auto pos = s.find_last_not_of(whitespace);
        if (pos == std::string::npos) {
            pos = 0;
        }
        else {
            ++pos;
        }
        return s.substr(0, pos);
    }

    std::string trim(const std::string& s) {
        return ltrim(rtrim(s));
    }

    std::string_view ltrim(std::string_view s) {
        auto pos = s.find_first_not_of(whitespace);
        if (pos != std::string::npos) {
            return s.substr(pos);
        }
        else {
            return s;
        }
    }

    std::string_view rtrim(std::string_view s) {
        auto pos = s.find_last_not_of(whitespace);
        if (pos == std::string::npos) {
            pos = 0;
        }
        else {
            ++pos;
        }
        return s.substr(0, pos);
    }

    std::string_view trim(std::string_view s) {
        return ltrim(rtrim(s));
    }

    std::vector<std::string_view> split(std::string_view s, char c) {
        std::vector<std::string_view> retval;
        if (s.empty()) {
            return retval;
        }
        std::string::size_type pos = s.find(c, 0), start = 0;
        while (pos != std::string::npos) {
            retval.emplace_back(s.data() + start, pos - start);
            start = pos + 1;
            pos = s.find(c, start);
        }
        retval.emplace_back(s.data() + start, s.size() - start);
        return retval;
    }

    std::vector<std::string_view> split(std::string_view s, std::string_view spl) {
        std::vector<std::string_view> retval;
        if (s.empty()) {
            return retval;
        }
        std::string::size_type pos = s.find(spl, 0), start = 0;
        while (pos != std::string::npos) {
            retval.emplace_back(s.data() + start, pos - start);
            start = pos + spl.size();
            pos = s.find(spl, start);
        }
        retval.emplace_back(s.data() + start, s.size() - start);
        return retval;
    }

    std::vector<std::string> read_file_lines(const fs::path& file) {
        if (!fs::exists(file)) {
            throw std::runtime_error{"Can't read lines from file which doesn't exist."};
        }
        else if (!fs::is_regular_file(file)) {
            throw std::runtime_error{"Can't read lines from file which isn't a regular file."};
        }
        std::string line;
        std::vector<std::string> input;
        std::ifstream in {file};
        while (std::getline(in, line)) {
            input.push_back(std::move(trim(line)));
        }
        return input;
    }

} /* namespace aoc */