//
// Created by Dan on 12/5/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <algorithm>
#include <numeric>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/5
     */

    using rule = std::pair<int, int>;

    struct input_info {
        std::vector<rule> rules;
        std::vector<std::vector<int>> updates;
    };

    rule parse_rule(std::string_view s) {
        const auto p = split(s, '|');
        return {parse<int>(p[0]), parse<int>(p[1])};
    }

    std::vector<int> parse_update(std::string_view s) {
        const auto p = split(s, ',');
        return p | std::views::transform([](std::string_view n){ return parse<int>(n); }) | std::ranges::to<std::vector>();
    }

    input_info get_input(const std::vector<std::string>& lines) {
        const auto divider = std::find_if(lines.begin(), lines.end(), [](const std::string& s){ return s.empty(); });
        return {
            std::ranges::subrange{lines.begin(), divider} | std::views::transform(&parse_rule) | std::ranges::to<std::vector>(),
            std::ranges::subrange{divider + 1, lines.end()} | std::views::transform(&parse_update) | std::ranges::to<std::vector>()
        };
    }

    bool is_ordered(const std::vector<int>& update, const rule& r) {
        const auto found1 = std::find(update.begin(), update.end(), r.first);
        const auto found2 = std::find(update.begin(), update.end(), r.second);
        if (found1 != update.end() && found2 != update.end()) {
            return found1 < found2;
        }
        else {
            return true;
        }
    }

    bool is_ordered(const std::vector<int>& update, const std::vector<rule>& rules) {
        return std::all_of(rules.begin(), rules.end(), [&update](const rule& r){ return is_ordered(update, r); });
    }

    bool fix_update(std::vector<int>& update, const rule& r) {
        const auto found1 = std::find(update.begin(), update.end(), r.first);
        const auto found2 = std::find(update.begin(), update.end(), r.second);
        if (found1 != update.end() && found2 != update.end() && found1 < found2) {
            std::swap(*found1, *found2);
            return true;
        }
        else {
            return false;
        }
    }

    std::vector<int> fix_update(const std::vector<int>& update, const std::vector<rule>& rules) {
        auto retval = update;
        auto applicable = rules | std::views::filter([&update](const rule& r){
            return std::find(update.begin(), update.end(), r.first) != update.end() &&
                    std::find(update.begin(), update.end(), r.second) != update.end();
        });
        bool fixed = true;
        while (fixed) {
            fixed = false;
            for (const auto &r: applicable) {
                fixed = fix_update(retval, r) || fixed;
            }
        }
        return retval;
    }

    int middle_page_for_ordered(const std::vector<int>& update, const std::vector<std::pair<int, int>>& rules) {
        if (is_ordered(update, rules)) {
            return update[update.size() / 2];
        }
        else {
            return 0;
        }
    }

    int middle_page_for_fixed(const std::vector<int>& update, const std::vector<std::pair<int, int>>& rules) {
        if (is_ordered(update, rules)) {
            return 0;
        }
        else {
            auto fixed = fix_update(update, rules);
            return fixed[fixed.size() / 2];
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto result = std::accumulate(input.updates.begin(), input.updates.end(), 0, [&input](int total, const std::vector<int>& upd){ return total +
                middle_page_for_ordered(upd, input.rules); });
        return std::to_string(result);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto result = std::accumulate(input.updates.begin(), input.updates.end(), 0, [&input](int total, const std::vector<int>& upd){ return total +
                middle_page_for_fixed(upd, input.rules); });
        return std::to_string(result);
    }

    aoc::registration r{2024, 5, part_1, part_2};

//    TEST_SUITE("2024_day05") {
//        TEST_CASE("2024_day05:example") {
//
//        }
//    }

} /* namespace <anon> */