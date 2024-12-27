//
// Created by Dan on 12/13/2024.
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

    /*
     See https://adventofcode.com/2017/day/22
     */

    grid<char> get_input(const std::vector<std::string>& lines) {
        return to_grid(lines);
    }

    std::vector<position> get_infected(const grid<char>& g) {
        std::vector<position> retval;
        for (const auto p : g.list_positions()) {
            if (g[p] == '#') {
                retval.push_back(p);
            }
        }
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    struct carrier {
        position pos;
        velocity dir;
    };

    carrier get_init(const grid<char>& g) {
        return {{static_cast<int>(g.num_rows()) / 2, static_cast<int>(g.num_cols()) / 2}, {-1, 0}};
    }

    bool burst1(carrier& current, std::vector<position>& infected) {
        const auto cfound = std::lower_bound(infected.begin(), infected.end(), current.pos);
        bool caused_infection = false;
        if (cfound == infected.end() || *cfound != current.pos) {
            //Clean, turn left
            current.dir = {-current.dir.dy, current.dir.dx};
            infected.insert(cfound, current.pos);
            caused_infection = true;
        }
        else {
            //Infected, turn right
            current.dir = {current.dir.dy, -current.dir.dx};
            infected.erase(cfound);
        }
        current.pos += current.dir;
        return caused_infection;
    }

    enum InfectionLevel {
        Clean = 0,
        Weakened = 1,
        Infected = 2,
        Flagged = 3,
        NumLevels = 4
    };

    struct state {
        position pos;
        int level = Infected;

        state() = default;
        state(const position p) : pos{p} {}
        state(const position p, int l) : pos{p}, level{l} {}

        auto operator<=>(const state& rhs) const { return pos <=> rhs.pos; }
        bool operator==(const state& rhs) const { return pos == rhs.pos; }

        auto operator<=>(const position rhs) const { return pos <=> rhs; }
        bool operator==(const position rhs) const { return pos == rhs; }
    };

    std::vector<state> to_states(const std::vector<position>& raw) {
        return raw | std::views::transform([](const position p){ return state{p}; }) | std::ranges::to<std::vector>();
    }

    bool burst2(carrier& current, std::vector<state>& infected) {
        const auto cfound = std::lower_bound(infected.begin(), infected.end(), current.pos);
        bool caused_infection = false;
        if (cfound == infected.end() || *cfound != current.pos) {
            //Clean, turn left
            current.dir = {-current.dir.dy, current.dir.dx};
            infected.emplace(cfound, current.pos, Weakened);
        }
        else if (cfound->level == Weakened) {
            //Weakened, same direction
            ++(cfound->level);
            caused_infection = true;
        }
        else if (cfound->level == Infected) {
            //Infected, turn right
            current.dir = {current.dir.dy, -current.dir.dx};
            ++(cfound->level);
        }
        else if (cfound->level == Flagged) {
            //Flagged, reverse
            current.dir = {-current.dir.dx, -current.dir.dy};
            cfound->level = Clean;
        }
        else {
            //Clean, turn left
            current.dir = {-current.dir.dy, current.dir.dx};
            ++(cfound->level);
        }
        current.pos += current.dir;
        return caused_infection;
    }

    void print_inf(const int idx, const std::vector<state>& infected, const int num) {
        fmt::println("At t={:8}, there are {} infected ({} in list):", idx, num, infected.size());
        for (int i = 0; i < 10 && i < infected.size(); ++i) {
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", i, infected[i].pos.x, infected[i].pos.y);
        }
        if (infected.size() > 100) {
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 20, infected[20].pos.x, infected[20].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 30, infected[30].pos.x, infected[30].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 40, infected[40].pos.x, infected[40].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 50, infected[50].pos.x, infected[50].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 60, infected[60].pos.x, infected[60].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 70, infected[60].pos.x, infected[70].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 80, infected[80].pos.x, infected[80].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 90, infected[90].pos.x, infected[90].pos.y);
            fmt::println("\tinf[{:3}] = ({:8}, {:8})", 100, infected[100].pos.x, infected[100].pos.y);
        }
        std::cout << std::endl;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto infected = get_infected(input);
        auto current = get_init(input);
        int num = 0;
        for (int i = 0; i < 10'000; ++i) {
            const bool caused_infection = burst1(current, infected);
            num += static_cast<int>(caused_infection);
        }
        return std::to_string(num);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto infected = to_states(get_infected(input));
        auto current = get_init(input);
        int num = 0;
        for (int i = 0; i < 10'000'000; ++i) {
            const bool caused_infection = burst2(current, infected);
            num += static_cast<int>(caused_infection);
        }
        return std::to_string(num);
    }

    aoc::registration r{2017, 22, part_1, part_2};

    TEST_SUITE("2017_day22") {
        TEST_CASE("2017_day22:example") {
            const std::vector<std::string> lines {
                    "..#",
                    "#..",
                    "..."
            };
            const auto input = to_grid(lines);
            auto infected = get_infected(input);
            auto current = get_init(input);
            int num = 0;
            for (int i = 0; i < 10'000; ++i) {
                if (i == 70) {
                    CHECK_EQ(num, 41);
                }
                const bool caused_infection = burst1(current, infected);
                num += static_cast<int>(caused_infection);
            }
            CHECK_EQ(num, 5587);


            auto infected2 = to_states(get_infected(input));
            auto current2 = get_init(input);
            int num2 = 0;
            for (int i = 0; i < 10'000'000; ++i) {
                if (i == 100) {
                    CHECK_EQ(num2, 26);
                }
                const bool caused_infection = burst2(current2, infected2);
                num2 += static_cast<int>(caused_infection);
                if (i < 200) {
                    print_inf(i+1, infected2, num2);
                }
            }
            CHECK_EQ(num2, 2511944);
        }
    }

} /* namespace <anon> */