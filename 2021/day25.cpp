//
// Created by Dan on 12/25/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct spot {
        char current = '.';
        char next = '.';

        spot() = default;
        explicit spot(char c) : current{c}, next{c} {}
        void update() { current = next; }
    };

    std::ostream& operator<<(std::ostream& os, const spot& s) {
        os << s.current;
        return os;
    }

    grid<spot> to_spots(const grid<char>& gc) {
        grid<spot> retval {gc.num_rows(), gc.num_cols()};
        for (const auto pos : gc.list_positions()) {
            retval[pos] = spot{gc[pos]};
        }
        return retval;
    }

    grid<spot> get_input(const std::vector<std::string>& lines) {
        return to_spots(to_grid(lines));
    }

    int iterate(grid<spot>& g, const velocity del, const char to_move) {
        using std::swap;
        int moved = 0;
        for (const auto pos : g.list_positions()) {
            const auto next = g.wrap_any(pos + del);
            if (g[pos].current == to_move && g[next].current == '.') {
                ++moved;
                g[pos].next = '.';
                g[next].next = to_move;
            }
        }
        std::for_each(g.begin(), g.end(), [](spot& s){ s.update(); });
        return moved;
    }

    int do_step(grid<spot>& g) {
        int moved = 0;
        moved += iterate(g, {0, 1}, '>');
        moved += iterate(g, {1, 0}, 'v');
        return moved;
    }

    int step_until_stopped(grid<spot>& g) {
        int step = 0, moved = -1;
//        fmt::print("Initial state:\n");
//        g.display(std::cout, std::nullopt, 1);
//        fmt::print("\n");
        while (moved != 0) {
            moved = do_step(g);
            step++;
//            fmt::print("After step {}:\n", step);
//            g.display(std::cout, std::nullopt, 1);
//            fmt::print("\n");
        }
        return step;
    }

    /*
    --- Day 25: Sea Cucumber ---
    This is it: the bottom of the ocean trench, the last place the sleigh keys could be. Your submarine's experimental antenna still isn't boosted enough to detect the keys, but they must be here. All you need to do is reach the seafloor and find them.

    At least, you'd touch down on the seafloor if you could; unfortunately, it's completely covered by two large herds of sea cucumbers, and there isn't an open space large enough for your submarine.

    You suspect that the Elves must have done this before, because just then you discover the phone number of a deep-sea marine biologist on a handwritten note taped to the wall of the submarine's cockpit.

    "Sea cucumbers? Yeah, they're probably hunting for food. But don't worry, they're predictable critters: they move in perfectly straight lines, only moving forward when there's space to do so. They're actually quite polite!"

    You explain that you'd like to predict when you could land your submarine.

    "Oh that's easy, they'll eventually pile up and leave enough space for-- wait, did you say submarine? And the only place with that many sea cucumbers would be at the very bottom of the Mariana--" You hang up the phone.

    There are two herds of sea cucumbers sharing the same region; one always moves east (>), while the other always moves south (v). Each location can contain at most one sea cucumber; the remaining locations are empty (.). The submarine helpfully generates a map of the situation (your puzzle input). For example:

    v...>>.vv>
    .vv>>.vv..
    >>.>v>...v
    >>v>>.>.v.
    v>v.vv.v..
    >.>>..v...
    .vv..>.>v.
    v.v..>>v.v
    ....v..v.>
    Every step, the sea cucumbers in the east-facing herd attempt to move forward one location, then the sea cucumbers in the south-facing herd attempt to move forward one location. When a herd moves forward, every sea cucumber in the herd first simultaneously considers whether there is a sea cucumber in the adjacent location it's facing (even another sea cucumber facing the same direction), and then every sea cucumber facing an empty location simultaneously moves into that location.

    So, in a situation like this:

    ...>>>>>...
    After one step, only the rightmost sea cucumber would have moved:

    ...>>>>.>..
    After the next step, two sea cucumbers move:

    ...>>>.>.>.
    During a single step, the east-facing herd moves first, then the south-facing herd moves. So, given this situation:

    ..........
    .>v....v..
    .......>..
    ..........
    After a single step, of the sea cucumbers on the left, only the south-facing sea cucumber has moved (as it wasn't out of the way in time for the east-facing cucumber on the left to move), but both sea cucumbers on the right have moved (as the east-facing sea cucumber moved out of the way of the south-facing sea cucumber):

    ..........
    .>........
    ..v....v>.
    ..........
    Due to strong water currents in the area, sea cucumbers that move off the right edge of the map appear on the left edge, and sea cucumbers that move off the bottom edge of the map appear on the top edge. Sea cucumbers always check whether their destination location is empty before moving, even if that destination is on the opposite side of the map:

    Initial state:
    ...>...
    .......
    ......>
    v.....>
    ......>
    .......
    ..vvv..

    After 1 step:
    ..vv>..
    .......
    >......
    v.....>
    >......
    .......
    ....v..

    After 2 steps:
    ....v>.
    ..vv...
    .>.....
    ......>
    v>.....
    .......
    .......

    After 3 steps:
    ......>
    ..v.v..
    ..>v...
    >......
    ..>....
    v......
    .......

    After 4 steps:
    >......
    ..v....
    ..>.v..
    .>.v...
    ...>...
    .......
    v......
    To find a safe place to land your submarine, the sea cucumbers need to stop moving. Again consider the first example:

    Initial state:
    v...>>.vv>
    .vv>>.vv..
    >>.>v>...v
    >>v>>.>.v.
    v>v.vv.v..
    >.>>..v...
    .vv..>.>v.
    v.v..>>v.v
    ....v..v.>

    After 1 step:
    ....>.>v.>
    v.v>.>v.v.
    >v>>..>v..
    >>v>v>.>.v
    .>v.v...v.
    v>>.>vvv..
    ..v...>>..
    vv...>>vv.
    >.v.v..v.v

    After 2 steps:
    >.v.v>>..v
    v.v.>>vv..
    >v>.>.>.v.
    >>v>v.>v>.
    .>..v....v
    .>v>>.v.v.
    v....v>v>.
    .vv..>>v..
    v>.....vv.

    After 3 steps:
    v>v.v>.>v.
    v...>>.v.v
    >vv>.>v>..
    >>v>v.>.v>
    ..>....v..
    .>.>v>v..v
    ..v..v>vv>
    v.v..>>v..
    .v>....v..

    After 4 steps:
    v>..v.>>..
    v.v.>.>.v.
    >vv.>>.v>v
    >>.>..v>.>
    ..v>v...v.
    ..>>.>vv..
    >.v.vv>v.v
    .....>>vv.
    vvv>...v..

    After 5 steps:
    vv>...>v>.
    v.v.v>.>v.
    >.v.>.>.>v
    >v>.>..v>>
    ..v>v.v...
    ..>.>>vvv.
    .>...v>v..
    ..v.v>>v.v
    v.v.>...v.

    ...

    After 10 steps:
    ..>..>>vv.
    v.....>>.v
    ..v.v>>>v>
    v>.>v.>>>.
    ..v>v.vv.v
    .v.>>>.v..
    v.v..>v>..
    ..v...>v.>
    .vv..v>vv.

    ...

    After 20 steps:
    v>.....>>.
    >vv>.....v
    .>v>v.vv>>
    v>>>v.>v.>
    ....vv>v..
    .v.>>>vvv.
    ..v..>>vv.
    v.v...>>.v
    ..v.....v>

    ...

    After 30 steps:
    .vv.v..>>>
    v>...v...>
    >.v>.>vv.>
    >v>.>.>v.>
    .>..v.vv..
    ..v>..>>v.
    ....v>..>v
    v.v...>vv>
    v.v...>vvv

    ...

    After 40 steps:
    >>v>v..v..
    ..>>v..vv.
    ..>>>v.>.v
    ..>>>>vvv>
    v.....>...
    v.v...>v>>
    >vv.....v>
    .>v...v.>v
    vvv.v..v.>

    ...

    After 50 steps:
    ..>>v>vv.v
    ..v.>>vv..
    v.>>v>>v..
    ..>>>>>vv.
    vvv....>vv
    ..v....>>>
    v>.......>
    .vv>....v>
    .>v.vv.v..

    ...

    After 55 steps:
    ..>>v>vv..
    ..v.>>vv..
    ..>>v>>vv.
    ..>>>>>vv.
    v......>vv
    v>v....>>v
    vvv...>..>
    >vv.....>.
    .>v.vv.v..

    After 56 steps:
    ..>>v>vv..
    ..v.>>vv..
    ..>>v>>vv.
    ..>>>>>vv.
    v......>vv
    v>v....>>v
    vvv....>.>
    >vv......>
    .>v.vv.v..

    After 57 steps:
    ..>>v>vv..
    ..v.>>vv..
    ..>>v>>vv.
    ..>>>>>vv.
    v......>vv
    v>v....>>v
    vvv.....>>
    >vv......>
    .>v.vv.v..

    After 58 steps:
    ..>>v>vv..
    ..v.>>vv..
    ..>>v>>vv.
    ..>>>>>vv.
    v......>vv
    v>v....>>v
    vvv.....>>
    >vv......>
    .>v.vv.v..
    In this example, the sea cucumbers stop moving after 58 steps.

    Find somewhere safe to land your submarine. What is the first step on which no sea cucumbers move?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        //384 - too low
        return std::to_string(step_until_stopped(input));
    }

    /*
    --- Part Two ---
    Suddenly, the experimental antenna control console lights up:

    Sleigh keys detected!
    According to the console, the keys are directly under the submarine. You landed right on them! Using a robotic arm on the submarine, you move the sleigh keys into the airlock.

    Now, you just need to get them to Santa in time to save Christmas! You check your clock - it is Christmas. There's no way you can get them back to the surface in time.

    Just as you start to lose hope, you notice a button on the sleigh keys: remote start. You can start the sleigh from the bottom of the ocean! You just need some way to boost the signal from the keys so it actually reaches the sleigh. Good thing the submarine has that experimental antenna! You'll definitely need 50 stars to boost it that far, though.

    The experimental antenna control console lights up again:

    Energy source detected.
    Integrating energy source from device "sleigh keys"...done.
    Installing device drivers...done.
    Recalibrating experimental antenna...done.
    Boost strength due to matching signal phase: 1 star
    Only 49 stars to go.
    */

    /*
    You use all fifty stars to boost the signal and remotely start the sleigh! Now, you just have to find your way back to the surface...

    ...did you know crab submarines come with colored lights?
    */

    aoc::registration r{2021, 25, part_1};

    TEST_SUITE("2021_day25") {
        TEST_CASE("2021_day25:example") {
            std::vector<std::string> lines = {
                    "v...>>.vv>",
                    ".vv>>.vv..",
                    ">>.>v>...v",
                    ">>v>>.>.v.",
                    "v>v.vv.v..",
                    ">.>>..v...",
                    ".vv..>.>v.",
                    "v.v..>>v.v",
                    "....v..v.>"
            };
            auto input = to_spots(to_grid(lines));
            auto num_steps = step_until_stopped(input);
            REQUIRE_EQ(num_steps, 58);
        }
    }

}