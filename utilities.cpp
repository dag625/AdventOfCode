//
// Created by Daniel Garcia on 12/1/20.
//

#include "utilities.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <cmrc/cmrc.hpp>

#include <fstream>

CMRC_DECLARE(aoc);

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

    std::vector<std::string_view> split_no_empty(std::string_view s, char c) {
        auto splited = split(s, c);
        splited.erase(std::remove_if(splited.begin(), splited.end(),
                                 [](const std::string_view sv){ return sv.empty(); }),
                      splited.end());
        return splited;
    }

    std::vector<std::string_view> split_no_empty(std::string_view s, std::string_view spl) {
        auto splited = split(s, spl);
        splited.erase(std::remove_if(splited.begin(), splited.end(),
                                     [](const std::string_view sv){ return sv.empty(); }),
                      splited.end());
        return splited;
    }

    std::optional<std::string_view> starts_with(std::string_view str, std::string_view to_find) {
        if (str.find(to_find) == 0) {
            return str.substr(to_find.size());
        }
        else {
            return std::nullopt;
        }
    }

    std::vector<std::string> read_file_lines(const fs::path& file) {
        const auto path = file.generic_string();
        const auto rc = cmrc::aoc::get_filesystem();
        if (!rc.exists(path)) {
            throw std::runtime_error{fmt::format("Can't read lines from resource '{}' which doesn't exist.", path)};
        }
        const auto f = rc.open(path);
        std::vector<std::string> input;
        auto current = f.begin(), start = current;
        const auto end = f.end();
        for (; current != end; ++current) {
            if (*current == '\n') {
                input.emplace_back(start, current);
                if (!input.back().empty() && input.back().back() == '\r') {
                    input.back() = input.back().substr(0, input.back().size() - 1);
                }
                start = current + 1;
            }
        }
        input.emplace_back(start, current);
        return input;
    }

    std::string read_file(const fs::path& file) {
        const auto path = file.string();
        const auto rc = cmrc::aoc::get_filesystem();
        if (!rc.exists(path)) {
            throw std::runtime_error{fmt::format("Can't read from resource '{}' which doesn't exist.", path)};
        }
        const auto f = rc.open(path);
        return {f.begin(), f.end()};
    }

    TEST_SUITE("utilities" * doctest::description("Tests for utility functions.")) {
        using namespace std::string_view_literals;
        TEST_CASE("utilities:split1" * doctest::description("Tests for split() functions.")) {
            REQUIRE_EQ(split("a,b,c,d"sv, ',').size(), 4);
            REQUIRE_EQ(split(",a,b,c,d"sv, ',').size(), 5);
            REQUIRE_EQ(split("a,b,c,d,"sv, ',').size(), 5);
            REQUIRE_EQ(split("a,b,,c,d"sv, ',').size(), 5);
            REQUIRE_EQ(split("a,b,c,d,,"sv, ',').size(), 6);
            REQUIRE_EQ(split(",a,b,c,d,"sv, ',').size(), 6);
            REQUIRE_EQ(split(",,a,b,c,d"sv, ',').size(), 6);
            REQUIRE_EQ(split("a,,b,c,,d"sv, ',').size(), 6);
            REQUIRE_EQ(split("a,,,b,c,d"sv, ',').size(), 6);
        }
        TEST_CASE("utilities:split_no_empty1" * doctest::description("Tests for split() functions.")) {
            REQUIRE_EQ(split_no_empty("a,b,c,d"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty(",a,b,c,d"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty("a,b,c,d,"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty("a,b,,c,d"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty("a,b,c,d,,"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty(",a,b,c,d,"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty(",,a,b,c,d"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty("a,,b,c,,d"sv, ',').size(), 4);
            REQUIRE_EQ(split_no_empty("a,,,b,c,d"sv, ',').size(), 4);
        }
    }

} /* namespace aoc */