//
// Created by Dan on 11/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "point.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr std::array<velocity, 4> DIR_MAP {velocity{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    enum class direction : int {
        North = 0,
        East = 1,
        South = 2,
        West = 3
    };

    enum class turn : int {
        Left = -1,
        Right =1
    };

    const velocity& dir_to_vel(direction current) {
        return DIR_MAP[static_cast<int>(current)];
    }

    direction make_turn(direction d, turn t) {
        const auto res = static_cast<int>(d) + static_cast<int>(t);
        if (res < 0) {
            return direction::West;
        }
        else if (res > 3) {
            return direction::North;
        }
        else {
            return static_cast<direction>(res);
        }
    }

    struct instruction {
        turn turn_dir = turn::Left;
        int distance = 0;
    };

    instruction parse_instruction(std::string_view s) {
        instruction retval;
        retval.turn_dir = s.front() == 'R' ? turn::Right : turn::Left;
        s.remove_prefix(1);
        retval.distance = parse<int>(s);
        return retval;
    }

    struct state {
        direction dir = direction::North;
        position pos {0, 0};

        state operator+(const instruction& i) {
            state retval = *this;
            retval.dir = make_turn(dir, i.turn_dir);
            retval.pos = pos + (dir_to_vel(retval.dir) * i.distance);
            return *this;
        }

        state& operator+=(const instruction& i) {
            dir = make_turn(dir, i.turn_dir);
            pos = pos + (dir_to_vel(dir) * i.distance);
            return *this;
        }
    };

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        std::vector<instruction> retval;
        for (const auto& l : lines) {
            const auto parts = split(l, ", ");
            std::transform(parts.begin(), parts.end(), std::back_inserter(retval), &parse_instruction);
        }
        return retval;
    }

    /*
    --- Day 1: No Time for a Taxicab ---
    Santa's sleigh uses a very high-precision clock to guide its movements, and the clock's oscillator is regulated by stars. Unfortunately, the stars have been stolen... by the Easter Bunny. To save Christmas, Santa needs you to retrieve all fifty stars by December 25th.

    Collect stars by solving puzzles. Two puzzles will be made available on each day in the Advent calendar; the second puzzle is unlocked when you complete the first. Each puzzle grants one star. Good luck!

    You're airdropped near Easter Bunny Headquarters in a city somewhere. "Near", unfortunately, is as close as you can get - the instructions on the Easter Bunny Recruiting Document the Elves intercepted start here, and nobody had time to work them out further.

    The Document indicates that you should start at the given coordinates (where you just landed) and face North. Then, follow the provided sequence: either turn left (L) or right (R) 90 degrees, then walk forward the given number of blocks, ending at a new intersection.

    There's no time to follow such ridiculous instructions on foot, though, so you take a moment and work out the destination. Given that you can only walk on the street grid of the city, how far is the shortest path to the destination?

    For example:

    Following R2, L3 leaves you 2 blocks East and 3 blocks North, or 5 blocks away.
    R2, R2, R2 leaves you 2 blocks due South of your starting position, which is 2 blocks away.
    R5, L5, R5, R3 leaves you 12 blocks away.
    How many blocks away is Easter Bunny HQ?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        state s{};
        for (const auto& ins : input) {
            s += ins;
        }
        return std::to_string(std::abs(s.pos.x) + std::abs(s.pos.y));
    }

    /*
    --- Part Two ---
    Then, you notice the instructions continue on the back of the Recruiting Document. Easter Bunny HQ is actually at the first location you visit twice.

    For example, if your instructions are R8, R4, R4, R8, the first location you visit twice is 4 blocks away, due East.

    How many blocks away is the first location you visit twice?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        state s{};
        std::vector<position> visited;
        visited.reserve(input.size() * 10);
        for (const auto& ins : input) {
            const auto dir = make_turn(s.dir, ins.turn_dir);
            const auto& vel = dir_to_vel(dir);
            auto pos = s.pos;
            bool revisited = false;
            for (int i = 1; i <= ins.distance; ++i) {
                pos += vel;
                const auto found = std::lower_bound(visited.begin(), visited.end(), pos);
                if (found != visited.end() && *found == pos) {
                    revisited = true;
                    break;
                }
                else {
                    visited.insert(found, pos);
                }
            }
            s.dir = dir;
            s.pos = pos;
            if (revisited) {
                break;
            }
        }
        return std::to_string(std::abs(s.pos.x) + std::abs(s.pos.y));
    }

    aoc::registration r{2016, 1, part_1, part_2};

//    TEST_SUITE("2016_day1") {
//        TEST_CASE("2016_day1:example") {
//
//        }
//    }

}