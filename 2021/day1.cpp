//
// Created by Dan on 12/1/2021.
//

#include "registration.h"

#include <vector>
#include <cstdint>
#include <iostream>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<int> get_input(const std::vector<std::string>& lines) {
        std::vector<int> values;
        std::transform(lines.begin(), lines.end(), std::back_inserter(values), [](const std::string& s){ return std::stoi(s); });
        return values;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto depths = get_input(lines);
        int last = std::numeric_limits<int>::max(), count = 0;
        for (const auto d : depths) {
            if (d > last) {
                ++count;
            }
            last = d;
        }
        return std::to_string(count);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto depths = get_input(lines);
        const auto end = depths.end() - 3;
        int count = 0;
        for (auto current = depths.begin(); current != end; ++current) {
            if (*(current + 3) > *current) {
                ++count;
            }
        }
        return std::to_string(count);
    }

    aoc::registration r {2021, 1, part_1, part_2};

}