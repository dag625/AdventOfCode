//
// Created by Dan on 11/12/2023.
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
    using namespace std::string_view_literals;

    constexpr int NUM_ITEMS = 5;
    constexpr std::array<std::string_view, NUM_ITEMS> ITEM_NAMES { "strontium"sv, "plutonium"sv, "thulium"sv, "ruthenium"sv, "curium"sv };
    constexpr int GENERATOR_OFFSET = 0;
    constexpr int CHIP_OFFSET = 1;
    constexpr int DESTINATION = 3;

    struct state {
        int step = 0;
        int elevator = 0;
        std::array<int, 10> items{};

        bool operator<(const state& rhs) const noexcept {
            return std::tie(elevator, items) < std::tie(rhs.elevator, rhs.items);
        }
        bool operator==(const state& rhs) const noexcept {
            return std::tie(elevator, items) == std::tie(rhs.elevator, rhs.items);
        }
        bool operator!=(const state& rhs) const noexcept {
            return std::tie(elevator, items) != std::tie(rhs.elevator, rhs.items);
        }
    };

    state get_input(const fs::path &input_dir) {
        state retval;
        //thulium:
        retval.items[4] = 1;
        retval.items[5] = 2;
        //ruthenium:
        retval.items[6] = 1;
        retval.items[7] = 1;
        //curium:
        retval.items[8] = 1;
        retval.items[9] = 1;
        return retval;
    }

    bool is_valid(const state& s) {
        for (int i = 0; i < NUM_ITEMS; ++i) {
            int chip = s.items[2*i + CHIP_OFFSET];
            int gen = s.items[2*i + GENERATOR_OFFSET];
            if (chip != gen) {
                for (int j = 0; j < NUM_ITEMS; ++j) {
                    if (s.items[2*j + GENERATOR_OFFSET] == chip) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool is_done(const state& s) {
        return s.elevator == DESTINATION &&
                std::all_of(s.items.begin(), s.items.end(), [](const int f){ return f == DESTINATION; });
    }

    std::vector<state> possible_next_steps(const state& start) {
        std::vector<state> retval;
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

            next.elevator = start.elevator + 1;
            next.items[i] = start.elevator + 1;
            if (next.elevator <= DESTINATION && is_valid(next)) {
                retval.push_back(next);
            }

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

                next.elevator = start.elevator + 1;
                next.items[i] = start.elevator + 1;
                next.items[j] = start.elevator + 1;
                if (next.elevator <= DESTINATION && is_valid(next)) {
                    retval.push_back(next);
                }
            }
        }
        return retval;
    }

    /*

    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    /*

    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    aoc::registration r{2016, 11, part_1, part_2};

//    TEST_SUITE("2016_day11") {
//        TEST_CASE("2016_day11:example") {
//
//        }
//    }

}