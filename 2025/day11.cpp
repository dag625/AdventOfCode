//
// Created by Dan on 12/11/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2025/day/11
     */

    struct device {
        std::string name;
        std::vector<std::string> outputs;

        auto operator<=>(const device& rhs) const noexcept{ return name <=> rhs.name; }
        auto operator<=>(const std::string& rhs) const noexcept{ return name <=> rhs; }
    };

    device parse_dev(std::string_view str) {
        const auto parts = split(str, ':');
        const auto outs = split_no_empty(parts[1], ' ');
        return {
            std::string{parts[0]},
            outs | std::views::transform([](std::string_view s){ return std::string{s}; }) | std::ranges::to<std::vector>()
        };
    }

    std::vector<device> get_input(const std::vector<std::string>& lines) {
        auto retval = lines | std::views::transform(&parse_dev) | std::ranges::to<std::vector>();
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    using seen_list = std::vector<std::pair<std::string, int64_t>>;

    int64_t count_paths_to(const std::vector<device>& devs, const std::string& from, const std::string_view to, seen_list& seen) {
        if (from == to) {
            return 1;
        }
        if (from == "out") {
            //For part 2 we can get here without it being our destination, but it's a valid node.
            return 0;
        }

        //Assume no cycles...
        const auto already_seen = std::find_if(seen.begin(), seen.end(), [&from](const auto& s){ return s.first == from; });
        if (already_seen !=  seen.end()) {
            return already_seen->second;
        }

        const auto found = std::lower_bound(devs.begin(), devs.end(), from);
        if (found == devs.end() || found->name != from) {
            throw std::logic_error{fmt::format("No such device '{}'.", from)};
        }
        int64_t total = 0;
        for (const auto& next : found->outputs) {
            const auto sub_paths = count_paths_to(devs, next, to, seen);
            total += sub_paths;
        }
        seen.emplace_back(from, total);
        return total;
    }

    int64_t count_paths_to(const std::vector<device>& devs, const std::string& from, const std::string_view to) {
        seen_list seen;
        return count_paths_to(devs, from, to, seen);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto res = count_paths_to(input, "you", "out");
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto s2d = count_paths_to(input, "svr", "dac");
        const auto s2f = count_paths_to(input, "svr", "fft");
        const auto f2d = count_paths_to(input, "fft", "dac");
        const auto d2f = count_paths_to(input, "dac", "fft");
        const auto f2o = count_paths_to(input, "fft", "out");
        const auto d2o = count_paths_to(input, "dac", "out");
        const auto total = s2d * d2f * f2o + s2f * f2d * d2o;
        return std::to_string(total);
    }

    aoc::registration r {2025, 11, part_1, part_2};

//    TEST_SUITE("2025_day11") {
//        TEST_CASE("2025_day11:example") {
//            const std::vector<std::string> lines {
//            
//            };
//            const auto input = get_input(lines);
//
//        }
//    }

} /* namespace <anon> */