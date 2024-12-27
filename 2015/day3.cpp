//
// Created by Daniel Garcia on 4/27/21.
//

#include "registration.h"
#include "utilities.h"
#include "point.h"
#include "hash_functions.h"

#include <doctest/doctest.h>

#include <unordered_map>
#include <array>

namespace {

    using namespace aoc;

    std::string get_input(const std::vector<std::string>& lines) {
		return lines.front();
    }

    velocity get_vel(char c) {
        switch (c) {
            case '^': return {0, 1};
            case 'v': return {0, -1};
            case '<': return {-1, 0};
            case '>': return {1, 0};
            default: return {0, 0};
        }
    }

    std::unordered_map<position, uint32_t> calculate_visits(const std::string& steps) {
        std::unordered_map<position, uint32_t> visits;
        position pos {0, 0};
        ++visits[pos];
        for (char c : steps) {
            pos += get_vel(c);
            ++visits[pos];
        }
        return visits;
    }

    std::unordered_map<position, uint32_t> calculate_alt_visits(const std::string& steps) {
        std::unordered_map<position, uint32_t> visits;
        position pos {0, 0}, alt_pos{0, 0};
        ++visits[pos];
        for (char c : steps) {
            std::swap(pos, alt_pos);
            pos += get_vel(c);
            ++visits[pos];
        }
        return visits;
    }

    auto count_visits(const std::unordered_map<position, uint32_t>& visits) {
        return std::count_if(visits.begin(), visits.end(), [](const auto& v){ return v.second > 0; });
    }

    /*
    --- Day 3: Perfectly Spherical Houses in a Vacuum ---

    Santa is delivering presents to an infinite two-dimensional grid of houses.

    He begins by delivering a present to the house at his starting location, and then an elf at the North Pole calls him via radio and tells him where to move next. Moves are always exactly one house to the north (^), south (v), east (>), or west (<). After each move, he delivers another present to the house at his new location.

    However, the elf back at the north pole has had a little too much eggnog, and so his directions are a little off, and Santa ends up visiting some houses more than once. How many houses receive at least one present?

    For example:

    > delivers presents to 2 houses: one at the starting location, and one to the east.
    ^>v< delivers presents to 4 houses in a square, including twice to the house at his starting/ending location.
    ^v^v^v^v^v delivers a bunch of presents to some very lucky children at only 2 houses.
     */
    std::string part_1(const std::vector<std::string>& lines) {
        auto steps = get_input(lines);
        return std::to_string(count_visits(calculate_visits(steps)));
    }

    /*
    The next year, to speed up the process, Santa creates a robot version of himself, Robo-Santa, to deliver presents with him.

    Santa and Robo-Santa start at the same location (delivering two presents to the same starting house), then take turns moving based on instructions from the elf, who is eggnoggedly reading from the same script as the previous year.

    This year, how many houses receive at least one present?

    For example:

    ^v delivers presents to 3 houses, because Santa goes north, and then Robo-Santa goes south.
    ^>v< now delivers presents to 3 houses, and Santa and Robo-Santa end up back where they started.
    ^v^v^v^v^v now delivers presents to 11 houses, with Santa going one direction and Robo-Santa going the other.
     */
    std::string part_2(const std::vector<std::string>& lines) {
        auto steps = get_input(lines);
        return std::to_string(count_visits(calculate_alt_visits(steps)));
    }

    aoc::registration r {2015, 3, part_1, part_2};

    TEST_SUITE("2015_day3") {
        TEST_CASE("2015_day3:example") {
        REQUIRE_EQ(count_visits(calculate_visits(">")), 2);
        REQUIRE_EQ(count_visits(calculate_visits("^>v<")), 4);
        REQUIRE_EQ(count_visits(calculate_visits("^v^v^v^v^v")), 2);
        REQUIRE_EQ(count_visits(calculate_alt_visits("^v")), 3);
        REQUIRE_EQ(count_visits(calculate_alt_visits("^>v<")), 3);
        REQUIRE_EQ(count_visits(calculate_alt_visits("^v^v^v^v^v")), 11);
    }

}

}

