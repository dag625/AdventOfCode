//
// Created by Dan on 12/11/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <regex>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<std::string> get_input(const fs::path& input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_12_input.txt");
        std::vector<std::string> retval;
        retval.reserve(lines.size());
        for (const auto& s : lines) {
            retval.push_back(s);
        }
        return retval;
    }

    /*

    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    /*

    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        return std::to_string(-1);
    }

    aoc::registration r {2021, 12, part_1, part_2};

//    TEST_SUITE("2021_day12") {
//        TEST_CASE("2021_day12:example") {

//        }
//    }

}