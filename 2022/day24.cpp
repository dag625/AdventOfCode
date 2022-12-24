//
// Created by Dan on 12/24/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <execution>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr position START_POS {-1, 0};
    constexpr position GOAL_POS  {35, 99};
    constexpr int WALL_X = 35;
    constexpr int WALL_Y = 100;
    //This is the smallest number that both 100 and 35 divide, which means the position of the blizzards repeats at this interval.
    constexpr int REPEAT_STATE_INTERVAL = 700;

    struct blizzard {
        position start;
        velocity move;

        std::strong_ordering operator<=>(const blizzard& rhs) const noexcept {
            return std::tie(start.x, start.y) <=> std::tie(rhs.start.x, rhs.start.y);
        }
        std::strong_ordering operator<=>(position rhs) const noexcept {
            return std::tie(start.x, start.y) <=> std::tie(rhs.x, rhs.y);
        }
    };

    position adjust(position p) {
        static velocity d {-1,-1};
        return p + d;
    }

    velocity to_move(char c) {
        switch (c) {
            case '<': return velocity{ 0,-1};
            case '>': return velocity{ 0, 1};
            case '^': return velocity{-1, 0};
            case 'v': return velocity{ 1, 0};
            default: return velocity{};
        }
    }

    constexpr std::array<velocity, 5> DIRECTIONS {
            velocity{ 0, 1},
            velocity{ 1, 0},
            velocity{ 0,-1},
            velocity{-1, 0},
            velocity{ 0, 0}
    };

    std::vector<blizzard> lines_to_input(const std::vector<std::string>& lines) {
        std::vector<char> data;
        data.reserve(lines.size() * lines[0].size());
        for (const auto& l : lines) {
            data.insert(data.end(), l.begin(), l.end());
        }
        grid<char> g {std::move(data), lines[0].size()};
        std::vector<blizzard> retval;
        for (const auto p : g.list_positions()) {
            if (g[p] != '.' && g[p] != '#') {
                retval.push_back({adjust(p), to_move(g[p])});
            }
        }
        return retval;
    }

    std::vector<blizzard> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_24_input.txt");
        return lines_to_input(lines);
    }

    void adjust_pos(int& coord, const int wall) {
        if (coord >= 0) {
            coord %= wall;
            return;
        }
        while (coord < 0) {
            coord += wall;
        }
    }

    position pos_at_time(const blizzard& b, int time, const int wall_x, const int wall_y) {
        position retval = b.start + b.move * time;
        adjust_pos(retval.x, wall_x);
        adjust_pos(retval.y, wall_y);
        return retval;
    }

    std::vector<std::vector<position>> precompute(const std::vector<blizzard>& blizzards, const int bs_repeat, const int wall_x, const int wall_y) {
        std::vector<std::vector<position>> retval;
        retval.reserve(bs_repeat);
        for (int i = 0; i < bs_repeat; ++i) {
            auto round = blizzards |
                    std::views::transform([i, wall_x, wall_y](const blizzard& b){ return pos_at_time(b, i, wall_x, wall_y); }) |
                    to<std::vector<position>>();
            std::sort(round.begin(), round.end());
            round.erase(std::unique(round.begin(), round.end()), round.end());
            retval.push_back(std::move(round));
        }
        return retval;
    }

    bool is_in(position p, const int wall_x, const int wall_y) {
        return p.x >= 0 && p.x < wall_x && p.y >= 0 && p.y < wall_y;
    }

    bool is_blocked(position p, int time, const std::vector<blizzard>& bs, const int wall_x, const int wall_y) {
        return std::any_of(std::execution::seq, bs.begin(), bs.end(),
                           [p, time, wall_x, wall_y](const blizzard& b){ return p == pos_at_time(b, time, wall_x, wall_y); });
    }

    int heuristic(const position a, const position goal) {
        const auto v = goal - a;
        //The performance impact between the two distances below is negligable for the uses so far.
        return static_cast<int>(std::round(std::sqrt(v.x * v.x + v.y * v.y)));//Triangular distance
        //return v.x + v.y;//"Manhattan" distance
    }

    struct info {
        position pos;
        int h_score = 0;
        int time = 0;

        info(position p, int t, const position goal) : pos{p}, time{t}, h_score{heuristic(p, goal)} {}

        bool operator==(const info& rhs) const noexcept {
            return std::tie(time, pos) == std::tie(rhs.time, rhs.pos);
        }
        bool operator<(const info& rhs) const noexcept {
            //Want a min-heap, but the std-library algorithms create a max heap.
            return std::tie(time, h_score) > std::tie(rhs.time, rhs.h_score);
        }
    };

    int find_path(const std::vector<std::vector<position>>& bs, const int start_time, const position start, const position goal, const int wall_x, const int wall_y) {
        std::vector<info> heap;
        heap.reserve(wall_x * wall_y);
        heap.emplace_back(start, start_time, goal);

        while (!heap.empty()) {
            const auto current = heap.front();
            std::pop_heap(heap.begin(), heap.end());
            heap.erase(heap.end() - 1);
            const auto next_time = current.time + 1;
            const auto bs_index = next_time % bs.size();

            for (const auto& v : DIRECTIONS) {
                info next {current.pos + v, next_time, goal};
                bool in = is_in(next.pos, wall_x, wall_y);
                const auto found_block = std::lower_bound(bs[bs_index].begin(), bs[bs_index].end(), next.pos);
                bool blocked = found_block != bs[bs_index].end() && *found_block == next.pos;
                if (next.pos == goal) {
                    return next_time;
                }
                else if ((in || next.pos == start) && !blocked) {
                    const auto found = std::find_if(heap.begin(), heap.end(), 
                        [p = next.pos, next_time](const info& i) { return i.pos == p && i.time == next_time; });
                    if (found == heap.end()) {
                        heap.push_back(next);
                        std::push_heap(heap.begin(), heap.end());
                    }
                }
            }

        }
        return std::numeric_limits<int>::max();
    }

    /*
    --- Day 24: Blizzard Basin ---
    With everything replanted for next year (and with elephants and monkeys to tend the grove), you and the Elves leave for the extraction point.

    Partway up the mountain that shields the grove is a flat, open area that serves as the extraction point. It's a bit of a climb, but nothing the expedition can't handle.

    At least, that would normally be true; now that the mountain is covered in snow, things have become more difficult than the Elves are used to.

    As the expedition reaches a valley that must be traversed to reach the extraction site, you find that strong, turbulent winds are pushing small blizzards of snow and sharp ice around the valley. It's a good thing everyone packed warm clothes! To make it across safely, you'll need to find a way to avoid them.

    Fortunately, it's easy to see all of this from the entrance to the valley, so you make a map of the valley and the blizzards (your puzzle input). For example:

    #.#####
    #.....#
    #>....#
    #.....#
    #...v.#
    #.....#
    #####.#
    The walls of the valley are drawn as #; everything else is ground. Clear ground - where there is currently no blizzard - is drawn as .. Otherwise, blizzards are drawn with an arrow indicating their direction of motion: up (^), down (v), left (<), or right (>).

    The above map includes two blizzards, one moving right (>) and one moving down (v). In one minute, each blizzard moves one position in the direction it is pointing:

    #.#####
    #.....#
    #.>...#
    #.....#
    #.....#
    #...v.#
    #####.#
    Due to conservation of blizzard energy, as a blizzard reaches the wall of the valley, a new blizzard forms on the opposite side of the valley moving in the same direction. After another minute, the bottom downward-moving blizzard has been replaced with a new downward-moving blizzard at the top of the valley instead:

    #.#####
    #...v.#
    #..>..#
    #.....#
    #.....#
    #.....#
    #####.#
    Because blizzards are made of tiny snowflakes, they pass right through each other. After another minute, both blizzards temporarily occupy the same position, marked 2:

    #.#####
    #.....#
    #...2.#
    #.....#
    #.....#
    #.....#
    #####.#
    After another minute, the situation resolves itself, giving each blizzard back its personal space:

    #.#####
    #.....#
    #....>#
    #...v.#
    #.....#
    #.....#
    #####.#
    Finally, after yet another minute, the rightward-facing blizzard on the right is replaced with a new one on the left facing the same direction:

    #.#####
    #.....#
    #>....#
    #.....#
    #...v.#
    #.....#
    #####.#
    This process repeats at least as long as you are observing it, but probably forever.

    Here is a more complex example:

    #.######
    #>>.<^<#
    #.<..<<#
    #>v.><>#
    #<^v^^>#
    ######.#
    Your expedition begins in the only non-wall position in the top row and needs to reach the only non-wall position in the bottom row. On each minute, you can move up, down, left, or right, or you can wait in place. You and the blizzards act simultaneously, and you cannot share a position with a blizzard.

    In the above example, the fastest way to reach your goal requires 18 steps. Drawing the position of the expedition as E, one way to achieve this is:

    Initial state:
    #E######
    #>>.<^<#
    #.<..<<#
    #>v.><>#
    #<^v^^>#
    ######.#

    Minute 1, move down:
    #.######
    #E>3.<.#
    #<..<<.#
    #>2.22.#
    #>v..^<#
    ######.#

    Minute 2, move down:
    #.######
    #.2>2..#
    #E^22^<#
    #.>2.^>#
    #.>..<.#
    ######.#

    Minute 3, wait:
    #.######
    #<^<22.#
    #E2<.2.#
    #><2>..#
    #..><..#
    ######.#

    Minute 4, move up:
    #.######
    #E<..22#
    #<<.<..#
    #<2.>>.#
    #.^22^.#
    ######.#

    Minute 5, move right:
    #.######
    #2Ev.<>#
    #<.<..<#
    #.^>^22#
    #.2..2.#
    ######.#

    Minute 6, move right:
    #.######
    #>2E<.<#
    #.2v^2<#
    #>..>2>#
    #<....>#
    ######.#

    Minute 7, move down:
    #.######
    #.22^2.#
    #<vE<2.#
    #>>v<>.#
    #>....<#
    ######.#

    Minute 8, move left:
    #.######
    #.<>2^.#
    #.E<<.<#
    #.22..>#
    #.2v^2.#
    ######.#

    Minute 9, move up:
    #.######
    #<E2>>.#
    #.<<.<.#
    #>2>2^.#
    #.v><^.#
    ######.#

    Minute 10, move right:
    #.######
    #.2E.>2#
    #<2v2^.#
    #<>.>2.#
    #..<>..#
    ######.#

    Minute 11, wait:
    #.######
    #2^E^2>#
    #<v<.^<#
    #..2.>2#
    #.<..>.#
    ######.#

    Minute 12, move down:
    #.######
    #>>.<^<#
    #.<E.<<#
    #>v.><>#
    #<^v^^>#
    ######.#

    Minute 13, move down:
    #.######
    #.>3.<.#
    #<..<<.#
    #>2E22.#
    #>v..^<#
    ######.#

    Minute 14, move right:
    #.######
    #.2>2..#
    #.^22^<#
    #.>2E^>#
    #.>..<.#
    ######.#

    Minute 15, move right:
    #.######
    #<^<22.#
    #.2<.2.#
    #><2>E.#
    #..><..#
    ######.#

    Minute 16, move right:
    #.######
    #.<..22#
    #<<.<..#
    #<2.>>E#
    #.^22^.#
    ######.#

    Minute 17, move down:
    #.######
    #2.v.<>#
    #<.<..<#
    #.^>^22#
    #.2..2E#
    ######.#

    Minute 18, move down:
    #.######
    #>2.<.<#
    #.2v^2<#
    #>..>2>#
    #<....>#
    ######E#
    What is the fewest number of minutes required to avoid the blizzards and reach the goal?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto precomputed_blizzards = precompute(input, REPEAT_STATE_INTERVAL, WALL_X, WALL_Y);
        return std::to_string(find_path(precomputed_blizzards, 0, START_POS, GOAL_POS, WALL_X, WALL_Y));
    }

    /*
    --- Part Two ---
    As the expedition reaches the far side of the valley, one of the Elves looks especially dismayed:

    He forgot his snacks at the entrance to the valley!

    Since you're so good at dodging blizzards, the Elves humbly request that you go back for his snacks. From the same initial conditions, how quickly can you make it from the start to the goal, then back to the start, then back to the goal?

    In the above example, the first trip to the goal takes 18 minutes, the trip back to the start takes 23 minutes, and the trip back to the goal again takes 13 minutes, for a total time of 54 minutes.

    What is the fewest number of minutes required to reach the goal, go back to the start, then reach the goal again?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto precomputed_blizzards = precompute(input, REPEAT_STATE_INTERVAL, WALL_X, WALL_Y);
        int trip1 = find_path(precomputed_blizzards, 0, START_POS, GOAL_POS, WALL_X, WALL_Y);
        int trip2 = find_path(precomputed_blizzards, trip1, GOAL_POS, START_POS, WALL_X, WALL_Y);
        int trip3 = find_path(precomputed_blizzards, trip2, START_POS, GOAL_POS, WALL_X, WALL_Y);
        return std::to_string(trip3);
    }

    aoc::registration r{2022, 24, part_1, part_2};

    TEST_SUITE("2022_day24") {
        TEST_CASE("2022_day24:example") {
            const std::vector<std::string> lines {
                    "#.######",
                    "#>>.<^<#",
                    "#.<..<<#",
                    "#>v.><>#",
                    "#<^v^^>#",
                    "######.#"
            };
            const auto input = lines_to_input(lines);
            const auto precomputed_blizzards = precompute(input, 12, 4, 6);
            const auto result = find_path(precomputed_blizzards, 0, START_POS, {4,5}, 4, 6);
            CHECK_EQ(result, 18);
        }
    }

}