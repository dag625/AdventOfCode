//
// Created by Dan on 12/27/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <set>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct conversion {
        std::string from;
        std::string to;
    };

    conversion to_conversion(std::string_view s) {
        const auto parts = split(s, " => ");
        return {std::string{parts[0]}, std::string{parts[1]}};
    }

    std::pair<std::vector<conversion>, std::string> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2015" / "day_19_input.txt");
        return {lines | std::views::take(lines.size() - 2) | std::views::transform(&to_conversion) | to<std::vector<conversion>>(), lines.back()};
    }

    std::set<std::string> convert_once(const std::string& orig, const conversion& c) {
        std::set<std::string> retval;
        std::string::size_type pos = orig.find(c.from, 0);
        while (pos != std::string::npos) {
            auto repl = orig;
            repl.replace(pos, c.from.size(), c.to, 0);
            retval.insert(repl);
            pos = orig.find(c.from, pos + 1);
        }
        return retval;
    }


    /*

    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto [conversions, orig] = get_input(input_dir);
        std::set<std::string> results;
        for (const auto& c : conversions) {
            auto converted = convert_once(orig, c);
            results.insert(converted.begin(), converted.end());
        }
        return std::to_string(results.size());
    }

    /*

    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    aoc::registration r {2015, 19, part_1, part_2};

    TEST_SUITE("2015_day19") {
        TEST_CASE("2015_day19:example") {
            std::vector<conversion> conversions = {
                    {"H", "HO"},
                    {"H", "OH"},
                    {"O", "HH"}
            };
            std::string test1 = "HOH";
            std::set<std::string> results1;
            for (const auto& c : conversions) {
                auto converted = convert_once(test1, c);
                results1.insert(converted.begin(), converted.end());
            }
            REQUIRE_EQ(results1.size(), 4);
        }
        TEST_CASE("2015_day19:test1") {
            std::vector<conversion> conversions = {
                    {"Ho", "Heh"}
            };
            std::string test1 = "oHoHHoTHo";
            std::set<std::string> results1;
            auto converted = convert_once(test1, conversions[0]);
            REQUIRE_EQ(converted.size(), 3);
        }
        TEST_CASE("2015_day19:test2") {
            std::vector<conversion> conversions = {
                    {"Ho", "Heh"}
            };
            std::string test1 = "xoxoxHoooxox";
            std::set<std::string> results1;
            auto converted = convert_once(test1, conversions[0]);
            REQUIRE_EQ(converted.size(), 1);
            REQUIRE_EQ(*converted.begin(), "xoxoxHehooxox");
        }
    }

}