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

    struct room {
        std::array<char, 2> occupants;
    };

    struct hallway {
        char occupant = EMPTY_SPOT;
    };

    using spot = std::variant<hallway, room>;
    using state = std::array<spot, HALLWAY_LEN>;

    /*
    This is the raw input from AoC:

    #############
    #...........#
    ###A#C#B#B###
      #D#D#A#C#
      #########
     */
    constexpr state INPUT = {
            hallway{},
            hallway{},
            room{{'A', 'D'}},
            hallway{},
            room{{'C', 'D'}},
            hallway{},
            room{{'B', 'A'}},
            hallway{},
            room{{'B', 'C'}},
            hallway{},
            hallway{}
    };

    struct movable {
        int index = 0;
        char occupant = EMPTY_SPOT;
        int depth = 0;
    };

    struct available {
        int index = 0;
        int depth = 0;
        char other_occupant = EMPTY_SPOT;
    };

    struct movable_occupant_getter {
        int index = 0;
        explicit movable_occupant_getter(int idx) : index{idx} {}
        movable operator()(const hallway& h) const noexcept {
            return {index, h.occupant, 0};
        }
        movable operator()(const room& r) const noexcept {
            if (r.occupants[0] != EMPTY_SPOT && r.occupants[1] != EMPTY_SPOT && r.occupants[0] == r.occupants[1]) {
                //Happy
                return {index, EMPTY_SPOT, 0};
            }
            else if (r.occupants[0] != EMPTY_SPOT) {
                return {index, r.occupants[0], 1};
            }
            else if (r.occupants[1] != EMPTY_SPOT) {
                return {index, r.occupants[1], 2};
            }
            else {
                return {index, EMPTY_SPOT, 0};
            }
        }
    };

    struct available_loc_getter {
        int index = 0;
        explicit available_loc_getter(int idx) : index{idx} {}
        std::optional<available> operator()(const hallway& h) const noexcept {
            if (h.occupant == EMPTY_SPOT) {
                return available{index, 0, EMPTY_SPOT};
            }
            else {
                return std::nullopt;
            }
        }
        std::optional<available> operator()(const room& r) const noexcept {
            if (r.occupants[0] != EMPTY_SPOT) {
                return std::nullopt;
            }
            else if (r.occupants[1] != EMPTY_SPOT) {
                return available{index, 1, r.occupants[1]};
            }
            else {
                return available{index, 2, EMPTY_SPOT};
            }
        }
    };

    struct occupied_getter {
        int index = 0;
        explicit occupied_getter(int idx) : index{idx} {}
        bool operator()(const hallway& h) const noexcept {
            return h.occupant != EMPTY_SPOT;
        }
        bool operator()(const room& r) const noexcept {
            return false;
        }
    };

    struct move_out {
        movable mv;
        explicit move_out(const movable& m) : mv{m} {}
        int operator()(hallway& h) const noexcept {
            h.occupant = EMPTY_SPOT;
            return 0;
        }
        int operator()(room& r) const noexcept {
            r.occupants[mv.depth - 1] = EMPTY_SPOT;
            return mv.depth;
        }
    };

    struct move_in {
        movable mv;
        available av;
        explicit move_in(const movable& m, const available& a) : mv{m}, av{a} {}
        int operator()(hallway& h) const noexcept {
            h.occupant = mv.occupant;
            return 0;
        }
        int operator()(room& r) const noexcept {
            r.occupants[av.depth - 1] = mv.occupant;
            return av.depth;
        }
    };

    std::vector<movable> get_movable_occupants(const state& s) {
        std::vector<movable> retval;
        int idx = 0;
        for (const auto& spot : s) {
            auto res = std::visit(movable_occupant_getter{idx}, spot);
            ++idx;
            if (res.occupant != EMPTY_SPOT) {
                retval.push_back(res);
            }
        }
        return retval;
    }

    std::vector<available> get_available_locations(const state& s) {
        std::vector<available> retval;
        int idx = 0;
        for (const auto& spot : s) {
            auto res = std::visit(available_loc_getter{idx}, spot);
            ++idx;
            if (res) {
                retval.push_back(*res);
            }
        }
        return retval;
    }

    constexpr int energy_per_move(char c) {
        switch (c) {
            case 'A': return 1;
            case 'B': return 10;
            case 'C': return 100;
            case 'D': return 1000;
            default:  return -1;
        }
    }

    bool clear_route(const state& s, const int start, const int end) {
        if (start < 0 || start >= s.size() || end < 0 || end >= s.size() || start == end) {
            return false;
        }
        const int delta = start < end ? 1 : -1;
        for (int idx = start + delta; idx < end; idx += delta) {
            if (std::visit(occupied_getter{idx}, s[idx])) {
                return false;
            }
        }
        return true;
    }

    struct move {
        state new_state;
        int energy_cost;

        bool operator<(const move& rhs) const noexcept {
            return energy_cost < rhs.energy_cost;
        }
    };

    std::optional<move> try_move(const state& current, const movable& mv, const available& av) {
        if (mv.occupant == EMPTY_SPOT ||    //Trying to move nobody
            (mv.depth == 0 && av.depth == 0) ||     //From hallway to hallway is banned
            !clear_route(current, mv.index, av.index) ||    //Don't block the route
            (av.other_occupant != EMPTY_SPOT && mv.occupant != av.other_occupant))  //Can only move to a room with a same type occuptant
        {
            return std::nullopt;
        }
        state ns = current;
        int energy = std::abs(mv.index - av.index);
        energy += std::visit(move_out{mv}, ns[mv.index]);
        energy += std::visit(move_in{mv, av}, ns[av.index]);
        return move{ns, energy * energy_per_move(mv.occupant)};
    }

    bool is_happy(const state& s) {
        bool happy = true;
        for (const auto& spot : s) {
            happy = std::visit([](auto& loc) -> bool {
                if constexpr(std::is_same_v<hallway, std::remove_cvref<decltype(loc)>>) {
                    return loc.occupant == EMPTY_SPOT;
                }
                else if constexpr(std::is_same_v<room, std::remove_cvref<decltype(loc)>>) {
                    return loc.occupants[0] != EMPTY_SPOT &&
                            loc.occupants[1] != EMPTY_SPOT &&
                            loc.occupants[0] == loc.occupants[1];
                }
                return false;
            }, spot);
            if (!happy) {
                break;
            }
        }
        return happy;
    }

    std::vector<move> iterate(const state& s, int current_energy = 0) {
        const auto movable = get_movable_occupants(s);
        const auto available = get_available_locations(s);
        std::vector<move> moves;
        for (const auto& mv : movable) {
            for (const auto& av : available) {
                auto res = try_move(s, mv, av);
                if (res) {
                    moves.push_back(*res);
                    moves.back().energy_cost += current_energy;
                }
            }
        }
        std::sort(moves.begin(), moves.end());
        return moves;
    }

    int run(const state& s) {
        std::vector<move> available, completed;
        available.push_back({s, 0});
        int iter = 0;
        while (!available.empty()) {
            std::vector<move> next;
            for (const auto& mv : available) {
                auto res = iterate(mv.new_state, mv.energy_cost);
                const auto next_mid = static_cast<int64_t>(next.size());
                const auto compl_mid = static_cast<int64_t>(completed.size());
                std::partition_copy(res.begin(), res.end(), std::back_inserter(completed), std::back_inserter(next),
                                    [](const move& m){ return is_happy(m.new_state); });
                std::inplace_merge(next.begin(), next.begin() + next_mid, next.end());
                std::inplace_merge(completed.begin(), completed.begin() + compl_mid, completed.end());
            }
            available.swap(next);
            ++iter;
        }
        return completed.front().energy_cost;
    }

    /*

    */
    std::string part_1(const std::filesystem::path& input_dir) {
        return std::to_string(run(INPUT));
    }

    /*

    */
    std::string part_2(const std::filesystem::path& input_dir) {
        return std::to_string(-1);
    }

    aoc::registration r {2021, 23, part_1, part_2};

//    TEST_SUITE("2021_day23") {
//        TEST_CASE("2021_day23:example") {
//
//        }
//    }

}