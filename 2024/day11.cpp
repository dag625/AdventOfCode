//
// Created by Dan on 12/11/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <compare>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/11
     */

    std::vector<int64_t> parse_line(std::string_view s) {
        const auto items = split(s, ' ');
        return items | std::views::transform([](std::string_view i){ return parse<int64_t>(i); }) | std::ranges::to<std::vector>();
    }

    std::vector<int64_t> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_11_input.txt");
        return parse_line(lines.front());
    }

    std::pair<int, int64_t> check_digits(const int64_t v) {
        int digits = 1;
        int64_t tens = 10, half_tens = 1;
        while ((v / tens) > 0) {
            ++digits;
            tens *= 10;
            if (digits % 2 == 0) {
                half_tens *= 10;
            }
        }
        return {digits, half_tens};
    }

    struct memo {
        int64_t stone = 0;
        int64_t depth = 0;
        int64_t num = 0;

        memo() = default;
        memo(int64_t s, int64_t d) : stone{s}, depth{d} {}
        memo(int64_t s, int64_t d, int64_t n) : stone{s}, depth{d}, num{n} {}

        auto operator<=>(const memo& rhs) const { return std::tie(stone, depth) <=> std::tie(rhs.stone, rhs.depth); }
        bool operator==(const memo& rhs) const { return std::tie(stone, depth) == std::tie(rhs.stone, rhs.depth); }
    };

    void check_and_add_to_cache(int64_t stone, int64_t depth, int64_t num, std::vector<memo>& cache) {
        memo state {stone, depth, num};
        const auto found = std::lower_bound(cache.begin(), cache.end(), state);
        if (found == cache.end() || *found != state) {
            cache.insert(found, state);
        }
    }

    int64_t blink(int64_t stone, int64_t depth, std::vector<memo>& cache) {
        memo state {stone, depth};
        const auto cache_check = std::lower_bound(cache.begin(), cache.end(), state);
        if (cache_check != cache.end() && *cache_check == state) {
            return cache_check->num;
        }

        if (depth == 0) {
            return 1;
        }
        if (stone == 0) {
            const auto ret = blink(1, depth - 1, cache);
            check_and_add_to_cache(stone, depth, ret, cache);
            return ret;
        }
        else if (const auto [num_digits, half_tens] = check_digits(stone); num_digits % 2 == 0) {
            const auto s1 = stone / half_tens;
            const auto s2 = stone % half_tens;
            const auto n1 = blink(s1, depth - 1, cache);
            const auto n2 = blink(s2, depth - 1, cache);
            const auto ret = n1 + n2;
            check_and_add_to_cache(stone, depth, ret, cache);
            return ret;
        }
        else {
            const auto ret = blink(stone * 2024, depth - 1, cache);
            check_and_add_to_cache(stone, depth, ret, cache);
            return ret;
        }
    }

    int64_t blink(const std::vector<int64_t>& stones, int depth) {
        std::vector<memo> cache;
        int64_t sum = 0;
        for (const auto s : stones) {
            sum += blink(s, depth, cache);
        }
        return sum;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto res = blink(input, 25);
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto res = blink(input, 75);
        return std::to_string(res);
    }

    aoc::registration r{2024, 11, part_1, part_2};

    TEST_SUITE("2024_day11") {
        TEST_CASE("2024_day11:example") {
            const std::vector<int64_t> input { 125, 17 };
            CHECK_EQ(blink(input, 1), 3);
            CHECK_EQ(blink(input, 2), 4);
            CHECK_EQ(blink(input, 3), 5);
            CHECK_EQ(blink(input, 4), 9);
            CHECK_EQ(blink(input, 5), 13);
            CHECK_EQ(blink(input, 6), 22);
            CHECK_EQ(blink(input, 25), 55312);
        }
    }

} /* namespace <anon> */