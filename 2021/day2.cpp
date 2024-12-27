//
// Created by Dan on 12/2/2021.
//


#include "registration.h"

#include <vector>
#include <iostream>
#include <string_view>
#include <charconv>
#include <cstdint>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    enum class direction {
        forward,
        up,
        down
    };

    struct move {
        direction dir;
        int num;
    };

    move parse_move(std::string_view line) {
        constexpr auto FWD = "forward "sv;
        constexpr auto UP = "up "sv;
        constexpr auto DOWN = "down "sv;
        std::string_view dir_str;
        direction dir{};
        if (line.starts_with(FWD)) {
            dir_str = FWD;
            dir = direction::forward;
        }
        else if (line.starts_with(UP)) {
            dir_str = UP;
            dir = direction::up;
        }
        else if (line.starts_with(DOWN)) {
            dir_str = DOWN;
            dir = direction::down;
        }
        else {
            throw std::runtime_error{"Invalid direction."};
        }

        int num = -1;
        auto res = std::from_chars(line.data() + dir_str.size(), line.data() + line.size(), num);
        if (res.ec != std::errc{}) {
            throw std::system_error{make_error_code(res.ec)};
        }
        return {dir, num};
    }

    std::vector<move> get_input(const std::vector<std::string>& lines) {
        std::vector<move> values;
        std::transform(lines.begin(), lines.end(), std::back_inserter(values), &parse_move);
        return values;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto dirs = get_input(lines);
        int horiz = 0, depth = 0;
        for (const auto& d : dirs) {
            switch (d.dir) {
                case direction::forward:    horiz += d.num; break;
                case direction::up:         depth -= d.num; break;
                case direction::down:       depth += d.num; break;
                default: break;
            }
        }
        return std::to_string(horiz * depth);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto dirs = get_input(lines);
        int64_t horiz = 0, depth = 0, aim = 0;
        for (const auto& d : dirs) {
            switch (d.dir) {
                case direction::forward:    horiz += d.num; depth += d.num * aim; break;
                case direction::up:         aim -= d.num; break;
                case direction::down:       aim += d.num; break;
                default: break;
            }
        }
        return std::to_string(horiz * depth);
    }

    aoc::registration r {2021, 2, part_1, part_2};

}