//
// Created by Daniel Garcia on 12/13/20.
//

#include "registration.h"
#include "utilities.h"
#include "grid.h"

#include <iostream>
#include <charconv>
#include <iomanip>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        enum class direction : int {
            //Order here matters!
            North = 0,
            East = 1,
            South = 2,
            West = 3,
            Forward,
            Left,
            Right
        };

        std::ostream& operator<<(std::ostream& os, direction d) {
            switch (d) {
                case direction::Forward: os << "Forward"; break;
                case direction::North: os << "North"; break;
                case direction::South: os << "South"; break;
                case direction::East: os << "East"; break;
                case direction::West: os << "West"; break;
                case direction::Left: os << "Left"; break;
                case direction::Right: os << "Right"; break;
                default: os << "Unknown"; break;
            }
            return os;
        }

        direction transform_left(direction curr, int degrees) {
            auto val = (static_cast<int>(curr) - degrees / 90);
            if (val < 0) {
                val += 4;
            }
            return static_cast<direction>(val);
        }

        direction transform_right(direction curr, int degrees) {
            return static_cast<direction>((static_cast<int>(curr) + degrees / 90) % 4);
        }

        struct instruction {
            direction dir;
            int value;
        };

        velocity transform_to_velocity(direction dir, int value) {
            //These orientations match the grid class...
            switch (dir) {
                case direction::North: return {value * -1, 0};
                case direction::South: return {value, 0};
                case direction::East: return {0, value};
                case direction::West: return {0, value * -1};
                default: return {0, 0};
            }
        }

        struct course {
            position pos;
            direction dir;
        };

        course& operator+=(course& c, instruction ins) {
            if (ins.dir == direction::Left) {
                c.dir = transform_left(c.dir, ins.value);
            }
            else if (ins.dir == direction::Right) {
                c.dir = transform_right(c.dir, ins.value);
            }
            else {
                c.pos += transform_to_velocity(ins.dir == direction::Forward ? c.dir : ins.dir, ins.value);
            }
            return c;
        }

        course operator+(course& c, instruction ins) { auto ret = c; ret += ins; return ret; }

        position shift_left(position p) {
            return {-p.y, p.x};
        }

        position shift_right(position p) {
            return {p.y, -p.x};
        }

        position move_waypoint(position p, instruction ins) {
            if (ins.dir == direction::Left || ins.dir == direction::Right) {
                auto f = ins.dir == direction::Left ? shift_left : shift_right;
                const int times = ins.value / 90;
                for (int i = 0; i < times; ++i) {
                    p = f(p);
                }
            }
            else if (ins.dir != direction::Forward) {
                p += transform_to_velocity(ins.dir, ins.value);
            }
            return p;
        }

        struct waypoint_course {
            position pos;
            position waypoint;
        };

        waypoint_course& operator+=(waypoint_course& c, instruction ins) {
            c.waypoint = move_waypoint(c.waypoint, ins);
            if (ins.dir == direction::Forward) {
                c.pos += velocity{ ins.value * c.waypoint.x, ins.value * c.waypoint.y };
            }
            return c;
        }

        waypoint_course operator+(waypoint_course& c, instruction ins) { auto ret = c; ret += ins; return ret; }

        instruction parse_instruction(const std::string& s) {
            if (s.size() < 2) {
                throw std::runtime_error{"Invalid empty instruction."};
            }
            int val = 0;
            auto res = std::from_chars(s.data() + 1, s.data() + s.size(), val);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            switch (s[0]) {
                case 'F': return {direction::Forward, val};
                case 'N': return {direction::North, val};
                case 'S': return {direction::South, val};
                case 'E': return {direction::East, val};
                case 'W': return {direction::West, val};
                case 'L': return {direction::Left, val};
                case 'R': return {direction::Right, val};
                default: throw std::runtime_error{"Invalid direction n instruction."};
            }
        }

        std::vector<instruction> get_input(const fs::path &input_dir) {
            auto lines = read_file_lines(input_dir / "2020" / "day_12_input.txt");
            std::vector<instruction> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_instruction);
            return retval;
        }

    }

    /*
    Your ferry made decent progress toward the island, but the storm came in faster than anyone expected. The ferry needs to take evasive actions!

    Unfortunately, the ship's navigation computer seems to be malfunctioning; rather than giving a route directly to safety, it produced extremely circuitous instructions. When the captain uses the PA system to ask if anyone can help, you quickly volunteer.

    The navigation instructions (your puzzle input) consists of a sequence of single-character actions paired with integer input values. After staring at them for a few minutes, you work out what they probably mean:

    Action N means to move north by the given value.
    Action S means to move south by the given value.
    Action E means to move east by the given value.
    Action W means to move west by the given value.
    Action L means to turn left the given number of degrees.
    Action R means to turn right the given number of degrees.
    Action F means to move forward by the given value in the direction the ship is currently facing.
    The ship starts by facing east. Only the L and R actions change the direction the ship is facing. (That is, if the ship is facing east and the next instruction is N10, the ship would move north 10 units, but would still move east if the following action were F.)

    For example:

    F10
    N3
    F7
    R90
    F11
    These instructions would be handled as follows:

    F10 would move the ship 10 units east (because the ship starts by facing east) to east 10, north 0.
    N3 would move the ship 3 units north to east 10, north 3.
    F7 would move the ship another 7 units east (because the ship is still facing east) to east 17, north 3.
    R90 would cause the ship to turn right by 90 degrees and face south; it remains at east 17, north 3.
    F11 would move the ship 11 units south to east 17, south 8.
    At the end of these instructions, the ship's Manhattan distance (sum of the absolute values of its east/west position and its north/south position) from its starting position is 17 + 8 = 25.

    Figure out where the navigation instructions lead. What is the Manhattan distance between that location and the ship's starting position?
    */
    std::string solve_day_12_1(const std::filesystem::path& input_dir) {
        auto instructions = get_input(input_dir);
        course c {{0, 0}, direction::East};
        for (const auto& ins : instructions) {
            c += ins;
        }
        return std::to_string(abs(c.pos.x) + abs(c.pos.y));
    }

    /*
    Before you can give the destination to the captain, you realize that the actual action meanings were printed on the back of the instructions the whole time.

    Almost all of the actions indicate how to move a waypoint which is relative to the ship's position:

    Action N means to move the waypoint north by the given value.
    Action S means to move the waypoint south by the given value.
    Action E means to move the waypoint east by the given value.
    Action W means to move the waypoint west by the given value.
    Action L means to rotate the waypoint around the ship left (counter-clockwise) the given number of degrees.
    Action R means to rotate the waypoint around the ship right (clockwise) the given number of degrees.
    Action F means to move forward to the waypoint a number of times equal to the given value.
    The waypoint starts 10 units east and 1 unit north relative to the ship. The waypoint is relative to the ship; that is, if the ship moves, the waypoint moves with it.

    For example, using the same instructions as above:

    F10 moves the ship to the waypoint 10 times (a total of 100 units east and 10 units north), leaving the ship at east 100, north 10. The waypoint stays 10 units east and 1 unit north of the ship.
    N3 moves the waypoint 3 units north to 10 units east and 4 units north of the ship. The ship remains at east 100, north 10.
    F7 moves the ship to the waypoint 7 times (a total of 70 units east and 28 units north), leaving the ship at east 170, north 38. The waypoint stays 10 units east and 4 units north of the ship.
    R90 rotates the waypoint around the ship clockwise 90 degrees, moving it to 4 units east and 10 units south of the ship. The ship remains at east 170, north 38.
    F11 moves the ship to the waypoint 11 times (a total of 44 units east and 110 units south), leaving the ship at east 214, south 72. The waypoint stays 4 units east and 10 units south of the ship.
    After these operations, the ship's Manhattan distance from its starting position is 214 + 72 = 286.

    Figure out where the navigation instructions actually lead. What is the Manhattan distance between that location and the ship's starting position?
    */
    std::string solve_day_12_2(const std::filesystem::path& input_dir) {
        auto instructions = get_input(input_dir);
        waypoint_course c {{0, 0},
                           {-1, 10}};
        for (const auto& ins : instructions) {
            c += ins;
        }
        return std::to_string(abs(c.pos.x) + abs(c.pos.y));
    }

    static aoc::registration r {2020, 12, solve_day_12_1, solve_day_12_2};

} /* namespace aoc2020 */