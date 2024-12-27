//
// Created by Dan on 12/19/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <optional>
#include <map>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr std::size_t ORE_R_IDX = 0;
    constexpr std::size_t CLAY_R_IDX = 1;
    constexpr std::size_t OBSDN_R_IDX = 2;
    constexpr std::size_t GEO_R_IDX = 3;
    constexpr std::size_t NONE_R_IDX = 4;

    constexpr std::size_t ORE_M_IDX = 0;
    constexpr std::size_t CLAY_M_IDX = 1;
    constexpr std::size_t OBSDN_M_IDX = 2;

    using blueprint = std::array<std::array<int, 3>, 4>;

    //Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 22 ore. Each obsidian robot costs 33 ore and 14 clay. Each geode robot costs 22 ore and 77 obsidian.
    //0         1  2    3   4     5     6 7    8    9    10    11    12 13   14   15       16    17    18 19  20  21 22    23   24    25    26    27 28  29  30 31
    blueprint parse_blueprint(std::string_view s) {
        const auto parts = split(s, ' ');
        blueprint retval{};
        retval[ORE_R_IDX][ORE_M_IDX] = parse<int>(parts[6]);
        retval[CLAY_R_IDX][ORE_M_IDX] = parse<int>(parts[12]);
        retval[OBSDN_R_IDX][ORE_M_IDX] = parse<int>(parts[18]);
        retval[OBSDN_R_IDX][CLAY_M_IDX] = parse<int>(parts[21]);
        retval[GEO_R_IDX][ORE_M_IDX] = parse<int>(parts[27]);
        retval[GEO_R_IDX][OBSDN_M_IDX] = parse<int>(parts[30]);
        return retval;
    }

    std::vector<blueprint> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(parse_blueprint) | to<std::vector<blueprint>>();
    }

    struct resource {
        int rate = 0;
        int total = 0;
    };

    struct status {
        std::array<resource, 4> resources{};
        int minute = 24;

        status() = default;
        explicit status(int start_minutes) : minute{start_minutes} { resources[ORE_R_IDX].rate = 1; }

        std::strong_ordering operator<=>(const status& rhs) const noexcept {
            const auto rval = std::lexicographical_compare_three_way(resources.begin(), resources.end(),
                                                           rhs.resources.begin(), rhs.resources.end(),
                                                           [](const resource& a, const resource& b){ return a.rate <=> b.rate; });
            if (rval == std::strong_ordering::equal) {
                const auto tval = std::lexicographical_compare_three_way(resources.begin(), resources.end(),
                                                                         rhs.resources.begin(), rhs.resources.end(),
                                                                         [](const resource& a, const resource& b){ return a.total <=> b.total; });
                if (tval == std::strong_ordering::equal) {
                    return minute <=> rhs.minute;
                }
                else {
                    return tval;
                }
            }
            else {
                return rval;
            }
        }
    };

    void mine(status& state) {
        for (auto& r : state.resources) {
            r.total += r.rate;
        }
    }

    int time_to_build(const blueprint& bp, const status& current, const int idx) {
        int longest = -1;
        for (int r = 0; r < bp[idx].size(); ++r) {
            if (bp[idx][r] != 0) {
                if (current.resources[r].rate == 0) {
                    return -1;
                }
                else if (current.resources[r].total >= bp[idx][r]) {
                    if (longest < 0) {
                        longest = 0;
                    }
                }
                else {
                    const int needed = bp[idx][r] - current.resources[r].total;
                    const auto div = std::div(needed, current.resources[r].rate);
                    const auto time = div.quot + (div.rem != 0 ? 1 : 0);
                    if (time > longest) {
                        longest = time;
                    }
                }
            }
        }
        return longest;
    }

    void build(const blueprint& bp, status& current, const int idx) {
        for (int r = 0; r < bp[idx].size(); ++r) {
            current.resources[r].total -= bp[idx][r];
        }
    }

    std::optional<status> check_build(const blueprint& bp, const status& current, const int idx) {
        auto next = current;
        const auto ttb = time_to_build(bp, next, idx);
        if (ttb < 0) {
            return std::nullopt;
        }
        else if (ttb > 0) {
            next.minute -= ttb;
            for (int i = 0; i < ttb; ++i) {
                mine(next);
            }
        }
        build(bp, next, idx);
        mine(next);
        --next.minute;
        if (next.minute < 0) {
            return std::nullopt;
        }
        ++next.resources[idx].rate;
        return next;
    }

    int check_recursive_impl(const blueprint& bp, const status& current, const std::array<int, 3>& max_r, const int other_best, const int num_minutes) {
        if (other_best > current.resources[GEO_R_IDX].total + current.resources[GEO_R_IDX].rate * current.minute + (current.minute * current.minute + current.minute) / 2) {
            return 0;
        }
        int best = 0;
        bool any_last = false;
        for (int i = GEO_R_IDX; i >= 0; --i) {
            if (i < GEO_R_IDX && current.resources[i].rate >= max_r[i]) {
                continue;
            }
            auto r = check_build(bp, current, i);
            if (r) {
                any_last = true;
                if (r->minute == 0) {
                    const int geo = r->resources[GEO_R_IDX].total;
                    if (geo > best) {
                        best = geo;
                    }
                }
                else {
                    const int res = check_recursive_impl(bp, *r, max_r, std::max(best, other_best), num_minutes);
                    if (res > best) {
                        best = res;
                    }
                }
            }
        }
        if (!any_last) {
            auto check = current;
            while (check.minute > 0) {
                mine(check);
                --check.minute;
            }
            best = check.resources[GEO_R_IDX].total;
        }
        return best;
    }

    int check_recursive(const blueprint& bp, const int num_minutes) {
        status start {num_minutes};
        std::array<int, 3> max_r{};
        max_r[ORE_M_IDX] = std::max_element(bp.begin(), bp.end(), [](const std::array<int, 3>& a, const std::array<int, 3>& b){ return a[ORE_R_IDX] < b[ORE_R_IDX]; })->at(0);
        max_r[CLAY_M_IDX] = bp[OBSDN_R_IDX][CLAY_M_IDX];
        max_r[OBSDN_M_IDX] = bp[GEO_R_IDX][OBSDN_M_IDX];
        const auto retval = check_recursive_impl(bp, start, max_r, 0, num_minutes);
        return retval;
    }

    /*
    --- Day 19: Not Enough Minerals ---
    Your scans show that the lava did indeed form obsidian!

    The wind has changed direction enough to stop sending lava droplets toward you, so you and the elephants exit the cave. As you do, you notice a collection of geodes around the pond. Perhaps you could use the obsidian to create some geode-cracking robots and break them open?

    To collect the obsidian from the bottom of the pond, you'll need waterproof obsidian-collecting robots. Fortunately, there is an abundant amount of clay nearby that you can use to make them waterproof.

    In order to harvest the clay, you'll need special-purpose clay-collecting robots. To make any type of robot, you'll need ore, which is also plentiful but in the opposite direction from the clay.

    Collecting ore requires ore-collecting robots with big drills. Fortunately, you have exactly one ore-collecting robot in your pack that you can use to kickstart the whole operation.

    Each robot can collect 1 of its resource type per minute. It also takes one minute for the robot factory (also conveniently from your pack) to construct any type of robot, although it consumes the necessary resources available when construction begins.

    The robot factory has many blueprints (your puzzle input) you can choose from, but once you've configured it with a blueprint, you can't change it. You'll need to work out which blueprint is best.

    For example:

    Blueprint 1:
      Each ore robot costs 4 ore.
      Each clay robot costs 2 ore.
      Each obsidian robot costs 3 ore and 14 clay.
      Each geode robot costs 2 ore and 7 obsidian.

    Blueprint 2:
      Each ore robot costs 2 ore.
      Each clay robot costs 3 ore.
      Each obsidian robot costs 3 ore and 8 clay.
      Each geode robot costs 3 ore and 12 obsidian.
    (Blueprints have been line-wrapped here for legibility. The robot factory's actual assortment of blueprints are provided one blueprint per line.)

    The elephants are starting to look hungry, so you shouldn't take too long; you need to figure out which blueprint would maximize the number of opened geodes after 24 minutes by figuring out which robots to build and when to build them.

    Using blueprint 1 in the example above, the largest number of geodes you could open in 24 minutes is 9. One way to achieve that is:

    == Minute 1 ==
    1 ore-collecting robot collects 1 ore; you now have 1 ore.

    == Minute 2 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.

    == Minute 3 ==
    Spend 2 ore to start building a clay-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 1 ore.
    The new clay-collecting robot is ready; you now have 1 of them.

    == Minute 4 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    1 clay-collecting robot collects 1 clay; you now have 1 clay.

    == Minute 5 ==
    Spend 2 ore to start building a clay-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 1 ore.
    1 clay-collecting robot collects 1 clay; you now have 2 clay.
    The new clay-collecting robot is ready; you now have 2 of them.

    == Minute 6 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    2 clay-collecting robots collect 2 clay; you now have 4 clay.

    == Minute 7 ==
    Spend 2 ore to start building a clay-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 1 ore.
    2 clay-collecting robots collect 2 clay; you now have 6 clay.
    The new clay-collecting robot is ready; you now have 3 of them.

    == Minute 8 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    3 clay-collecting robots collect 3 clay; you now have 9 clay.

    == Minute 9 ==
    1 ore-collecting robot collects 1 ore; you now have 3 ore.
    3 clay-collecting robots collect 3 clay; you now have 12 clay.

    == Minute 10 ==
    1 ore-collecting robot collects 1 ore; you now have 4 ore.
    3 clay-collecting robots collect 3 clay; you now have 15 clay.

    == Minute 11 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    3 clay-collecting robots collect 3 clay; you now have 4 clay.
    The new obsidian-collecting robot is ready; you now have 1 of them.

    == Minute 12 ==
    Spend 2 ore to start building a clay-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 1 ore.
    3 clay-collecting robots collect 3 clay; you now have 7 clay.
    1 obsidian-collecting robot collects 1 obsidian; you now have 1 obsidian.
    The new clay-collecting robot is ready; you now have 4 of them.

    == Minute 13 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    4 clay-collecting robots collect 4 clay; you now have 11 clay.
    1 obsidian-collecting robot collects 1 obsidian; you now have 2 obsidian.

    == Minute 14 ==
    1 ore-collecting robot collects 1 ore; you now have 3 ore.
    4 clay-collecting robots collect 4 clay; you now have 15 clay.
    1 obsidian-collecting robot collects 1 obsidian; you now have 3 obsidian.

    == Minute 15 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 1 ore.
    4 clay-collecting robots collect 4 clay; you now have 5 clay.
    1 obsidian-collecting robot collects 1 obsidian; you now have 4 obsidian.
    The new obsidian-collecting robot is ready; you now have 2 of them.

    == Minute 16 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    4 clay-collecting robots collect 4 clay; you now have 9 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 6 obsidian.

    == Minute 17 ==
    1 ore-collecting robot collects 1 ore; you now have 3 ore.
    4 clay-collecting robots collect 4 clay; you now have 13 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 8 obsidian.

    == Minute 18 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    1 ore-collecting robot collects 1 ore; you now have 2 ore.
    4 clay-collecting robots collect 4 clay; you now have 17 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 3 obsidian.
    The new geode-cracking robot is ready; you now have 1 of them.

    == Minute 19 ==
    1 ore-collecting robot collects 1 ore; you now have 3 ore.
    4 clay-collecting robots collect 4 clay; you now have 21 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 5 obsidian.
    1 geode-cracking robot cracks 1 geode; you now have 1 open geode.

    == Minute 20 ==
    1 ore-collecting robot collects 1 ore; you now have 4 ore.
    4 clay-collecting robots collect 4 clay; you now have 25 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 7 obsidian.
    1 geode-cracking robot cracks 1 geode; you now have 2 open geodes.

    == Minute 21 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    1 ore-collecting robot collects 1 ore; you now have 3 ore.
    4 clay-collecting robots collect 4 clay; you now have 29 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 2 obsidian.
    1 geode-cracking robot cracks 1 geode; you now have 3 open geodes.
    The new geode-cracking robot is ready; you now have 2 of them.

    == Minute 22 ==
    1 ore-collecting robot collects 1 ore; you now have 4 ore.
    4 clay-collecting robots collect 4 clay; you now have 33 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 4 obsidian.
    2 geode-cracking robots crack 2 geodes; you now have 5 open geodes.

    == Minute 23 ==
    1 ore-collecting robot collects 1 ore; you now have 5 ore.
    4 clay-collecting robots collect 4 clay; you now have 37 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 6 obsidian.
    2 geode-cracking robots crack 2 geodes; you now have 7 open geodes.

    == Minute 24 ==
    1 ore-collecting robot collects 1 ore; you now have 6 ore.
    4 clay-collecting robots collect 4 clay; you now have 41 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 8 obsidian.
    2 geode-cracking robots crack 2 geodes; you now have 9 open geodes.
    However, by using blueprint 2 in the example above, you could do even better: the largest number of geodes you could open in 24 minutes is 12.

    Determine the quality level of each blueprint by multiplying that blueprint's ID number with the largest number of geodes that can be opened in 24 minutes using that blueprint. In this example, the first blueprint has ID 1 and can open 9 geodes, so its quality level is 9. The second blueprint has ID 2 and can open 12 geodes, so its quality level is 24. Finally, if you add up the quality levels of all of the blueprints in the list, you get 33.

    Determine the quality level of each blueprint using the largest number of geodes it could produce in 24 minutes. What do you get if you add up the quality level of all of the blueprints in your list?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        int result = 0;
        for (int id = 1; id <= input.size(); ++id) {
            const auto res = check_recursive(input[id - 1], 24);
            result += res * id;
        }
        return std::to_string(result);
    }

    /*
    --- Part Two ---
    While you were choosing the best blueprint, the elephants found some food on their own, so you're not in as much of a hurry; you figure you probably have 32 minutes before the wind changes direction again and you'll need to get out of range of the erupting volcano.

    Unfortunately, one of the elephants ate most of your blueprint list! Now, only the first three blueprints in your list are intact.

    In 32 minutes, the largest number of geodes blueprint 1 (from the example above) can open is 56. One way to achieve that is:

    == Minute 1 ==
    1 ore-collecting robot collects 1 ore; you now have 1 ore.

    == Minute 2 ==
    1 ore-collecting robot collects 1 ore; you now have 2 ore.

    == Minute 3 ==
    1 ore-collecting robot collects 1 ore; you now have 3 ore.

    == Minute 4 ==
    1 ore-collecting robot collects 1 ore; you now have 4 ore.

    == Minute 5 ==
    Spend 4 ore to start building an ore-collecting robot.
    1 ore-collecting robot collects 1 ore; you now have 1 ore.
    The new ore-collecting robot is ready; you now have 2 of them.

    == Minute 6 ==
    2 ore-collecting robots collect 2 ore; you now have 3 ore.

    == Minute 7 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    The new clay-collecting robot is ready; you now have 1 of them.

    == Minute 8 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    1 clay-collecting robot collects 1 clay; you now have 1 clay.
    The new clay-collecting robot is ready; you now have 2 of them.

    == Minute 9 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    2 clay-collecting robots collect 2 clay; you now have 3 clay.
    The new clay-collecting robot is ready; you now have 3 of them.

    == Minute 10 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    3 clay-collecting robots collect 3 clay; you now have 6 clay.
    The new clay-collecting robot is ready; you now have 4 of them.

    == Minute 11 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    4 clay-collecting robots collect 4 clay; you now have 10 clay.
    The new clay-collecting robot is ready; you now have 5 of them.

    == Minute 12 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    5 clay-collecting robots collect 5 clay; you now have 15 clay.
    The new clay-collecting robot is ready; you now have 6 of them.

    == Minute 13 ==
    Spend 2 ore to start building a clay-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    6 clay-collecting robots collect 6 clay; you now have 21 clay.
    The new clay-collecting robot is ready; you now have 7 of them.

    == Minute 14 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 2 ore.
    7 clay-collecting robots collect 7 clay; you now have 14 clay.
    The new obsidian-collecting robot is ready; you now have 1 of them.

    == Minute 15 ==
    2 ore-collecting robots collect 2 ore; you now have 4 ore.
    7 clay-collecting robots collect 7 clay; you now have 21 clay.
    1 obsidian-collecting robot collects 1 obsidian; you now have 1 obsidian.

    == Minute 16 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    7 clay-collecting robots collect 7 clay; you now have 14 clay.
    1 obsidian-collecting robot collects 1 obsidian; you now have 2 obsidian.
    The new obsidian-collecting robot is ready; you now have 2 of them.

    == Minute 17 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 2 ore.
    7 clay-collecting robots collect 7 clay; you now have 7 clay.
    2 obsidian-collecting robots collect 2 obsidian; you now have 4 obsidian.
    The new obsidian-collecting robot is ready; you now have 3 of them.

    == Minute 18 ==
    2 ore-collecting robots collect 2 ore; you now have 4 ore.
    7 clay-collecting robots collect 7 clay; you now have 14 clay.
    3 obsidian-collecting robots collect 3 obsidian; you now have 7 obsidian.

    == Minute 19 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    7 clay-collecting robots collect 7 clay; you now have 7 clay.
    3 obsidian-collecting robots collect 3 obsidian; you now have 10 obsidian.
    The new obsidian-collecting robot is ready; you now have 4 of them.

    == Minute 20 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 3 ore.
    7 clay-collecting robots collect 7 clay; you now have 14 clay.
    4 obsidian-collecting robots collect 4 obsidian; you now have 7 obsidian.
    The new geode-cracking robot is ready; you now have 1 of them.

    == Minute 21 ==
    Spend 3 ore and 14 clay to start building an obsidian-collecting robot.
    2 ore-collecting robots collect 2 ore; you now have 2 ore.
    7 clay-collecting robots collect 7 clay; you now have 7 clay.
    4 obsidian-collecting robots collect 4 obsidian; you now have 11 obsidian.
    1 geode-cracking robot cracks 1 geode; you now have 1 open geode.
    The new obsidian-collecting robot is ready; you now have 5 of them.

    == Minute 22 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 2 ore.
    7 clay-collecting robots collect 7 clay; you now have 14 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 9 obsidian.
    1 geode-cracking robot cracks 1 geode; you now have 2 open geodes.
    The new geode-cracking robot is ready; you now have 2 of them.

    == Minute 23 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 2 ore.
    7 clay-collecting robots collect 7 clay; you now have 21 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 7 obsidian.
    2 geode-cracking robots crack 2 geodes; you now have 4 open geodes.
    The new geode-cracking robot is ready; you now have 3 of them.

    == Minute 24 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 2 ore.
    7 clay-collecting robots collect 7 clay; you now have 28 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 5 obsidian.
    3 geode-cracking robots crack 3 geodes; you now have 7 open geodes.
    The new geode-cracking robot is ready; you now have 4 of them.

    == Minute 25 ==
    2 ore-collecting robots collect 2 ore; you now have 4 ore.
    7 clay-collecting robots collect 7 clay; you now have 35 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 10 obsidian.
    4 geode-cracking robots crack 4 geodes; you now have 11 open geodes.

    == Minute 26 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 4 ore.
    7 clay-collecting robots collect 7 clay; you now have 42 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 8 obsidian.
    4 geode-cracking robots crack 4 geodes; you now have 15 open geodes.
    The new geode-cracking robot is ready; you now have 5 of them.

    == Minute 27 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 4 ore.
    7 clay-collecting robots collect 7 clay; you now have 49 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 6 obsidian.
    5 geode-cracking robots crack 5 geodes; you now have 20 open geodes.
    The new geode-cracking robot is ready; you now have 6 of them.

    == Minute 28 ==
    2 ore-collecting robots collect 2 ore; you now have 6 ore.
    7 clay-collecting robots collect 7 clay; you now have 56 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 11 obsidian.
    6 geode-cracking robots crack 6 geodes; you now have 26 open geodes.

    == Minute 29 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 6 ore.
    7 clay-collecting robots collect 7 clay; you now have 63 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 9 obsidian.
    6 geode-cracking robots crack 6 geodes; you now have 32 open geodes.
    The new geode-cracking robot is ready; you now have 7 of them.

    == Minute 30 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 6 ore.
    7 clay-collecting robots collect 7 clay; you now have 70 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 7 obsidian.
    7 geode-cracking robots crack 7 geodes; you now have 39 open geodes.
    The new geode-cracking robot is ready; you now have 8 of them.

    == Minute 31 ==
    Spend 2 ore and 7 obsidian to start building a geode-cracking robot.
    2 ore-collecting robots collect 2 ore; you now have 6 ore.
    7 clay-collecting robots collect 7 clay; you now have 77 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 5 obsidian.
    8 geode-cracking robots crack 8 geodes; you now have 47 open geodes.
    The new geode-cracking robot is ready; you now have 9 of them.

    == Minute 32 ==
    2 ore-collecting robots collect 2 ore; you now have 8 ore.
    7 clay-collecting robots collect 7 clay; you now have 84 clay.
    5 obsidian-collecting robots collect 5 obsidian; you now have 10 obsidian.
    9 geode-cracking robots crack 9 geodes; you now have 56 open geodes.
    However, blueprint 2 from the example above is still better; using it, the largest number of geodes you could open in 32 minutes is 62.

    You no longer have enough blueprints to worry about quality levels. Instead, for each of the first three blueprints, determine the largest number of geodes you could open; then, multiply these three values together.

    Don't worry about quality levels; instead, just determine the largest number of geodes you could open using each of the first three blueprints. What do you get if you multiply these numbers together?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        int result = 1;
        for (int id = 1; id <= 3; ++id) {
            const auto res = check_recursive(input[id - 1], 32);
            result *= res;
        }
        return std::to_string(result);
        //9360 too low
    }

    aoc::registration r{2022, 19, part_1, part_2};

    TEST_SUITE("2022_day19") {
        TEST_CASE("2022_day19:example") {
            std::vector<std::string> lines {
                "Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.",
                "Blueprint 2: Each ore robot costs 2 ore. Each clay robot costs 3 ore. Each obsidian robot costs 3 ore and 8 clay. Each geode robot costs 3 ore and 12 obsidian."
            };
            auto input = lines | std::views::transform(parse_blueprint) | to<std::vector<blueprint>>();
            int result = 0;
            for (int id = 1; id <= input.size(); ++id) {
                const auto res = check_recursive(input[id - 1], 24);
                result += res * id;
            }
            CHECK_EQ(result, 33);

            input = lines | std::views::transform(parse_blueprint) | to<std::vector<blueprint>>();
            const auto res1 = check_recursive(input[0], 32);
            CHECK_EQ(res1, 56);
            const auto res2 = check_recursive(input[1], 32);
            CHECK_EQ(res2, 62);
        }

        TEST_CASE("2022_day19:build_time") {
            std::vector<std::string> lines {
                    "Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.",
                    "Blueprint 2: Each ore robot costs 2 ore. Each clay robot costs 3 ore. Each obsidian robot costs 3 ore and 8 clay. Each geode robot costs 3 ore and 12 obsidian."
            };
            const auto bp = lines | std::views::transform(parse_blueprint) | to<std::vector<blueprint>>();
            status current{24};
            const auto r1 = time_to_build(bp[0], current, ORE_R_IDX);
            CHECK_EQ(r1, 4);

            const auto r2 = time_to_build(bp[1], current, ORE_R_IDX);
            CHECK_EQ(r2, 2);

            const auto r3 = time_to_build(bp[0], current, CLAY_R_IDX);
            CHECK_EQ(r3, 2);

            const auto r4 = time_to_build(bp[1], current, CLAY_R_IDX);
            CHECK_EQ(r4, 3);

            const auto r5 = time_to_build(bp[0], current, OBSDN_R_IDX);
            CHECK_EQ(r5, -1);

            current.resources[CLAY_M_IDX].rate = 1;
            const auto r6 = time_to_build(bp[0], current, OBSDN_R_IDX);
            CHECK_EQ(r6, 14);

            current.resources[CLAY_M_IDX].rate = 7;
            const auto r7 = time_to_build(bp[0], current, OBSDN_R_IDX);
            CHECK_EQ(r7, 3);
        }
    }

}