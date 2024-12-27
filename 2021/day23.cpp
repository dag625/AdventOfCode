//
// Created by Dan on 12/22/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int HALLWAY_LEN = 11;
    constexpr char EMPTY_SPOT = '\0';

    using amphipod = char;

    struct spot {
        bool is_room = false;
        std::array<amphipod, 4> occupants{};

        bool operator==(const spot& rhs) const noexcept {
            return std::equal(occupants.begin(), occupants.end(), rhs.occupants.begin(), rhs.occupants.end());
        }
        bool operator<(const spot& rhs) const noexcept {
            const auto diff = std::mismatch(occupants.begin(), occupants.end(), rhs.occupants.begin(), rhs.occupants.end());
            if (diff.first == occupants.end()) {
                return false;
            }
            else {
                return *diff.first < *diff.second;
            }
        }
    };

    using state = std::array<spot, HALLWAY_LEN>;

    std::string to_string(const state& s) {
        std::string retval;
        retval.reserve(23);
        bool first = true;
        for (const auto& spot : s) {
            if (!first) {
                retval += '|';
            }
            first = false;
            retval += spot.occupants[0] ? spot.occupants[0] : 'x';
            if (spot.is_room) {
                retval += spot.occupants[1] ? spot.occupants[1] : 'x';
                retval += spot.occupants[2] ? spot.occupants[2] : 'x';
                retval += spot.occupants[3] ? spot.occupants[3] : 'x';
            }
        }
        return retval;
    }

    state state_from_string(std::string_view s) {
        const auto spots = split(s, '|');
        state retval{};
        if (spots.size() != retval.size()) {
            return retval;
        }
        for (int i = 0; i < spots.size(); ++i) {
            if (spots[i].size() == 1) {
                retval[i].is_room = false;
                retval[i].occupants[0] = spots[i][0] == 'x' ? EMPTY_SPOT : spots[i][0];
            }
            else if (spots[i].size() == 2) {
                retval[i].is_room = true;
                retval[i].occupants[0] = spots[i][0] == 'x' ? EMPTY_SPOT : spots[i][0];
                retval[i].occupants[3] = spots[i][1] == 'x' ? EMPTY_SPOT : spots[i][1];
            }
            else if (spots[i].size() == 4) {
                retval[i].is_room = true;
                retval[i].occupants[0] = spots[i][0] == 'x' ? EMPTY_SPOT : spots[i][0];
                retval[i].occupants[1] = spots[i][1] == 'x' ? EMPTY_SPOT : spots[i][1];
                retval[i].occupants[2] = spots[i][2] == 'x' ? EMPTY_SPOT : spots[i][2];
                retval[i].occupants[3] = spots[i][3] == 'x' ? EMPTY_SPOT : spots[i][3];
            }
        }
        return retval;
    }

    bool operator==(const state& a, const state& b) {
        return to_string(a) == to_string(b);
    }

    bool is_less(const state& a, const state& b) {
        return to_string(a) < to_string(b);
    }

    /*
    This is the raw input from AoC:

    #############
    #...........#
    ###A#C#B#B###
      #D#D#A#C#
      #########
     */
    constexpr state INPUT = {
            spot{},
            spot{},
            spot{true, {amphipod{'A'}, amphipod{'D'}, amphipod{'D'}, amphipod{'D'}}},
            spot{},
            spot{true, {amphipod{'C'}, amphipod{'C'}, amphipod{'B'}, amphipod{'D'}}},
            spot{},
            spot{true, {amphipod{'B'}, amphipod{'B'}, amphipod{'A'}, amphipod{'A'}}},
            spot{},
            spot{true, {amphipod{'B'}, amphipod{'A'}, amphipod{'C'}, amphipod{'C'}}},
            spot{},
            spot{}
    };

    constexpr int energy_per_move(char c) {
        switch (c) {
            case 'A': return 1;
            case 'B': return 10;
            case 'C': return 100;
            case 'D': return 1000;
            default:  return -1;
        }
    }

    constexpr int expected_destination(char c) {
        switch (c) {
            case 'A': return 2;
            case 'B': return 4;
            case 'C': return 6;
            case 'D': return 8;
            default:  return -1;
        }
    }

    bool clear_route(const state& s, const int start, const int end) {
        if (start < 0 || start >= s.size() || end < 0 || end >= s.size() || start == end) {
            return false;
        }
        if (start < end) {
            for (int idx = start + 1; idx < end; ++idx) {
                if (!s[idx].is_room && s[idx].occupants[0] != EMPTY_SPOT) {
                    return false;
                }
            }
        }
        else {
            for (int idx = end + 1; idx < start; ++idx) {
                if (!s[idx].is_room && s[idx].occupants[0] != EMPTY_SPOT) {
                    return false;
                }
            }
        }
        return true;
    }

    struct move {
        state new_state;
        int energy_cost = 0;

        constexpr move() = default;
        constexpr move(const state& ns) : new_state{ns}, energy_cost{0} {}
        constexpr move(const state& ns, int cost) : new_state{ns}, energy_cost{cost} {}

        bool operator<(const move& rhs) const noexcept {
            return energy_cost < rhs.energy_cost;
        }
    };

    bool is_happy(const state& s, const bool use_mid) {
        int idx = 0;
        for (const auto& spot : s) {
            if (spot.is_room) {
                bool p1_contrib = spot.occupants[0] == EMPTY_SPOT ||
                                  spot.occupants[3] == EMPTY_SPOT ||
                                  spot.occupants[0] != spot.occupants[3] ||
                        expected_destination(spot.occupants[0]) != idx;
                bool p2_contrib = use_mid && (spot.occupants[1] == EMPTY_SPOT ||
                                               spot.occupants[2] == EMPTY_SPOT ||
                                               spot.occupants[0] != spot.occupants[1] ||
                                               spot.occupants[0] != spot.occupants[2]);
                if (p1_contrib || p2_contrib)
                {
                    return false;
                }
            }
            else {
                if (spot.occupants[0] != EMPTY_SPOT) {
                    return false;
                }
            }
            ++idx;
        }
        return true;
    }

    std::pair<int, bool> find_lowest_dest_available(const state& st, const int idx, const char type, const bool use_mid) {
        const auto& s = st[idx].occupants;
        //Assume in room and that 0 is open.
        if (use_mid) {
            int lowest = 0;
            bool found = false, all_type = true;
            for (int i = 1; i < s.size(); ++i) {
                if (!found && s[i] == EMPTY_SPOT) {
                    lowest = i;
                }
                else {
                    found = true;
                    all_type = all_type && s[i] == type;
                }
            }
            return {lowest, all_type};
        }
        else {
            return {s[3] == EMPTY_SPOT ? 3 : 0, s[3] == type};
        }
    }

    std::vector<move> iterate(const state& s, const bool use_mid) {
        std::vector<move> retval;
        retval.reserve(50);
        int idx = 0;
        for (const auto& stop : s) {
            if (stop.is_room) {
                int to_hw_cost = 0;
                char type = EMPTY_SPOT;
                bool lower_same = false;
                int at = 0;
                if (stop.occupants[0] != EMPTY_SPOT) {
                    type = stop.occupants[0];
                    to_hw_cost = 1;
                    lower_same = type == stop.occupants[3] && (!use_mid || (type == stop.occupants[1] && type == stop.occupants[2]));
                }
                else if (use_mid && stop.occupants[1] != EMPTY_SPOT) {
                    at = 1;
                    type = stop.occupants[1];
                    to_hw_cost = 2;
                    lower_same = type == stop.occupants[3] && type == stop.occupants[2];
                }
                else if (use_mid && stop.occupants[2] != EMPTY_SPOT) {
                    at = 2;
                    type = stop.occupants[2];
                    to_hw_cost = 3;
                    lower_same = type == stop.occupants[3];
                }
                else if (stop.occupants[3] != EMPTY_SPOT && expected_destination(stop.occupants[3]) != idx) {
                    at = 3;
                    type = stop.occupants[3];
                    to_hw_cost = use_mid ? 4 : 2;
                    lower_same = true;
                }
                if (type != EMPTY_SPOT) {
                    const int dest = expected_destination(type);
                    if (dest == idx && lower_same) {
                        ++idx;
                        continue;
                    }
                    const auto [lowest_dest_available, lower_types_match] = find_lowest_dest_available(s, dest, type, use_mid);
                    const auto lowest_dest_energy = use_mid ? lowest_dest_available : (lowest_dest_available == 3 ? 1 : 0);
                    if (clear_route(s, idx, dest) && s[dest].occupants[0] == EMPTY_SPOT && (lowest_dest_available == 3 || lower_types_match)) {
                        retval.emplace_back(s, (std::abs(dest - idx) + (lowest_dest_energy + 1) + to_hw_cost) *
                                               energy_per_move(type));
                        retval.back().new_state[idx].occupants[at] = EMPTY_SPOT;
                        retval.back().new_state[dest].occupants[lowest_dest_available] = stop.occupants[at];
                    }
                    else {
                        for (int hw = 0; hw < s.size(); ++hw) {
                            if (!s[hw].is_room && s[hw].occupants[0] == EMPTY_SPOT && clear_route(s, idx, hw)) {
                                retval.emplace_back(s, (std::abs(hw - idx) + to_hw_cost) * energy_per_move(type));
                                retval.back().new_state[idx].occupants[at] = EMPTY_SPOT;
                                retval.back().new_state[hw].occupants[0] = stop.occupants[at];
                            }
                        }
                    }
                }
            }
            else if (stop.occupants[0] != EMPTY_SPOT) {
                const int dest = expected_destination(stop.occupants[0]);
                const auto [lowest_dest_available, lower_types_match] = find_lowest_dest_available(s, dest, stop.occupants[0], use_mid);
                const auto lowest_dest_energy = use_mid ? lowest_dest_available : (lowest_dest_available == 3 ? 1 : 0);
                if (clear_route(s, idx, dest) && s[dest].occupants[0] == EMPTY_SPOT && (lowest_dest_available == 3 || lower_types_match)) {
                    retval.emplace_back(s, (std::abs(dest - idx) + (lowest_dest_energy + 1)) * energy_per_move(stop.occupants[0]));
                    retval.back().new_state[idx].occupants[0] = EMPTY_SPOT;
                    retval.back().new_state[dest].occupants[lowest_dest_available] = stop.occupants[0];
                }
            }
            ++idx;
        }
        return retval;
    }

    struct cache_line {
        state s;
        int cost = std::numeric_limits<int>::max();

        bool operator==(const cache_line& rhs) const noexcept {
            return std::equal(s.begin(), s.end(), rhs.s.begin(), rhs.s.end());
        }
        bool operator<(const cache_line& rhs) const noexcept {
            const auto res = std::mismatch(s.begin(), s.end(), rhs.s.begin(), rhs.s.end());
            if (res.first == s.end()) {
                return false;
            }
            return *res.first < *res.second;
        }
    };

    int run(const state& s, const bool use_mid, std::vector<cache_line>& seen, int current_energy = 0) {
        const auto str = to_string(s);
        const auto found1 = std::lower_bound(seen.cbegin(), seen.cend(), s, [](const cache_line& c, const state& s){ return c.s < s; });
        if (found1 != seen.cend() && s == found1->s) {
            return found1->cost;
        }
        else if (is_happy(s, use_mid)) {
            return current_energy;
        }
        const auto res = iterate(s, use_mid);
        if (res.empty()) {
            const auto s2 = to_string(s);
        }
        int min = std::numeric_limits<int>::max();
        for (const auto& r : res) {
            auto rmin = run(r.new_state, use_mid, seen, r.energy_cost + current_energy);
            const auto found2 = std::lower_bound(seen.begin(), seen.end(), r.new_state, [](const cache_line& c, const state& s){ return c.s < s; });
//            if (found2 == seen.end() || found2->s != r.new_state) {
//                seen.insert(found2, {r.new_state, rmin});
//            }
            if (rmin < min) {
                min = rmin;
            }
        }
        return min;
    }

    /*
    --- Day 23: Amphipod ---
    A group of amphipods notice your fancy submarine and flag you down. "With such an impressive shell," one amphipod says, "surely you can help us with a question that has stumped our best scientists."

    They go on to explain that a group of timid, stubborn amphipods live in a nearby burrow. Four types of amphipods live there: Amber (A), Bronze (B), Copper (C), and Desert (D). They live in a burrow that consists of a hallway and four side rooms. The side rooms are initially full of amphipods, and the hallway is initially empty.

    They give you a diagram of the situation (your puzzle input), including locations of each amphipod (A, B, C, or D, each of which is occupying an otherwise open space), walls (#), and open space (.).

    For example:

    #############
    #...........#
    ###B#C#B#D###
      #A#D#C#A#
      #########
    The amphipods would like a method to organize every amphipod into side rooms so that each side room contains one type of amphipod and the types are sorted A-D going left to right, like this:

    #############
    #...........#
    ###A#B#C#D###
      #A#B#C#D#
      #########
    Amphipods can move up, down, left, or right so long as they are moving into an unoccupied open space. Each type of amphipod requires a different amount of energy to move one step: Amber amphipods require 1 energy per step, Bronze amphipods require 10 energy, Copper amphipods require 100, and Desert ones require 1000. The amphipods would like you to find a way to organize the amphipods that requires the least total energy.

    However, because they are timid and stubborn, the amphipods have some extra rules:

    Amphipods will never stop on the space immediately outside any room. They can move into that space so long as they immediately continue moving. (Specifically, this refers to the four open spaces in the hallway that are directly above an amphipod starting position.)
    Amphipods will never move from the hallway into a room unless that room is their destination room and that room contains no amphipods which do not also have that room as their own destination. If an amphipod's starting room is not its destination room, it can stay in that room until it leaves the room. (For example, an Amber amphipod will not move from the hallway into the right three rooms, and will only move into the leftmost room if that room is empty or if it only contains other Amber amphipods.)
    Once an amphipod stops moving in the hallway, it will stay in that spot until it can move into a room. (That is, once any amphipod starts moving, any other amphipods currently in the hallway are locked in place and will not move again until they can move fully into a room.)
    In the above example, the amphipods can be organized using a minimum of 12521 energy. One way to do this is shown below.

    Starting configuration:

    #############
    #...........#
    ###B#C#B#D###
      #A#D#C#A#
      #########
    One Bronze amphipod moves into the hallway, taking 4 steps and using 40 energy:

    #############
    #...B.......#
    ###B#C#.#D###
      #A#D#C#A#
      #########
    The only Copper amphipod not in its side room moves there, taking 4 steps and using 400 energy:

    #############
    #...B.......#
    ###B#.#C#D###
      #A#D#C#A#
      #########
    A Desert amphipod moves out of the way, taking 3 steps and using 3000 energy, and then the Bronze amphipod takes its place, taking 3 steps and using 30 energy:

    #############
    #.....D.....#
    ###B#.#C#D###
      #A#B#C#A#
      #########
    The leftmost Bronze amphipod moves to its room using 40 energy:

    #############
    #.....D.....#
    ###.#B#C#D###
      #A#B#C#A#
      #########
    Both amphipods in the rightmost room move into the hallway, using 2003 energy in total:

    #############
    #.....D.D.A.#
    ###.#B#C#.###
      #A#B#C#.#
      #########
    Both Desert amphipods move into the rightmost room using 7000 energy:

    #############
    #.........A.#
    ###.#B#C#D###
      #A#B#C#D#
      #########
    Finally, the last Amber amphipod moves into its room, using 8 energy:

    #############
    #...........#
    ###A#B#C#D###
      #A#B#C#D#
      #########
    What is the least energy required to organize the amphipods?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        std::vector<cache_line> seen;
        return std::to_string(run(INPUT, false, seen));
    }

    /*
    --- Part Two ---
    As you prepare to give the amphipods your solution, you notice that the diagram they handed you was actually folded up. As you unfold it, you discover an extra part of the diagram.

    Between the first and second lines of text that contain amphipod starting positions, insert the following lines:

      #D#C#B#A#
      #D#B#A#C#
    So, the above example now becomes:

    #############
    #...........#
    ###B#C#B#D###
      #D#C#B#A#
      #D#B#A#C#
      #A#D#C#A#
      #########
    The amphipods still want to be organized into rooms similar to before:

    #############
    #...........#
    ###A#B#C#D###
      #A#B#C#D#
      #A#B#C#D#
      #A#B#C#D#
      #########
    In this updated example, the least energy required to organize these amphipods is 44169:

    #############
    #...........#
    ###B#C#B#D###
      #D#C#B#A#
      #D#B#A#C#
      #A#D#C#A#
      #########

    #############
    #..........D#
    ###B#C#B#.###
      #D#C#B#A#
      #D#B#A#C#
      #A#D#C#A#
      #########

    #############
    #A.........D#
    ###B#C#B#.###
      #D#C#B#.#
      #D#B#A#C#
      #A#D#C#A#
      #########

    #############
    #A........BD#
    ###B#C#.#.###
      #D#C#B#.#
      #D#B#A#C#
      #A#D#C#A#
      #########

    #############
    #A......B.BD#
    ###B#C#.#.###
      #D#C#.#.#
      #D#B#A#C#
      #A#D#C#A#
      #########

    #############
    #AA.....B.BD#
    ###B#C#.#.###
      #D#C#.#.#
      #D#B#.#C#
      #A#D#C#A#
      #########

    #############
    #AA.....B.BD#
    ###B#.#.#.###
      #D#C#.#.#
      #D#B#C#C#
      #A#D#C#A#
      #########

    #############
    #AA.....B.BD#
    ###B#.#.#.###
      #D#.#C#.#
      #D#B#C#C#
      #A#D#C#A#
      #########

    #############
    #AA...B.B.BD#
    ###B#.#.#.###
      #D#.#C#.#
      #D#.#C#C#
      #A#D#C#A#
      #########

    #############
    #AA.D.B.B.BD#
    ###B#.#.#.###
      #D#.#C#.#
      #D#.#C#C#
      #A#.#C#A#
      #########

    #############
    #AA.D...B.BD#
    ###B#.#.#.###
      #D#.#C#.#
      #D#.#C#C#
      #A#B#C#A#
      #########

    #############
    #AA.D.....BD#
    ###B#.#.#.###
      #D#.#C#.#
      #D#B#C#C#
      #A#B#C#A#
      #########

    #############
    #AA.D......D#
    ###B#.#.#.###
      #D#B#C#.#
      #D#B#C#C#
      #A#B#C#A#
      #########

    #############
    #AA.D......D#
    ###B#.#C#.###
      #D#B#C#.#
      #D#B#C#.#
      #A#B#C#A#
      #########

    #############
    #AA.D.....AD#
    ###B#.#C#.###
      #D#B#C#.#
      #D#B#C#.#
      #A#B#C#.#
      #########

    #############
    #AA.......AD#
    ###B#.#C#.###
      #D#B#C#.#
      #D#B#C#.#
      #A#B#C#D#
      #########

    #############
    #AA.......AD#
    ###.#B#C#.###
      #D#B#C#.#
      #D#B#C#.#
      #A#B#C#D#
      #########

    #############
    #AA.......AD#
    ###.#B#C#.###
      #.#B#C#.#
      #D#B#C#D#
      #A#B#C#D#
      #########

    #############
    #AA.D.....AD#
    ###.#B#C#.###
      #.#B#C#.#
      #.#B#C#D#
      #A#B#C#D#
      #########

    #############
    #A..D.....AD#
    ###.#B#C#.###
      #.#B#C#.#
      #A#B#C#D#
      #A#B#C#D#
      #########

    #############
    #...D.....AD#
    ###.#B#C#.###
      #A#B#C#.#
      #A#B#C#D#
      #A#B#C#D#
      #########

    #############
    #.........AD#
    ###.#B#C#.###
      #A#B#C#D#
      #A#B#C#D#
      #A#B#C#D#
      #########

    #############
    #..........D#
    ###A#B#C#.###
      #A#B#C#D#
      #A#B#C#D#
      #A#B#C#D#
      #########

    #############
    #...........#
    ###A#B#C#D###
      #A#B#C#D#
      #A#B#C#D#
      #A#B#C#D#
      #########
    Using the initial configuration from the full diagram, what is the least energy required to organize the amphipods?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        std::vector<cache_line> seen;
        return std::to_string(run(INPUT, true, seen));
    }

    aoc::registration r {2021, 23, part_1, part_2};

    TEST_SUITE("2021_day23") {
        TEST_CASE("2021_day23:example1") {
            std::vector<cache_line> seen;
            constexpr state EXAMPLE = {
                    spot{},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'D'}, amphipod{'D'}, amphipod{'A'}},
                    spot{},
                    spot{true, amphipod{'C'}, amphipod{'C'}, amphipod{'B'}, amphipod{'D'}},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'B'}, amphipod{'A'}, amphipod{'C'}},
                    spot{},
                    spot{true, amphipod{'D'}, amphipod{'A'}, amphipod{'C'}, amphipod{'A'}},
                    spot{},
                    spot{}
            };
            REQUIRE_EQ(run(EXAMPLE, false, seen), 12521);
        }
        TEST_CASE("2021_day23:example2") {
            std::vector<cache_line> seen;
            constexpr state EXAMPLE = {
                    spot{},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'D'}, amphipod{'D'}, amphipod{'A'}},
                    spot{},
                    spot{true, amphipod{'C'}, amphipod{'C'}, amphipod{'B'}, amphipod{'D'}},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'B'}, amphipod{'A'}, amphipod{'C'}},
                    spot{},
                    spot{true, amphipod{'D'}, amphipod{'A'}, amphipod{'C'}, amphipod{'A'}},
                    spot{},
                    spot{}
            };
            REQUIRE_EQ(run(EXAMPLE, true, seen), 44169);
        }
        TEST_CASE("2021_day23:example2_detail") {
            std::vector<cache_line> seen;
            constexpr state EXAMPLE = {
                    spot{},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'D'}, amphipod{'D'}, amphipod{'A'}},
                    spot{},
                    spot{true, amphipod{'C'}, amphipod{'C'}, amphipod{'B'}, amphipod{'D'}},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'B'}, amphipod{'A'}, amphipod{'C'}},
                    spot{},
                    spot{true, amphipod{'D'}, amphipod{'A'}, amphipod{'C'}, amphipod{'A'}},
                    spot{},
                    spot{}
            };
            const std::vector<std::string> steps = {
                    "x|x|BDDA|x|CCBD|x|BBAC|x|DACA|x|x",
                    "x|x|BDDA|x|CCBD|x|BBAC|x|xACA|x|D",
                    "A|x|BDDA|x|CCBD|x|BBAC|x|xxCA|x|D",
                    "A|x|BDDA|x|CCBD|x|xBAC|x|xxCA|B|D",
                    "A|x|BDDA|x|CCBD|x|xxAC|B|xxCA|B|D",
                    "A|A|BDDA|x|CCBD|x|xxxC|B|xxCA|B|D",
                    "A|A|BDDA|x|xCBD|x|xxCC|B|xxCA|B|D",
                    "A|A|BDDA|x|xxBD|x|xCCC|B|xxCA|B|D",
                    "A|A|BDDA|x|xxxD|B|xCCC|B|xxCA|B|D",
                    "A|A|BDDA|D|xxxx|B|xCCC|B|xxCA|B|D",
                    "A|A|BDDA|D|xxxB|x|xCCC|B|xxCA|B|D",
                    "A|A|BDDA|D|xxBB|x|xCCC|x|xxCA|B|D",
                    "A|A|BDDA|D|xBBB|x|xCCC|x|xxCA|x|D",
                    "A|A|BDDA|D|xBBB|x|CCCC|x|xxxA|x|D",
                    "A|A|BDDA|D|xBBB|x|CCCC|x|xxxx|A|D",
                    "A|A|BDDA|x|xBBB|x|CCCC|x|xxxD|A|D",
                    "A|A|xDDA|x|BBBB|x|CCCC|x|xxxD|A|D"
            };
            for (int i = 0; i < steps.size() - 1; ++i) {
                const auto res = iterate(state_from_string(steps[i]), true);
                REQUIRE(std::any_of(res.begin(), res.end(), [&steps, i](const move& m){ return to_string(m.new_state) == steps[i + 1]; }));
            }
        }
    }

}