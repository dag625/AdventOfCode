//
// Created by Dan on 12/27/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <array>

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr auto INPUT = std::array{
            50,
            44,
            11,
            49,
            42,
            46,
            18,
            32,
            26,
            40,
            21,
            7,
            18,
            43,
            10,
            47,
            36,
            24,
            22,
            40
    };

    int get_capacity(const std::vector<int>& containers) {
        int retval = 0;
        for (const auto idx : containers) {
            retval += INPUT[idx];
        }
        return retval;
    }

    std::vector<std::vector<int>> combos_starting_with(const std::vector<int>& current, const int capacity) {
        const auto current_cap = get_capacity(current);
        if (current_cap > capacity) {
            return {};
        }
        else if (current_cap == capacity) {
            return {current};
        }
        else {
            std::vector<std::vector<int>> retval;
            std::vector<int> next = current;
            next.push_back(-1);
            for (int i = current.back() + 1; i < INPUT.size(); ++i) {
                next.back() = i;
                auto res = combos_starting_with(next, capacity);
                retval.insert(retval.end(), res.begin(), res.end());
            }
            return retval;
        }
    }

    std::vector<std::vector<int>> combos_of_containers(const int capacity) {
        std::vector<std::vector<int>> retval;
        for (int i = 0; i < INPUT.size(); ++i) {
            auto res = combos_starting_with({i}, capacity);
            retval.insert(retval.end(), res.begin(), res.end());
        }
        return retval;
    }


    /*
    --- Day 17: No Such Thing as Too Much ---
    The elves bought too much eggnog again - 150 liters this time. To fit it all into your refrigerator, you'll need to move it into smaller containers. You take an inventory of the capacities of the available containers.

    For example, suppose you have containers of size 20, 15, 10, 5, and 5 liters. If you need to store 25 liters, there are four ways to do it:

    15 and 10
    20 and 5 (the first 5)
    20 and 5 (the second 5)
    15, 5, and 5
    Filling all containers entirely, how many different combinations of containers can exactly fit all 150 liters of eggnog?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto combos = combos_of_containers(150);
        return std::to_string(combos.size());
    }

    /*
    --- Part Two ---
    While playing with all the containers in the kitchen, another load of eggnog arrives! The shipping and receiving department is requesting as many containers as you can spare.

    Find the minimum number of containers that can exactly fit all 150 liters of eggnog. How many different ways can you fill that number of containers and still hold exactly 150 litres?

    In the example above, the minimum number of containers was two. There were three ways to use that many containers, and so the answer there would be 3.
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto combos = combos_of_containers(150);
        const auto min_combo = std::min_element(combos.begin(), combos.end(), [](const std::vector<int>& a, const std::vector<int>& b){ return a.size() < b.size(); });
        return std::to_string(std::count_if(combos.begin(), combos.end(), [sz = min_combo->size()](const std::vector<int>& a){ return a.size() == sz; }));
    }

    aoc::registration r {2015, 17, part_1, part_2};

//    TEST_SUITE("2015_day17") {
//        TEST_CASE("2015_day17:example") {
//
//        }
//    }

}