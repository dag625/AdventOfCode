//
// Created by Dan on 11/12/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <compare>
#include <numeric>

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr int NUM_ITEMS_P1 = 5;
    constexpr int NUM_ITEMS_P2 = 7;
    constexpr std::array<std::string_view, NUM_ITEMS_P2> ITEM_NAMES { "strontium"sv, "plutonium"sv, "thulium"sv, "ruthenium"sv, "curium"sv, "elerium"sv, "dilithium"sv };
    constexpr int GENERATOR_OFFSET = 0;
    constexpr int CHIP_OFFSET = 1;
    constexpr int DESTINATION = 3;

    template <int NUM>
    struct stepless_state {
        int elevator = 0;
        std::array<int, NUM * 2> items{};

        std::strong_ordering operator<=>(const stepless_state& rhs) const noexcept { return std::tie(elevator, items) <=> std::tie(rhs.elevator, rhs.items); }
        bool operator==(const stepless_state& rhs) const noexcept { return std::tie(elevator, items) == std::tie(rhs.elevator, rhs.items); }
    };

    template <int NUM>
    struct state : public stepless_state<NUM> {
        int step = 0;

        [[nodiscard]] stepless_state<NUM> stepless() const noexcept { return static_cast<stepless_state<NUM>>(*this); }

        std::strong_ordering operator<=>(const state& rhs) const noexcept { return step <=> rhs.step; }
    };

    template <int NUM>
    state<NUM> get_input(const fs::path &input_dir) {
        state<NUM> retval;
        //strontium:
        retval.items[0] = 0;
        retval.items[1] = 0;
        //plutonium:
        retval.items[2] = 0;
        retval.items[3] = 0;
        //thulium:
        retval.items[4] = 1;
        retval.items[5] = 2;
        //ruthenium:
        retval.items[6] = 1;
        retval.items[7] = 1;
        //curium:
        retval.items[8] = 1;
        retval.items[9] = 1;
        //Note for NUM == NUM_PARTS_P2 for part 2, all the parts are on the ground level and should be set to 0,
        //which is the default/init case so I won't bother putting the explicit assignments here.
        return retval;
    }

    template <int NUM>
    bool is_valid(const state<NUM>& s) {
        for (int i = 0; i < NUM; ++i) {
            int chip = s.items[2*i + CHIP_OFFSET];
            int gen = s.items[2*i + GENERATOR_OFFSET];
            if (chip != gen) {
                for (int j = 0; j < NUM; ++j) {
                    if (s.items[2*j + GENERATOR_OFFSET] == chip) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    template <int NUM>
    bool is_done(const state<NUM>& s) {
        return s.elevator == DESTINATION &&
                std::all_of(s.items.begin(), s.items.end(), [](const int f){ return f == DESTINATION; });
    }

    template <int NUM>
    std::vector<state<NUM>> possible_next_steps(const state<NUM>& start) {
        std::vector<state<NUM>> retval;
        for (int i = 0; i < start.items.size(); ++i) {
            if (start.elevator != start.items[i]) {
                continue;
            }
            state next = start;
            ++next.step;

            next.elevator = start.elevator - 1;
            next.items[i] = start.elevator - 1;
            if (next.elevator >= 0 && is_valid(next)) {
                retval.push_back(next);
            }
            next.items = start.items;

            next.elevator = start.elevator + 1;
            next.items[i] = start.elevator + 1;
            if (next.elevator <= DESTINATION && is_valid(next)) {
                retval.push_back(next);
            }
            next.items = start.items;

            for (int j = i + 1; j < start.items.size(); ++j) {
                if (start.elevator != start.items[j]) {
                    continue;
                }

                next.elevator = start.elevator - 1;
                next.items[i] = start.elevator - 1;
                next.items[j] = start.elevator - 1;
                if (next.elevator >= 0 && is_valid(next)) {
                    retval.push_back(next);
                }
                next.items = start.items;

                next.elevator = start.elevator + 1;
                next.items[i] = start.elevator + 1;
                next.items[j] = start.elevator + 1;
                if (next.elevator <= DESTINATION && is_valid(next)) {
                    retval.push_back(next);
                }
                next.items = start.items;
            }
        }
        return retval;
    }

    template <int NUM>
    int do_with_size(const std::filesystem::path &input_dir) {
        const auto input = get_input<NUM>(input_dir);
        std::vector<state<NUM>> choices;
        std::vector<stepless_state<NUM>> seen;
        choices.push_back(input);
        seen.push_back(input.stepless());
        while (!choices.empty()) {
            auto next = possible_next_steps(choices.front());
            const auto finished = std::find_if(next.begin(), next.end(), &is_done<NUM>);
            if (finished != next.end()) {
                return finished->step;
            }
            choices.erase(choices.begin());
            for (const auto& n : next) {
                const auto sn = n.stepless();
                const auto is_seen = std::lower_bound(seen.begin(), seen.end(), sn);
                if (is_seen == seen.end() || *is_seen != sn) {
                    seen.insert(is_seen, sn);
                    const auto mid = choices.size();
                    choices.push_back(n);
                    std::inplace_merge(choices.begin(), choices.begin() + mid, choices.end());
                }
            }
        }
        return -1;
    }

    /*

    */
    std::string part_1(const std::filesystem::path &input_dir) {
        return std::to_string(do_with_size<NUM_ITEMS_P1>(input_dir));
    }

    /*

    */
    std::string part_2(const std::filesystem::path &input_dir) {
        return std::to_string(do_with_size<NUM_ITEMS_P2>(input_dir));
    }

    aoc::registration r{2016, 11, part_1, part_2};

//    TEST_SUITE("2016_day11") {
//        TEST_CASE("2016_day11:example") {
//
//        }
//    }

}