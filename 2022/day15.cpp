//
// Created by Dan on 12/15/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <execution>

#include "utilities.h"
#include "point.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct sensor_zone {
        int min_x = std::numeric_limits<int>::max();
        int max_x = std::numeric_limits<int>::min();
        int min_y = std::numeric_limits<int>::max();
        int max_y = std::numeric_limits<int>::min();
    };

    struct s_and_b {
        position s_pos;
        position b_pos;
        int dist = 0;
        sensor_zone zone{};
    };

    int manhattan_distance(const position a, const position b) noexcept {
        const auto d = a - b;
        return std::abs(d.x) + std::abs(d.y);
    }

    void find_zone(s_and_b& v) {
        v.dist = manhattan_distance(v.s_pos, v.b_pos);
        v.zone = sensor_zone{v.s_pos.x - v.dist, v.s_pos.x + v.dist, v.s_pos.y - v.dist, v.s_pos.y + v.dist};
    }

    s_and_b parse_sensor_and_beacon(std::string_view s) {
        const auto parts = split(s, '=');
        s_and_b retval {{parse<int>(parts[1]), parse<int>(parts[2])}, {parse<int>(parts[3]), parse<int>(parts[4])}};
        find_zone(retval);
        return retval;
    }

    std::vector<s_and_b> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_15_input.txt");
        return lines | std::views::transform(parse_sensor_and_beacon) | to<std::vector<s_and_b>>();
    }

    sensor_zone get_bounds(const std::vector<s_and_b>& input) {
        sensor_zone bounds{};
        for (const auto& v : input) {
            if (v.zone.max_x > bounds.max_x) {
                bounds.max_x = v.zone.max_x;
            }
            if (v.zone.max_y > bounds.max_y) {
                bounds.max_y = v.zone.max_y;
            }
            if (v.zone.min_x < bounds.min_x) {
                bounds.min_x = v.zone.min_x;
            }
            if (v.zone.min_y < bounds.min_y) {
                bounds.min_y = v.zone.min_y;
            }
        }
        return bounds;
    }

    constexpr int P1_ROW = 2000000;
    constexpr int P2_MIN = 0;
    constexpr int P2_MAX = 4000000;

    std::pair<bool, bool> check_ranges(position p, const std::vector<s_and_b>& input) {
        bool is_in_range = false, has_beacon = false;
        for (const auto& v : input) {
            const auto dist = manhattan_distance(p, v.s_pos);
            if (dist <= v.dist) {
                is_in_range = true;
            }
            if (p == v.b_pos) {
                has_beacon = true;
            }
        }
        return {is_in_range, has_beacon};
    }

    std::vector<position> get_boundary(const position b, const position e) {
        auto incr = e - b;
        incr.x /= std::abs(incr.x);
        incr.y /= std::abs(incr.y);
        const auto v = to_vel(incr);
        std::vector<position> retval;
        for (position c = b; c != e; c += v) {
            retval.push_back(c);
        }
        //Don't add e/end because we will call this four times and adding e/end will cause duplicate values.
        return retval;
    }

    std::vector<position> get_boundaries(const s_and_b& v) {
        position top {v.s_pos.x, v.zone.min_y - 1};
        position bottom {v.s_pos.x, v.zone.max_y + 1};
        position left {v.zone.min_x - 1, v.s_pos.y};
        position right {v.zone.max_x + 1, v.s_pos.y};
        std::vector<position> retval, tmp;

        tmp = get_boundary(top, right);
        retval.insert(retval.end(), tmp.begin(), tmp.end());
        tmp.clear();

        tmp = get_boundary(right, bottom);
        retval.insert(retval.end(), tmp.begin(), tmp.end());
        tmp.clear();

        tmp = get_boundary(bottom, left);
        retval.insert(retval.end(), tmp.begin(), tmp.end());
        tmp.clear();

        tmp = get_boundary(left, top);
        retval.insert(retval.end(), tmp.begin(), tmp.end());

        return retval;
    }

    /*
    --- Day 15: Beacon Exclusion Zone ---
    You feel the ground rumble again as the distress signal leads you to a large network of subterranean tunnels. You don't have time to search them all, but you don't need to: your pack contains a set of deployable sensors that you imagine were originally built to locate lost Elves.

    The sensors aren't very powerful, but that's okay; your handheld device indicates that you're close enough to the source of the distress signal to use them. You pull the emergency sensor system out of your pack, hit the big button on top, and the sensors zoom off down the tunnels.

    Once a sensor finds a spot it thinks will give it a good reading, it attaches itself to a hard surface and begins monitoring for the nearest signal source beacon. Sensors and beacons always exist at integer coordinates. Each sensor knows its own position and can determine the position of a beacon precisely; however, sensors can only lock on to the one beacon closest to the sensor as measured by the Manhattan distance. (There is never a tie where two beacons are the same distance to a sensor.)

    It doesn't take long for the sensors to report back their positions and closest beacons (your puzzle input). For example:

    Sensor at x=2, y=18: closest beacon is at x=-2, y=15
    Sensor at x=9, y=16: closest beacon is at x=10, y=16
    Sensor at x=13, y=2: closest beacon is at x=15, y=3
    Sensor at x=12, y=14: closest beacon is at x=10, y=16
    Sensor at x=10, y=20: closest beacon is at x=10, y=16
    Sensor at x=14, y=17: closest beacon is at x=10, y=16
    Sensor at x=8, y=7: closest beacon is at x=2, y=10
    Sensor at x=2, y=0: closest beacon is at x=2, y=10
    Sensor at x=0, y=11: closest beacon is at x=2, y=10
    Sensor at x=20, y=14: closest beacon is at x=25, y=17
    Sensor at x=17, y=20: closest beacon is at x=21, y=22
    Sensor at x=16, y=7: closest beacon is at x=15, y=3
    Sensor at x=14, y=3: closest beacon is at x=15, y=3
    Sensor at x=20, y=1: closest beacon is at x=15, y=3
    So, consider the sensor at 2,18; the closest beacon to it is at -2,15. For the sensor at 9,16, the closest beacon to it is at 10,16.

    Drawing sensors as S and beacons as B, the above arrangement of sensors and beacons looks like this:

                   1    1    2    2
         0    5    0    5    0    5
     0 ....S.......................
     1 ......................S.....
     2 ...............S............
     3 ................SB..........
     4 ............................
     5 ............................
     6 ............................
     7 ..........S.......S.........
     8 ............................
     9 ............................
    10 ....B.......................
    11 ..S.........................
    12 ............................
    13 ............................
    14 ..............S.......S.....
    15 B...........................
    16 ...........SB...............
    17 ................S..........B
    18 ....S.......................
    19 ............................
    20 ............S......S........
    21 ............................
    22 .......................B....
    This isn't necessarily a comprehensive map of all beacons in the area, though. Because each sensor only identifies its closest beacon, if a sensor detects a beacon, you know there are no other beacons that close or closer to that sensor. There could still be beacons that just happen to not be the closest beacon to any sensor. Consider the sensor at 8,7:

                   1    1    2    2
         0    5    0    5    0    5
    -2 ..........#.................
    -1 .........###................
     0 ....S...#####...............
     1 .......#######........S.....
     2 ......#########S............
     3 .....###########SB..........
     4 ....#############...........
     5 ...###############..........
     6 ..#################.........
     7 .#########S#######S#........
     8 ..#################.........
     9 ...###############..........
    10 ....B############...........
    11 ..S..###########............
    12 ......#########.............
    13 .......#######..............
    14 ........#####.S.......S.....
    15 B........###................
    16 ..........#SB...............
    17 ................S..........B
    18 ....S.......................
    19 ............................
    20 ............S......S........
    21 ............................
    22 .......................B....
    This sensor's closest beacon is at 2,10, and so you know there are no beacons that close or closer (in any positions marked #).

    None of the detected beacons seem to be producing the distress signal, so you'll need to work out where the distress beacon is by working out where it isn't. For now, keep things simple by counting the positions where a beacon cannot possibly be along just a single row.

    So, suppose you have an arrangement of beacons and sensors like in the example above and, just in the row where y=10, you'd like to count the number of positions a beacon cannot possibly exist. The coverage from all sensors near that row looks like this:

                     1    1    2    2
           0    5    0    5    0    5
     9 ...#########################...
    10 ..####B######################..
    11 .###S#############.###########.
    In this example, in the row where y=10, there are 26 positions where a beacon cannot be present.

    Consult the report from the sensors you just deployed. In the row where y=2000000, how many positions cannot contain a beacon?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto bounds = get_bounds(input);
        int count = 0;
        for (int x = bounds.min_x; x <= bounds.max_x; ++x) {
            const position p {x, P1_ROW};
            const auto [is_in_range, has_beacon] = check_ranges(p, input);
            if (is_in_range && !has_beacon) {
                ++count;
            }
        }
        return std::to_string(count);
    }

    /*
    --- Part Two ---
    Your handheld device indicates that the distress signal is coming from a beacon nearby. The distress beacon is not detected by any sensor, but the distress beacon must have x and y coordinates each no lower than 0 and no larger than 4000000.

    To isolate the distress beacon's signal, you need to determine its tuning frequency, which can be found by multiplying its x coordinate by 4000000 and then adding its y coordinate.

    In the example above, the search space is smaller: instead, the x and y coordinates can each be at most 20. With this reduced search area, there is only a single position that could have a beacon: x=14, y=11. The tuning frequency for this distress beacon is 56000011.

    Find the only possible position for the distress beacon. What is its tuning frequency?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto bounds = get_bounds(input);
        position result {-1, -1};
        for (auto it = input.begin(); it != input.end(); ++it) {
            std::vector<position> b = get_boundaries(*it);
            for (const auto &p: b) {
                if (!(p.x >= P2_MIN && p.x <= P2_MAX && p.y >= P2_MIN && p.y <= P2_MAX) ) {
                    continue;
                }
                bool in_any = false;
                for (auto jit = input.begin(); jit != input.end(); ++jit) {
                    if (it == jit) {
                        continue;
                    }
                    const auto dist = manhattan_distance(jit->s_pos, p);
                    if (dist <= jit->dist) {
                        in_any = true;
                        break;
                    }
                }
                if (!in_any) {
                    result = p;
                    break;
                }
            }
            if (result.x >= 0) {
                break;
            }
        }
        const auto res_val = static_cast<uint64_t>(result.x) * P2_MAX + result.y;
        return std::to_string(res_val);
    }

    aoc::registration r{2022, 15, part_1, part_2};

    TEST_SUITE("2022_day15") {
        TEST_CASE("2022_day15:example") {
            std::vector<std::string> lines {
                "Sensor at x=2, y=18: closest beacon is at x=-2, y=15",
                "Sensor at x=9, y=16: closest beacon is at x=10, y=16",
                "Sensor at x=13, y=2: closest beacon is at x=15, y=3",
                "Sensor at x=12, y=14: closest beacon is at x=10, y=16",
                "Sensor at x=10, y=20: closest beacon is at x=10, y=16",
                "Sensor at x=14, y=17: closest beacon is at x=10, y=16",
                "Sensor at x=8, y=7: closest beacon is at x=2, y=10",
                "Sensor at x=2, y=0: closest beacon is at x=2, y=10",
                "Sensor at x=0, y=11: closest beacon is at x=2, y=10",
                "Sensor at x=20, y=14: closest beacon is at x=25, y=17",
                "Sensor at x=17, y=20: closest beacon is at x=21, y=22",
                "Sensor at x=16, y=7: closest beacon is at x=15, y=3",
                "Sensor at x=14, y=3: closest beacon is at x=15, y=3",
                "Sensor at x=20, y=1: closest beacon is at x=15, y=3"
            };
            const auto input = lines | std::views::transform(parse_sensor_and_beacon) | to<std::vector<s_and_b>>();
            const auto bounds = get_bounds(input);
            int count = 0;
            for (int x = bounds.min_x; x <= bounds.max_x; ++x) {
                const position p {x, 10};
                bool is_in_range = false, has_beacon = false;
                for (const auto& v : input) {
                    const auto dist = manhattan_distance(p, v.s_pos);
                    if (dist <= v.dist) {
                        is_in_range = true;
                    }
                    if (p == v.b_pos) {
                        has_beacon = true;
                    }
                }
                if (is_in_range && !has_beacon) {
                    ++count;
                }
            }
            CHECK_EQ(count, 26);
        }
    }

}