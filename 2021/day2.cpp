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

    std::vector<move> get_input(const fs::path& input_dir) {
        auto input = read_file_lines(input_dir / "2021" / "day_2_input.txt");
        std::vector<move> values;
        std::transform(input.begin(), input.end(), std::back_inserter(values), &parse_move);
        return values;
    }

    /*
    --- Day 2: Dive! ---
    Now, you need to figure out how to pilot this thing.

    It seems like the submarine can take a series of commands like forward 1, down 2, or up 3:

    forward X increases the horizontal position by X units.
    down X increases the depth by X units.
    up X decreases the depth by X units.
    Note that since you're on a submarine, down and up affect your depth, and so they have the opposite result of what you might expect.

    The submarine seems to already have a planned course (your puzzle input). You should probably figure out where it's going. For example:

    forward 5
    down 5
    forward 8
    up 3
    down 8
    forward 2
    Your horizontal position and depth both start at 0. The steps above would then modify them as follows:

    forward 5 adds 5 to your horizontal position, a total of 5.
    down 5 adds 5 to your depth, resulting in a value of 5.
    forward 8 adds 8 to your horizontal position, a total of 13.
    up 3 decreases your depth by 3, resulting in a value of 2.
    down 8 adds 8 to your depth, resulting in a value of 10.
    forward 2 adds 2 to your horizontal position, a total of 15.
    After following these instructions, you would have a horizontal position of 15 and a depth of 10. (Multiplying these together produces 150.)

    Calculate the horizontal position and depth you would have after following the planned course. What do you get if you multiply your final horizontal position by your final depth?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto dirs = get_input(input_dir);
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

    /*
    --- Part Two ---
    Based on your calculations, the planned course doesn't seem to make any sense. You find the submarine manual and discover that the process is actually slightly more complicated.

    In addition to horizontal position and depth, you'll also need to track a third value, aim, which also starts at 0. The commands also mean something entirely different than you first thought:

    down X increases your aim by X units.
    up X decreases your aim by X units.
    forward X does two things:
    It increases your horizontal position by X units.
    It increases your depth by your aim multiplied by X.
    Again note that since you're on a submarine, down and up do the opposite of what you might expect: "down" means aiming in the positive direction.

    Now, the above example does something different:

    forward 5 adds 5 to your horizontal position, a total of 5. Because your aim is 0, your depth does not change.
    down 5 adds 5 to your aim, resulting in a value of 5.
    forward 8 adds 8 to your horizontal position, a total of 13. Because your aim is 5, your depth increases by 8*5=40.
    up 3 decreases your aim by 3, resulting in a value of 2.
    down 8 adds 8 to your aim, resulting in a value of 10.
    forward 2 adds 2 to your horizontal position, a total of 15. Because your aim is 10, your depth increases by 2*10=20 to a total of 60.
    After following these new instructions, you would have a horizontal position of 15 and a depth of 60. (Multiplying these produces 900.)

    Using this new interpretation of the commands, calculate the horizontal position and depth you would have after following the planned course. What do you get if you multiply your final horizontal position by your final depth?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto dirs = get_input(input_dir);
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