//
// Created by Dan on 12/14/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <compare>
#include <array>
#include <iostream>

#include "utilities.h"
#include "point.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct location {
        position pos;
        bool is_rock = true;

        location() = default;
        location(position p, bool r = true) : pos{p}, is_rock{r} {}

        bool operator==(const location& rhs) const noexcept {
            return pos == rhs.pos;
        }

        std::strong_ordering operator<=>(const location& rhs) const noexcept {
            if (pos < rhs.pos) {
                return std::strong_ordering::less;
            }
            else if (rhs.pos < pos) {
                return std::strong_ordering::greater;
            }
            else {
                return std::strong_ordering::equal;
            }
        }

        std::strong_ordering operator<=>(const position& rhs) const noexcept {
            if (pos < rhs) {
                return std::strong_ordering::less;
            }
            else if (rhs < pos) {
                return std::strong_ordering::greater;
            }
            else {
                return std::strong_ordering::equal;
            }
        }
    };

    location parse_loc(std::string_view s) {
        const auto parts = split(s, ',');
        return {{parse<int>(parts[0]), parse<int>(parts[1])}};
    }

    std::vector<location> parse_locs(std::string_view s) {
        const auto point_strs = split(s, " -> ");
        const auto endpoints = point_strs | std::views::transform(parse_loc) | to<std::vector<location>>();
        std::vector<location> all;
        for (auto it = endpoints.begin(); it != endpoints.end() - 1; ++it) {
            position current = it->pos;
            const position end = (it + 1)->pos;
            auto dir = to_vel(end - current);
            if (dir.dx != 0) { dir.dx = dir.dx / std::abs(dir.dx); }
            if (dir.dy != 0) { dir.dy = dir.dy / std::abs(dir.dy); }

            while (current != end) {
                all.emplace_back(current);
                current += dir;
            }
        }
        all.push_back(endpoints.back());
        return all;
    }

    std::vector<location> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_14_input.txt");
        std::vector<location> all;
        for (const auto& l : lines) {
            const auto pts = parse_locs(l);
            all.insert(all.end(), pts.begin(), pts.end());
        }
        std::sort(all.begin(), all.end());
        all.erase(std::unique(all.begin(), all.end()), all.end());
        return all;
    }

    constexpr position SAND_ORIGIN {500, 0};
    constexpr auto SAND_DIRS = std::array{
        velocity{0,  1},
        velocity{-1, 1},
        velocity{1,  1}
    };

    position sand_fall(const std::vector<location>& locs, const int max_y) {
        auto current = SAND_ORIGIN;
        while (current.y < max_y) {
            bool moved = false;
            for (const auto& v : SAND_DIRS) {
                const auto new_p = current + v;
                const auto found = std::lower_bound(locs.begin(), locs.end(), new_p);
                if (found == locs.end() || found->pos != new_p) {
                    current = new_p;
                    moved = true;
                    break;
                }
            }
            if (!moved) {
                break;
            }
        }
        return current;
    }

    int calc_max_y(const std::vector<location>& locs) {
        int max_y = 0;
        for (const auto& l : locs) {
            if (l.pos.y > max_y) {
                max_y = l.pos.y;
            }
        }
        ++max_y;
        return max_y;
    }

    void draw(const std::vector<location>& locs, int max_y);

    int count_until_free_flow(std::vector<location>& locs) {
        const auto max_y = calc_max_y(locs);
        int count = 0;
        while (true) {
            const auto res = sand_fall(locs, max_y);
            if (res.y == max_y) {
                break;
            }
            ++count;
            const auto ins_pos = std::lower_bound(locs.cbegin(), locs.cend(), res);
            locs.insert(ins_pos, {res, false});
        }
        //draw(locs, max_y + 1);
        return count;
    }

    int count_until_reach_origin(std::vector<location>& locs) {
        const auto max_y = calc_max_y(locs);
        int count = 0;
        while (true) {
            const auto res = sand_fall(locs, max_y);
            ++count;
            if (res == SAND_ORIGIN) {
                break;
            }
            const auto ins_pos = std::lower_bound(locs.cbegin(), locs.cend(), res);
            locs.insert(ins_pos, {res, false});
        }
        //draw(locs, max_y);
        return count;
    }

    void draw(const std::vector<location>& locs, const int max_y) {
        int min_x = std::numeric_limits<int>::max(), max_x = std::numeric_limits<int>::min();
        for (const auto& l : locs) {
            if (l.pos.x < min_x) { min_x = l.pos.x; }
            if (l.pos.x > max_x) { max_x = l.pos.x; }
        }
        --min_x;
        ++max_x;
        std::cout << '\n';
        for (int y = 0; y < max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                const position p{x, y};
                const auto found = std::lower_bound(locs.begin(), locs.end(), p);
                if (found != locs.end() && found->pos == p) {
                    if (p == SAND_ORIGIN) {
                        std::cout << '*';
                    }
                    else if (found->is_rock) {
                        std::cout << '#';
                    }
                    else {
                        std::cout << 'o';
                    }
                }
                else {
                    if (p == SAND_ORIGIN) {
                        std::cout << '+';
                    }
                    else {
                        std::cout << '.';
                    }
                }
            }
            std::cout << '\n';
        }
    }

    /*
    --- Day 14: Regolith Reservoir ---
    The distress signal leads you to a giant waterfall! Actually, hang on - the signal seems like it's coming from the waterfall itself, and that doesn't make any sense. However, you do notice a little path that leads behind the waterfall.

    Correction: the distress signal leads you behind a giant waterfall! There seems to be a large cave system here, and the signal definitely leads further inside.

    As you begin to make your way deeper underground, you feel the ground rumble for a moment. Sand begins pouring into the cave! If you don't quickly figure out where the sand is going, you could quickly become trapped!

    Fortunately, your familiarity with analyzing the path of falling material will come in handy here. You scan a two-dimensional vertical slice of the cave above you (your puzzle input) and discover that it is mostly air with structures made of rock.

    Your scan traces the path of each solid rock structure and reports the x,y coordinates that form the shape of the path, where x represents distance to the right and y represents distance down. Each path appears as a single line of text in your scan. After the first point of each path, each point indicates the end of a straight horizontal or vertical line to be drawn from the previous point. For example:

    498,4 -> 498,6 -> 496,6
    503,4 -> 502,4 -> 502,9 -> 494,9
    This scan means that there are two paths of rock; the first path consists of two straight lines, and the second path consists of three straight lines. (Specifically, the first path consists of a line of rock from 498,4 through 498,6 and another line of rock from 498,6 through 496,6.)

    The sand is pouring into the cave from point 500,0.

    Drawing rock as #, air as ., and the source of the sand as +, this becomes:


      4     5  5
      9     0  0
      4     0  3
    0 ......+...
    1 ..........
    2 ..........
    3 ..........
    4 ....#...##
    5 ....#...#.
    6 ..###...#.
    7 ........#.
    8 ........#.
    9 #########.
    Sand is produced one unit at a time, and the next unit of sand is not produced until the previous unit of sand comes to rest. A unit of sand is large enough to fill one tile of air in your scan.

    A unit of sand always falls down one step if possible. If the tile immediately below is blocked (by rock or sand), the unit of sand attempts to instead move diagonally one step down and to the left. If that tile is blocked, the unit of sand attempts to instead move diagonally one step down and to the right. Sand keeps moving as long as it is able to do so, at each step trying to move down, then down-left, then down-right. If all three possible destinations are blocked, the unit of sand comes to rest and no longer moves, at which point the next unit of sand is created back at the source.

    So, drawing sand that has come to rest as o, the first unit of sand simply falls straight down and then stops:

    ......+...
    ..........
    ..........
    ..........
    ....#...##
    ....#...#.
    ..###...#.
    ........#.
    ......o.#.
    #########.
    The second unit of sand then falls straight down, lands on the first one, and then comes to rest to its left:

    ......+...
    ..........
    ..........
    ..........
    ....#...##
    ....#...#.
    ..###...#.
    ........#.
    .....oo.#.
    #########.
    After a total of five units of sand have come to rest, they form this pattern:

    ......+...
    ..........
    ..........
    ..........
    ....#...##
    ....#...#.
    ..###...#.
    ......o.#.
    ....oooo#.
    #########.
    After a total of 22 units of sand:

    ......+...
    ..........
    ......o...
    .....ooo..
    ....#ooo##
    ....#ooo#.
    ..###ooo#.
    ....oooo#.
    ...ooooo#.
    #########.
    Finally, only two more units of sand can possibly come to rest:

    ......+...
    ..........
    ......o...
    .....ooo..
    ....#ooo##
    ...o#ooo#.
    ..###ooo#.
    ....oooo#.
    .o.ooooo#.
    #########.
    Once all 24 units of sand shown above have come to rest, all further sand flows out the bottom, falling into the endless void. Just for fun, the path any new sand takes before falling forever is shown here with ~:

    .......+...
    .......~...
    ......~o...
    .....~ooo..
    ....~#ooo##
    ...~o#ooo#.
    ..~###ooo#.
    ..~..oooo#.
    .~o.ooooo#.
    ~#########.
    ~..........
    ~..........
    ~..........
    Using your scan, simulate the falling sand. How many units of sand come to rest before sand starts flowing into the abyss below?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        return std::to_string(count_until_free_flow(input));
    }

    /*
    --- Part Two ---
    You realize you misread the scan. There isn't an endless void at the bottom of the scan - there's floor, and you're standing on it!

    You don't have time to scan the floor, so assume the floor is an infinite horizontal line with a y coordinate equal to two plus the highest y coordinate of any point in your scan.

    In the example above, the highest y coordinate of any point is 9, and so the floor is at y=11. (This is as if your scan contained one extra rock path like -infinity,11 -> infinity,11.) With the added floor, the example above now looks like this:

            ...........+........
            ....................
            ....................
            ....................
            .........#...##.....
            .........#...#......
            .......###...#......
            .............#......
            .............#......
            .....#########......
            ....................
    <-- etc #################### etc -->
    To find somewhere safe to stand, you'll need to simulate falling sand until a unit of sand comes to rest at 500,0, blocking the source entirely and stopping the flow of sand into the cave. In the example above, the situation finally looks like this after 93 units of sand come to rest:

    ............o............
    ...........ooo...........
    ..........ooooo..........
    .........ooooooo.........
    ........oo#ooo##o........
    .......ooo#ooo#ooo.......
    ......oo###ooo#oooo......
    .....oooo.oooo#ooooo.....
    ....oooooooooo#oooooo....
    ...ooo#########ooooooo...
    ..ooooo.......ooooooooo..
    #########################
    Using your scan, simulate the falling sand until the source of the sand becomes blocked. How many units of sand come to rest?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        return std::to_string(count_until_reach_origin(input));//27481 too high
    }

    aoc::registration r{2022, 14, part_1, part_2};

    TEST_SUITE("2022_day14") {
        TEST_CASE("2022_day14:example") {
            const auto lines = std::vector<std::string>{
                "498,4 -> 498,6 -> 496,6",
                "503,4 -> 502,4 -> 502,9 -> 494,9"
            };
            std::vector<location> locs;
            for (const auto& l : lines) {
                const auto pts = parse_locs(l);
                locs.insert(locs.end(), pts.begin(), pts.end());
            }
            std::sort(locs.begin(), locs.end());
            locs.erase(std::unique(locs.begin(), locs.end()), locs.end());

            const auto res = count_until_reach_origin(locs);
            CHECK_EQ(res, 93);
            CHECK_EQ(std::count_if(locs.begin(), locs.end(), [](const location& l){ return !l.is_rock; }), 92);
        }
    }

}