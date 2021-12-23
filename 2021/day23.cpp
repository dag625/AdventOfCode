//
// Created by Dan on 12/22/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <variant>

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int HALLWAY_LEN = 11;
    constexpr char EMPTY_SPOT = '\0';

    using amphipod = char;

    struct spot {
        bool is_room = false;
        //std::array<amphipod, 2> occupants{};
        amphipod upper;
        amphipod lower;

        amphipod& at(int idx) { return idx == 0 ? upper : lower; }
        const amphipod& at(int idx) const { return idx == 0 ? upper : lower; }

        bool operator==(const spot& rhs) const noexcept {
            return at(0) == rhs.at(0) && at(1) == rhs.at(1);
        }
        bool operator<(const spot& rhs) const noexcept {
            return std::tie(at(0), at(1)) < std::tie(rhs.at(0), rhs.at(1));
        }
    };

    using state = std::array<spot, HALLWAY_LEN>;

    std::string to_string(const state& s) {
        std::string retval;
        retval.reserve(15);
        for (const auto& spot : s) {
            retval += spot.at(0);
            if (spot.is_room) {
                retval += spot.at(1);
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
            spot{true, amphipod{'A'}, amphipod{'D'}},
            spot{},
            spot{true, amphipod{'C'}, amphipod{'D'}},
            spot{},
            spot{true, amphipod{'B'}, amphipod{'A'}},
            spot{},
            spot{true, amphipod{'B'}, amphipod{'C'}},
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

    struct movable {
        int index = 0;
        amphipod occupant = EMPTY_SPOT;
        int depth = 0;

        bool operator==(const movable& rhs) const noexcept {
            return index == rhs.index &&
                occupant == rhs.occupant &&
                depth == rhs.depth;
        }
        bool operator<(const movable& rhs) const noexcept {
            return std::tie(index, occupant, depth) <
                std::tie(rhs.index, rhs.occupant, rhs.depth);
        }
    };

    struct available {
        int index = 0;
        int depth = 0;
        char other_occupant = EMPTY_SPOT;

        bool operator==(const available& rhs) const noexcept {
            return index == rhs.index &&
                    other_occupant == rhs.other_occupant &&
                   depth == rhs.depth;
        }
        bool operator<(const available& rhs) const noexcept {
            return std::tie(index, other_occupant, depth) <
                   std::tie(rhs.index, rhs.other_occupant, rhs.depth);
        }
    };

    std::vector<movable> get_movable_occupants(const state& s) {
        std::vector<movable> retval;
        retval.reserve(8);
        int idx = 0;
        for (const auto& spot : s) {
            if (spot.is_room) {
                if (spot.at(0) != EMPTY_SPOT && spot.at(1) != EMPTY_SPOT && spot.at(0) == spot.at(1)) {
                    //Happy
                }
                else if (spot.at(0) != EMPTY_SPOT) {
                    retval.push_back({idx, spot.at(0), 1});
                }
                else if (spot.at(1) != EMPTY_SPOT) {
                    retval.push_back({idx, spot.at(1), 2});
                }
            }
            else if (spot.at(0) != EMPTY_SPOT) {
                retval.push_back({idx, spot.at(0), 0});
            }
            ++idx;
        }
        return retval;
    }

    std::vector<available> get_available_locations(const state& s) {
        std::vector<available> retval;
        retval.reserve(11);
        int idx = 0;
        for (const auto& spot : s) {
            if (spot.is_room) {
                if (spot.at(0) != EMPTY_SPOT) {
                    //Taken
                }
                else if (spot.at(1) != EMPTY_SPOT) {
                    retval.push_back({idx, 1, spot.at(1)});
                }
                else {
                    retval.push_back({idx, 2, EMPTY_SPOT});
                }
            }
            else if (spot.at(0) == EMPTY_SPOT) {
                retval.push_back({idx, 0, EMPTY_SPOT});
            }
            ++idx;
        }
        return retval;
    }

    bool clear_route(const state& s, const int start, const int end) {
        if (start < 0 || start >= s.size() || end < 0 || end >= s.size() || start == end) {
            return false;
        }
        if (start < end) {
            for (int idx = start + 1; idx < end; ++idx) {
                if (!s[idx].is_room && s[idx].upper != EMPTY_SPOT) {
                    return false;
                }
            }
        }
        else {
            for (int idx = end + 1; idx < start; ++idx) {
                if (!s[idx].is_room && s[idx].upper != EMPTY_SPOT) {
                    return false;
                }
            }
        }
        return true;
    }

    struct move {
        state new_state;
        int energy_cost;

        constexpr move() = default;
        constexpr move(const state& ns) : new_state{ns}, energy_cost{0} {}
        constexpr move(const state& ns, int cost) : new_state{ns}, energy_cost{cost} {}

        bool operator<(const move& rhs) const noexcept {
            return energy_cost < rhs.energy_cost;
        }
    };

    struct actual_move {
        movable from;
        available to;

        bool operator==(const actual_move& rhs) const noexcept {
            return from == rhs.from &&
                   to == rhs.to;
        }
        bool operator<(const actual_move& rhs) const noexcept {
            return std::tie(from, to) <
                   std::tie(rhs.from, rhs.to);
        }
    };

    std::optional<move> try_move(const state& current, const movable& mv, const available& av) {
        if (mv.occupant == EMPTY_SPOT ||    //Trying to move nobody
            //mv.occupant.moves >= 2 ||    //Can only move out and then move back in
            (mv.depth == 0 && av.depth == 0) ||     //From hallway to hallway is banned
            !clear_route(current, mv.index, av.index) ||    //Don't block the route
            (av.depth > 0 && av.index != expected_destination(mv.occupant)) ||  //They have expected destinations!
            (av.other_occupant != EMPTY_SPOT && mv.occupant != av.other_occupant))  //Can only move to a room with a same type occupant
        {
            return std::nullopt;
        }
        state ns = current;
        int energy = std::abs(mv.index - av.index);
        //Move out
        if (ns[mv.index].is_room) {
            ns[mv.index].at(mv.depth - 1) = EMPTY_SPOT;
            energy += mv.depth;
        }
        else {
            ns[mv.index].at(0) = EMPTY_SPOT;
        }
        //Move in
        if (ns[av.index].is_room) {
            ns[av.index].at(av.depth - 1) = mv.occupant;
            energy += av.depth;
            //++ns[av.index].at(av.depth - 1).moves;
        }
        else {
            ns[av.index].at(0) = mv.occupant;
            //++ns[av.index].at(0).moves;
        }
        return move{ns, energy * energy_per_move(mv.occupant)};
    }

    bool is_happy(const state& s) {
        int idx = 0;
        for (const auto& spot : s) {
            if (spot.is_room) {
                if (spot.at(0) == EMPTY_SPOT ||
                    spot.at(1) == EMPTY_SPOT ||
                    spot.at(0) != spot.at(1) ||
                    expected_destination(spot.at(0)) != idx)
                {
                    return false;
                }
            }
            else {
                if (spot.at(0) != EMPTY_SPOT) {
                    return false;
                }
            }
            ++idx;
        }
        return true;
    }

    int count_ams(const state& s) {
        int res = 0;
        for (const auto& p : s) {
            res += (p.upper != EMPTY_SPOT ? 1 : 0) + (p.lower != EMPTY_SPOT ? 1 : 0);
        }
        return res;
    }

    std::vector<move> iterate(const state& s) {
        std::vector<move> retval;
        retval.reserve(50);
        int idx = 0;
        for (const auto& stop : s) {
            if (stop.is_room) {
                int to_hw_cost = 0;
                char type = EMPTY_SPOT;
                bool other_same = false;
                int at = 0;
                if (stop.upper != EMPTY_SPOT) {
                    type = stop.upper;
                    to_hw_cost = 1;
                    other_same = type == stop.lower;
                }
                else if (stop.lower != EMPTY_SPOT && expected_destination(stop.lower) != idx) {
                    at = 1;
                    type = stop.lower;
                    to_hw_cost = 2;
                    other_same = true;
                }
                if (type != EMPTY_SPOT) {
                    const int dest = expected_destination(type);
                    if (dest == idx && other_same) {
                        ++idx;
                        continue;
                    }
                    const bool lower_dest_empty = s[dest].lower == EMPTY_SPOT;
                    if (clear_route(s, idx, dest) && s[dest].upper == EMPTY_SPOT && (lower_dest_empty || s[dest].lower == type)) {
                        retval.emplace_back(s, (std::abs(dest - idx) + (lower_dest_empty ? 2 : 1) + to_hw_cost) *
                                energy_per_move(type));
                        retval.back().new_state[idx].at(at) = EMPTY_SPOT;
                        retval.back().new_state[dest].at(lower_dest_empty ? 1 : 0) = stop.at(at);
                    }
                    else {
                        for (int hw = 0; hw < s.size(); ++hw) {
                            if (!s[hw].is_room && s[hw].upper == EMPTY_SPOT && clear_route(s, idx, hw)) {
                                retval.emplace_back(s, (std::abs(hw - idx) + to_hw_cost) * energy_per_move(type));
                                retval.back().new_state[idx].at(at) = EMPTY_SPOT;
                                retval.back().new_state[hw].upper = stop.at(at);
                            }
                        }
                    }
                }
            }
            else if (stop.upper != EMPTY_SPOT) {
                const int dest = expected_destination(stop.upper);
                const bool lower_dest_empty = s[dest].lower == EMPTY_SPOT;
                if (clear_route(s, idx, dest) && s[dest].upper == EMPTY_SPOT && (lower_dest_empty || s[dest].lower == stop.upper)) {
                    retval.emplace_back(s, (std::abs(dest - idx) + (lower_dest_empty ? 2 : 1)) * energy_per_move(stop.upper));
                    retval.back().new_state[idx].upper = EMPTY_SPOT;
                    retval.back().new_state[dest].at(lower_dest_empty ? 1 : 0) = stop.upper;
                }
            }
            ++idx;
        }
        return retval;


//        const auto movable = get_movable_occupants(s);
//        const auto available = get_available_locations(s);
//        std::vector<move> moves;
//        for (const auto& mv : movable) {
//            for (const auto& av : available) {
//                auto res = try_move(s, mv, av);
//                if (res) {
//                    moves.push_back(*res);
//                }
//            }
//        }
//        std::sort(moves.begin(), moves.end(), [](const auto& p1, const auto& p2){ return p1.energy_cost < p2.energy_cost; });
//        return moves;
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

    int run(const state& s, std::vector<cache_line>& seen, int current_energy = 0) {
        const auto found1 = std::lower_bound(seen.cbegin(), seen.cend(), s, [](const cache_line& c, const state& s){ return c.s < s; });
        if (found1 != seen.cend() && s == found1->s) {
            return found1->cost;
        }
        else if (is_happy(s)) {
            return current_energy;
        }
        const auto res = iterate(s);
        int min = std::numeric_limits<int>::max();
        for (const auto& r : res) {
            auto rmin = run(r.new_state, seen, r.energy_cost + current_energy);
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

    */
    std::string part_1(const std::filesystem::path& input_dir) {
        std::vector<cache_line> seen;
        //18188 too high
        //14855 too low
        //20566 too high, duh
        return std::to_string(run(INPUT, seen));
    }

    /*

    */
    std::string part_2(const std::filesystem::path& input_dir) {
        return std::to_string(-1);
    }

    aoc::registration r {2021, 23, part_1, part_2};

    TEST_SUITE("2021_day23") {
        TEST_CASE("2021_day23:example") {
            std::vector<cache_line> seen;
            constexpr state EXAMPLE = {
                    spot{},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'A'}},
                    spot{},
                    spot{true, amphipod{'C'}, amphipod{'D'}},
                    spot{},
                    spot{true, amphipod{'B'}, amphipod{'C'}},
                    spot{},
                    spot{true, amphipod{'D'}, amphipod{'A'}},
                    spot{},
                    spot{}
            };
            REQUIRE_EQ(run(EXAMPLE, seen), 12521);
        }
    }

}