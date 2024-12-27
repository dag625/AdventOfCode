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

    std::vector<uint64_t> get_input(const std::vector<std::string>& lines) {
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

    constexpr int SEQ_LEN = 4;

    using seq = std::array<int, SEQ_LEN>;

    struct seq_info {
        seq s{};
        int num = 0;

        auto operator<=>(const seq& rhs) const { return s <=> rhs; }
        bool operator==(const seq& rhs) const { return s == rhs; }
    };

    std::vector<seq_info> find_sequences(const price_info& info) {
        std::vector<seq_info> retval;
        seq s{};
        for (int i = 0; i < info.second.size() - s.size() + 1; ++i) {
            std::copy(info.second.begin() + i, info.second.begin() + i + static_cast<int>(s.size()), s.begin());
            const auto found = std::lower_bound(retval.begin(), retval.end(), s);
            if (found == retval.end() || *found != s) {
                retval.emplace(found, s, info.first[i + s.size()]);
            }
        }
        return retval;
    }

    int total_bananas(const std::vector<std::vector<seq_info>>& infos, const seq& s) {
        return std::accumulate(infos.begin(), infos.end(), 0,
                               [&s](int total, const std::vector<seq_info>& i)
                {
                    const auto found = std::lower_bound(i.begin(), i.end(), s);
                    return total + (found != i.end() && *found == s ? found->num : 0);
                });
    }

    int find_max_total_bananas(const std::vector<price_info>& info) {
        const auto per_seller = info | std::views::transform(&find_sequences) | std::ranges::to<std::vector>();
        auto all_seqs = per_seller | std::views::join | std::views::transform([](const seq_info& i){ return i.s; }) | std::ranges::to<std::vector>();
        std::sort(all_seqs.begin(), all_seqs.end());
        all_seqs.erase(std::unique(all_seqs.begin(), all_seqs.end()), all_seqs.end());

        int max = 0;
        for (const auto& s : all_seqs) {
            const auto res = total_bananas(per_seller, s);
            if (res > max) {
                max = res;
            }
        }
        return max;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto secret2000 = input | std::views::transform([](uint64_t v){ return update(v, 2000); });
        const auto sum = std::accumulate(secret2000.begin(), secret2000.end(), 0ull);
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto prices = input | std::views::transform([](uint64_t v){ return get_deltas(v, 2000); }) | std::ranges::to<std::vector>();
        const auto result = find_max_total_bananas(prices);
        return std::to_string(result);
    }

    aoc::registration r{2024, 22, part_1, part_2};

    TEST_SUITE("2024_day22") {
        TEST_CASE("2024_day22:example") {
            const std::vector<uint64_t> input {
                    123,
                    1,
                    2,
                    3,
                    2024,
            };
            const auto prices = input | std::views::transform([](uint64_t v){ return get_deltas(v, 2000); }) | std::ranges::to<std::vector>();
            const auto per_seller = prices | std::views::transform(&find_sequences) | std::ranges::to<std::vector>();

        }
    }

} /* namespace <anon> */