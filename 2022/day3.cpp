//
// Created by Dan on 12/3/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<std::string> get_input(const fs::path &input_dir) {
        return read_file_lines(input_dir / "2022" / "day_3_input.txt");
    }

    std::pair<std::string_view, std::string_view> get_compartments(const std::string& s) {
        const auto midpoint = static_cast<std::string::difference_type>(s.size() / 2);
        return {{s.begin(), s.begin() + midpoint}, {s.begin() + midpoint, s.end()}};
    }

    char in_both(const std::pair<std::string_view, std::string_view>& ruck) {
        std::unordered_set<char> in_first;
        for (char c : ruck.first) {
            in_first.insert(c);
        }
        for (char c : ruck.second) {
            if (in_first.find(c) != in_first.end()) {
                return c;
            }
        }
        return 0;
    }

    int priority(char c) {
        if (c >= 'A' && c <= 'Z') {
            return static_cast<int>(c - 'A') + 27;
        }
        else {
            return static_cast<int>(c - 'a') + 1;
        }
    }

    char find_common(std::vector<std::string>::const_iterator current, const std::vector<std::string>::const_iterator& end) {
        std::unordered_map<char, uint8_t> found;
        uint8_t mask = 0x1u;
        for (; current != end; ++current) {
            for (char c : *current) {
                found[c] |= mask;
                if (found[c] == 0x7u) {
                    return c;
                }
            }
            mask <<= 1;
        }
        return '~';
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        int sum = 0;
        for (const auto& ruck : lines) {
            const auto comps = get_compartments(ruck);
            const auto dup = in_both(comps);
            sum += priority(dup);
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        int sum = 0;
        for (auto iter = lines.begin(); iter != lines.end(); iter += 3) {
            const auto cmn = find_common(iter, iter + 3);
            sum += priority(cmn);
        }
        return std::to_string(sum);
    }

    aoc::registration r{2022, 3, part_1, part_2};

//    TEST_SUITE("2022_day03") {
//        TEST_CASE("2022_day03:example") {
//
//        }
//    }

}