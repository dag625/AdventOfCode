//
// Created by Dan on 12/19/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>
#include <iostream>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/19
     */

    struct info {
        std::vector<std::string> towels;
        std::vector<std::string> designs;
    };

    std::vector<std::string> parse_towels(std::string_view s) {
        const auto parts = split(s, ", ");
        return parts | std::views::transform([](std::string_view t){ return std::string{t}; }) | std::ranges::to<std::vector>();
    }

    info get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_19_input.txt");
        return {
                parse_towels(lines.front()),
                {lines.begin() + 2, lines.end()}
        };
    }

    bool is_possible(std::string_view design, const std::vector<std::string>& towels) {
        const auto found = std::find(towels.begin(), towels.end(), design);
        if (found != towels.end()) {
            return true;
        }
        else {
            for (const auto& t : towels) {
                if (design.starts_with(t)) {
                    const bool res = is_possible(design.substr(t.size()), towels);
                    if (res) {
                        return true;
                    }
                }
            }
            return false;
        }
    }

    struct memo {
        std::string end;
        int64_t count = 0;

        memo() = default;
        memo(std::string_view s, int64_t c) : end{s}, count{c} {}

        auto operator<=>(const memo& rhs) const { return end <=> rhs.end; }
        bool operator==(const memo& rhs) const { return end == rhs.end; }

        auto operator<=>(std::string_view rhs) const { return end <=> rhs; }
        bool operator==(std::string_view rhs) const { return end == rhs; }
    };

    int64_t num_possible(const std::string_view design, const std::vector<std::string>& towels, std::vector<memo>& cache) {
        auto found = std::lower_bound(cache.begin(), cache.end(), design);
        if (found != cache.end() && *found == design) {
            return found->count;
        }
        else {
            int64_t total = 0;
            for (const auto& t : towels) {
                if (design.starts_with(t)) {
                    const auto res = num_possible(design.substr(t.size()), towels, cache);
                    total += res;
                }
            }
            found = std::lower_bound(cache.begin(), cache.end(), design);
            if (found == cache.end() || *found != design) {
                cache.emplace(found, design, total);
            }
            return total;
        }
    }

    int64_t num_possible(std::string_view design, const std::vector<std::string>& towels) {
        std::vector<memo> cache;
        cache.emplace_back("", 1);
        const auto ret = num_possible(design, towels, cache);
        return ret;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num_possible = std::count_if(input.designs.begin(), input.designs.end(),
                                                [&t = input.towels](std::string_view s){ return is_possible(s, t); });
        return std::to_string(num_possible);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num_ways = std::accumulate(input.designs.begin(), input.designs.end(), 0ll,
                                                [&t = input.towels](int64_t tot, std::string_view s){ return tot + num_possible(s, t); });
        return std::to_string(num_ways);
    }

    aoc::registration r{2024, 19, part_1, part_2};

    TEST_SUITE("2024_day19") {
        TEST_CASE("2024_day19:example") {
            const info input {
                    {"r", "wr", "b", "g", "bwu", "rb", "gb", "br"},
                    {
                            "brwrr",
                            "bggr",
                            "gbbr",
                            "rrbgbr",
                            "ubwu",
                            "bwurrg",
                            "brgr",
                            "bbrgwb"
                    }
            };
            const auto num_poss = std::count_if(input.designs.begin(), input.designs.end(),
                                                    [&t = input.towels](std::string_view s){ return is_possible(s, t); });
            CHECK(is_possible(input.designs[0], input.towels));
            CHECK(is_possible(input.designs[1], input.towels));
            CHECK(is_possible(input.designs[2], input.towels));
            CHECK(is_possible(input.designs[3], input.towels));
            CHECK(!is_possible(input.designs[4], input.towels));
            CHECK(is_possible(input.designs[5], input.towels));
            CHECK(is_possible(input.designs[6], input.towels));
            CHECK(!is_possible(input.designs[7], input.towels));
            CHECK_EQ(num_poss, 6);

            const auto num_ways = std::accumulate(input.designs.begin(), input.designs.end(), 0ll,
                                                  [&t = input.towels](int64_t tot, std::string_view s){ return tot + num_possible(s, t); });
            CHECK_EQ(num_possible(input.designs[0], input.towels), 2);
            CHECK_EQ(num_possible(input.designs[1], input.towels), 1);
            CHECK_EQ(num_possible(input.designs[2], input.towels), 4);
            CHECK_EQ(num_possible(input.designs[3], input.towels), 6);
            CHECK_EQ(num_possible(input.designs[4], input.towels), 0);
            CHECK_EQ(num_possible(input.designs[5], input.towels), 1);
            CHECK_EQ(num_possible(input.designs[6], input.towels), 2);
            CHECK_EQ(num_possible(input.designs[7], input.towels), 0);
            CHECK_EQ(num_ways, 16);
        }
    }

} /* namespace <anon> */