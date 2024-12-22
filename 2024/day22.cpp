//
// Created by Dan on 12/22/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/22
     */

    std::vector<uint64_t> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_22_input.txt");
        return lines | std::views::transform(&parseu64) | std::ranges::to<std::vector>();
    }

    uint64_t mix(uint64_t val, uint64_t secret) {
        return val ^ secret;
    }

    uint64_t prune(uint64_t secret) {
        return secret & 0x0ffffffull;
    }

    uint64_t update(uint64_t secret) {
        secret = prune(mix(secret << 6,  secret));
        secret = prune(mix(secret >> 5,  secret));
        secret = prune(mix(secret << 11, secret));
        return secret;
    }

    uint64_t update(uint64_t secret, const int n) {
        for (int i = 0; i < n; ++i) {
            secret = update(secret);
        }
        return secret;
    }

    using price_info = std::pair<std::vector<int>, std::vector<int>>;

    price_info get_deltas(uint64_t secret, const int num) {
        std::vector<int> prices, deltas;
        prices.reserve(num + 1);
        deltas.reserve(num);
        prices.push_back(static_cast<int>(secret) % 10);
        for (int i = 0; i < num; ++i) {
            const auto next = update(secret);
            prices.push_back(static_cast<int>(next) % 10);
            deltas.push_back(prices[i + 1] - prices[i]);
            secret = next;
        }
        return {std::move(prices), std::move(deltas)};
    }

    using seq = std::array<int, 4>;

    constexpr seq init_seq() {
        return {-9, -9, -9, -9};
    }

    constexpr void incr_seq(seq& s, int idx) {
        ++s[idx];
        if (idx != 0 && s[idx] == 10) {
            s[idx] = -9;
            incr_seq(s, idx - 1);
        }
    }

    constexpr void incr_seq(seq& s) {
        incr_seq(s, 3);
    }

    constexpr bool is_max(seq& s) { return s[0] >= 10; }

    int num_bananas(const price_info& info, const seq& s) {
        for (int i = 0; i < info.second.size() - s.size() + 1; ++i) {
            if (std::equal(info.second.begin() + i, info.second.begin() + i + static_cast<int>(s.size()), s.begin(), s.end())) {
                return info.first[i + s.size()];
            }
        }
        return 0;
    }

    int total_bananas(const std::vector<price_info>& info, const seq& s) {
        return std::accumulate(info.begin(), info.end(), 0, [&s](int total, const price_info& i){ return total + num_bananas(i, s); });
    }

    int find_max_total_bananas(const std::vector<price_info>& info) {
        int max = 0;
        for (auto s = init_seq(); !is_max(s); incr_seq(s)) {
            const auto res = total_bananas(info, s);
            if (res > max) {
                max = res;
            }
        }
        return max;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto secret2000 = input | std::views::transform([](uint64_t v){ return update(v, 2000); });
        const auto sum = std::accumulate(secret2000.begin(), secret2000.end(), 0ull);
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto prices = input | std::views::transform([](uint64_t v){ return get_deltas(v, 2000); }) | std::ranges::to<std::vector>();
        const auto result = find_max_total_bananas(prices);
        return std::to_string(result);
    }

    aoc::registration r{2024, 22, part_1, part_2};

//    TEST_SUITE("2024_day22") {
//        TEST_CASE("2024_day22:example") {
//
//        }
//    }

} /* namespace <anon> */