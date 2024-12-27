//
// Created by Dan on 12/3/2021.
//

#include "registration.h"

#include <vector>
#include <iostream>
#include <string_view>
#include <array>
#include <ranges>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int INPUT_LEN = 12;

    using iter = std::vector<std::string>::const_iterator;

    template <typename P>
    void find_bit_criteria(const std::vector<std::string>& data, std::string& prefix, const char tie, P use_ones) {
        auto one_count = 0;
        auto range = data |
                     std::views::filter([&prefix, &one_count](const std::string& s){
                         bool retval = s.starts_with(prefix);
                         if (retval && prefix.size() < s.size()) {
                             one_count += static_cast<int>(s[prefix.size()] - '0');
                         }
                         return retval;
                     });
        const auto num = std::ranges::distance(range);
        if (num == 1) {
            prefix = range.front();
            return;
        }

        if (use_ones(one_count, num)) {
            prefix += '1';
        }
        else if (num % 2 == 0 && one_count == num / 2) {
            prefix += tie;
        }
        else {
            prefix += '0';
        }

        find_bit_criteria(data, prefix, tie, use_ones);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        std::array<int, INPUT_LEN> counts{};
        for (const auto& s : lines) {
            for (int i = 0; i < INPUT_LEN; ++i) {
                counts[i] += s[i] - '0';
            }
        }
        const int num = static_cast<int>(lines.size()) / 2;
        uint32_t gamma = 0, epsilon = 0, mask = 0;
        for (auto c : counts) {
            gamma <<= 1u;
            mask <<= 1u;
            gamma |= (c > num ? 1u : 0u);
            mask |= 1u;
        }
        epsilon = (~gamma) & mask;
        return std::to_string(gamma * epsilon);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        std::string ox_gen_rating_str, co2_scrub_rating_str;
        find_bit_criteria(lines, ox_gen_rating_str, '1', [](int64_t num_ones, int64_t total){ return num_ones > total / 2; });
        find_bit_criteria(lines, co2_scrub_rating_str, '0', [](int64_t num_ones, int64_t total){ return num_ones < total / 2 || (total % 2 == 1 && num_ones == total / 2); });
        int ox_gen_rating = 0, co2_scrub_rating = 0;
        auto res1 = std::from_chars(ox_gen_rating_str.data(), ox_gen_rating_str.data() + ox_gen_rating_str.size(), ox_gen_rating, 2);
        auto res2 = std::from_chars(co2_scrub_rating_str.data(), co2_scrub_rating_str.data() + co2_scrub_rating_str.size(), co2_scrub_rating, 2);
        const std::errc no_err{};
        if (res1.ec != no_err) {
            throw std::system_error{std::make_error_code(res1.ec)};
        }
        else if (res2.ec != no_err) {
            throw std::system_error{std::make_error_code(res2.ec)};
        }
        return std::to_string(ox_gen_rating * co2_scrub_rating);
    }

    aoc::registration r {2021, 3, part_1, part_2};

}