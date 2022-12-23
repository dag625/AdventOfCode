//
// Created by Dan on 12/23/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <tuple>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<position> lines_to_positions(const std::vector<std::string>& lines) {
        auto data = lines | std::views::join | to<std::vector<char>>();
        grid<char> g {std::move(data), lines.front().size()};
        std::vector<position> retval;
        retval.reserve(g.size());
        for (const auto p : g.list_positions()) {
            if (g[p] == '#') {
                retval.push_back(p);
            }
        }
        //Remember, Y increases to the right, X increases downward
        return retval;
    }

    std::vector<position> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_23_input.txt");
        return lines_to_positions(lines);
    }

    constexpr auto NEIGHBORS = std::array<velocity, 8>{
            velocity{-1,-1},
            velocity{-1, 0},
            velocity{-1, 1},
            velocity{ 0, 1},
            velocity{ 1, 1},
            velocity{ 1, 0},
            velocity{ 1,-1},
            velocity{ 0,-1}
    };

    constexpr auto MOVE_CHECKS = std::array<std::pair<std::array<int, 3>, velocity>, 4>{
            std::pair<std::array<int, 3>, velocity>{std::array<int, 3>{0, 1, 2}, velocity{-1, 0}},
            std::pair<std::array<int, 3>, velocity>{std::array<int, 3>{4, 5, 6}, velocity{ 1, 0}},
            std::pair<std::array<int, 3>, velocity>{std::array<int, 3>{6, 7, 0}, velocity{ 0,-1}},
            std::pair<std::array<int, 3>, velocity>{std::array<int, 3>{2, 3, 4}, velocity{ 0, 1}}
    };

    struct proposed {
        position original{};
        position attempted{};
        bool stayed = true;
        proposed() = default;
        proposed(position p) : original{p}, attempted{p} {}

        [[nodiscard]] position new_pos() const { return stayed ? original : attempted; }
    };

    std::array<bool, NEIGHBORS.size()> get_neighbors(position p, const std::vector<position>& current) {
        std::array<bool, NEIGHBORS.size()> retval{};
        for (int i = 0; i < NEIGHBORS.size(); ++i) {
            const auto found = std::find(current.begin(), current.end(), p + NEIGHBORS[i]);
            retval[i] = found != current.end();
        }
        return retval;
    }

    std::pair<std::vector<position>, bool> do_round(const std::vector<position>& current, const int round) {
        std::vector<proposed> proposals;
        proposals.reserve(current.size());
        for (const auto& cp : current) {
            proposals.emplace_back(cp);
            const auto neighbors = get_neighbors(cp, current);
            if (std::all_of(neighbors.begin(), neighbors.end(), [](bool v){ return !v; })) {
                continue;
            }
            for (int i = 0; i < MOVE_CHECKS.size(); ++i) {
                const auto idx = (i + round) % MOVE_CHECKS.size();
                const auto& check = MOVE_CHECKS[idx];
                const bool available = !neighbors[check.first[0]] && !neighbors[check.first[1]] && !neighbors[check.first[2]];
                if (available) {
                    proposals.back().attempted = cp + check.second;
                    proposals.back().stayed = false;
                    break;
                }
            }
        }
        std::vector<position> retval;
        retval.reserve(current.size());
        bool any_moved = false;
        for (const auto& prop : proposals) {
            if (!prop.stayed) {
                const auto res = std::count_if(proposals.begin(), proposals.end(), [&prop](const proposed& other){ return prop.attempted == other.attempted; });
                if (res > 1) {
                    retval.push_back(prop.original);
                }
                else {
                    retval.push_back(prop.attempted);
                    any_moved = true;
                }
            }
            else {
                retval.push_back(prop.original);
            }
        }
        return {retval, any_moved};
    }

    int64_t get_bounding_area(const std::vector<position>& elves) {
        const auto [min_x, max_x] = std::minmax_element(elves.begin(), elves.end(), [](position a, position b){ return a.x < b.x; });
        const auto [min_y, max_y] = std::minmax_element(elves.begin(), elves.end(), [](position a, position b){ return a.y < b.y; });
        return (max_x->x - min_x->x + 1) * (max_y->y - min_y->y + 1);
    }

    /*
    --- Day 23: Unstable Diffusion ---
    You enter a large crater of gray dirt where the grove is supposed to be. All around you, plants you imagine were expected to be full of fruit are instead withered and broken. A large group of Elves has formed in the middle of the grove.

    "...but this volcano has been dormant for months. Without ash, the fruit can't grow!"

    You look up to see a massive, snow-capped mountain towering above you.

    "It's not like there are other active volcanoes here; we've looked everywhere."

    "But our scanners show active magma flows; clearly it's going somewhere."

    They finally notice you at the edge of the grove, your pack almost overflowing from the random star fruit you've been collecting. Behind you, elephants and monkeys explore the grove, looking concerned. Then, the Elves recognize the ash cloud slowly spreading above your recent detour.

    "Why do you--" "How is--" "Did you just--"

    Before any of them can form a complete question, another Elf speaks up: "Okay, new plan. We have almost enough fruit already, and ash from the plume should spread here eventually. If we quickly plant new seedlings now, we can still make it to the extraction point. Spread out!"

    The Elves each reach into their pack and pull out a tiny plant. The plants rely on important nutrients from the ash, so they can't be planted too close together.

    There isn't enough time to let the Elves figure out where to plant the seedlings themselves; you quickly scan the grove (your puzzle input) and note their positions.

    For example:

    ....#..
    ..###.#
    #...#.#
    .#...##
    #.###..
    ##.#.##
    .#..#..
    The scan shows Elves # and empty ground .; outside your scan, more empty ground extends a long way in every direction. The scan is oriented so that north is up; orthogonal directions are written N (north), S (south), W (west), and E (east), while diagonal directions are written NE, NW, SE, SW.

    The Elves follow a time-consuming process to figure out where they should each go; you can speed up this process considerably. The process consists of some number of rounds during which Elves alternate between considering where to move and actually moving.

    During the first half of each round, each Elf considers the eight positions adjacent to themself. If no other Elves are in one of those eight positions, the Elf does not do anything during this round. Otherwise, the Elf looks in each of four directions in the following order and proposes moving one step in the first valid direction:

    If there is no Elf in the N, NE, or NW adjacent positions, the Elf proposes moving north one step.
    If there is no Elf in the S, SE, or SW adjacent positions, the Elf proposes moving south one step.
    If there is no Elf in the W, NW, or SW adjacent positions, the Elf proposes moving west one step.
    If there is no Elf in the E, NE, or SE adjacent positions, the Elf proposes moving east one step.
    After each Elf has had a chance to propose a move, the second half of the round can begin. Simultaneously, each Elf moves to their proposed destination tile if they were the only Elf to propose moving to that position. If two or more Elves propose moving to the same position, none of those Elves move.

    Finally, at the end of the round, the first direction the Elves considered is moved to the end of the list of directions. For example, during the second round, the Elves would try proposing a move to the south first, then west, then east, then north. On the third round, the Elves would first consider west, then east, then north, then south.

    As a smaller example, consider just these five Elves:

    .....
    ..##.
    ..#..
    .....
    ..##.
    .....
    The northernmost two Elves and southernmost two Elves all propose moving north, while the middle Elf cannot move north and proposes moving south. The middle Elf proposes the same destination as the southwest Elf, so neither of them move, but the other three do:

    ..##.
    .....
    ..#..
    ...#.
    ..#..
    .....
    Next, the northernmost two Elves and the southernmost Elf all propose moving south. Of the remaining middle two Elves, the west one cannot move south and proposes moving west, while the east one cannot move south or west and proposes moving east. All five Elves succeed in moving to their proposed positions:

    .....
    ..##.
    .#...
    ....#
    .....
    ..#..
    Finally, the southernmost two Elves choose not to move at all. Of the remaining three Elves, the west one proposes moving west, the east one proposes moving east, and the middle one proposes moving north; all three succeed in moving:

    ..#..
    ....#
    #....
    ....#
    .....
    ..#..
    At this point, no Elves need to move, and so the process ends.

    The larger example above proceeds as follows:

    == Initial State ==
    ..............
    ..............
    .......#......
    .....###.#....
    ...#...#.#....
    ....#...##....
    ...#.###......
    ...##.#.##....
    ....#..#......
    ..............
    ..............
    ..............

    == End of Round 1 ==
    ..............
    .......#......
    .....#...#....
    ...#..#.#.....
    .......#..#...
    ....#.#.##....
    ..#..#.#......
    ..#.#.#.##....
    ..............
    ....#..#......
    ..............
    ..............

    == End of Round 2 ==
    ..............
    .......#......
    ....#.....#...
    ...#..#.#.....
    .......#...#..
    ...#..#.#.....
    .#...#.#.#....
    ..............
    ..#.#.#.##....
    ....#..#......
    ..............
    ..............

    == End of Round 3 ==
    ..............
    .......#......
    .....#....#...
    ..#..#...#....
    .......#...#..
    ...#..#.#.....
    .#..#.....#...
    .......##.....
    ..##.#....#...
    ...#..........
    .......#......
    ..............

    == End of Round 4 ==
    ..............
    .......#......
    ......#....#..
    ..#...##......
    ...#.....#.#..
    .........#....
    .#...###..#...
    ..#......#....
    ....##....#...
    ....#.........
    .......#......
    ..............

    == End of Round 5 ==
    .......#......
    ..............
    ..#..#.....#..
    .........#....
    ......##...#..
    .#.#.####.....
    ...........#..
    ....##..#.....
    ..#...........
    ..........#...
    ....#..#......
    ..............
    After a few more rounds...

    == End of Round 10 ==
    .......#......
    ...........#..
    ..#.#..#......
    ......#.......
    ...#.....#..#.
    .#......##....
    .....##.......
    ..#........#..
    ....#.#..#....
    ..............
    ....#..#..#...
    ..............
    To make sure they're on the right track, the Elves like to check after round 10 that they're making good progress toward covering enough ground. To do this, count the number of empty ground tiles contained by the smallest rectangle that contains every Elf. (The edges of the rectangle should be aligned to the N/S/E/W directions; the Elves do not have the patience to calculate arbitrary rectangles.) In the above example, that rectangle is:

    ......#.....
    ..........#.
    .#.#..#.....
    .....#......
    ..#.....#..#
    #......##...
    ....##......
    .#........#.
    ...#.#..#...
    ............
    ...#..#..#..
    In this region, the number of empty ground tiles is 110.

    Simulate the Elves' process and find the smallest rectangle that contains the Elves after 10 rounds. How many empty ground tiles does that rectangle contain?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        auto elves = get_input(input_dir);
        for (int round = 0; round < 10; ++round) {
            bool any_moved = false;
            std::tie(elves, any_moved) = do_round(elves, round);
        }
        return std::to_string(get_bounding_area(elves) - elves.size());
    }

    /*
    --- Part Two ---
    It seems you're on the right track. Finish simulating the process and figure out where the Elves need to go. How many rounds did you save them?

    In the example above, the first round where no Elf moved was round 20:

    .......#......
    ....#......#..
    ..#.....#.....
    ......#.......
    ...#....#.#..#
    #.............
    ....#.....#...
    ..#.....#.....
    ....#.#....#..
    .........#....
    ....#......#..
    .......#......
    Figure out where the Elves need to go. What is the number of the first round where no Elf moves?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        auto elves = get_input(input_dir);
        bool any_moved = true;
        int round = 0;
        for (; any_moved; ++round) {
            std::tie(elves, any_moved) = do_round(elves, round);
        }
        return std::to_string(round);
    }

    aoc::registration r{2022, 23, part_1, part_2};

    TEST_SUITE("2022_day23") {
        TEST_CASE("2022_day23:example1") {
            std::vector<std::string> lines {
                ".....",
                "..##.",
                "..#..",
                ".....",
                "..##.",
                "....."
            };
            auto elves = lines_to_positions(lines);
            for (int round = 0; round < 10; ++round) {
                bool any_moved = false;
                std::tie(elves, any_moved) = do_round(elves, round);
            }
            CHECK_EQ(get_bounding_area(elves) - elves.size(), 25);
        }
    }

}