//
// Created by Daniel Garcia on 12/25/20.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <array>

namespace {

    constexpr auto PUBLIC_KEY_1 = 10212254ull;
    constexpr auto PUBLIC_KEY_2 = 12577395ull;

    constexpr auto MODULUS = 20201227ull;

    uint64_t encrypt_transform(uint64_t val, const uint64_t subj_num) {
        val *= subj_num;
        val %= MODULUS;
        return val;
    }

    uint64_t encrypt(uint64_t val, const uint64_t loop_size) {
        const auto subj_num = val;
        val = 1;
        for (uint64_t i = 0; i < loop_size; ++i) {
            val = encrypt_transform(val, subj_num);
        }
        return val;
    }

    std::array<uint64_t, 2> find_loop_sizes(const uint64_t pub_key_1, const uint64_t pub_key_2) {
        uint64_t val = 1ull;
        std::array<uint64_t, 2> loop_sizes{};
        for (uint64_t ls = 1ull; loop_sizes[0] == 0 || loop_sizes[1] == 0; ++ls) {
            val = encrypt_transform(val, 7);
            if (val == pub_key_1) {
                loop_sizes[0] = ls;
            }
            if (val == pub_key_2) {
                loop_sizes[1] = ls;
            }
        }
        return loop_sizes;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto loop_sizes = find_loop_sizes(PUBLIC_KEY_1, PUBLIC_KEY_2);
        return std::to_string(encrypt(PUBLIC_KEY_2, loop_sizes[0]));
    }

    aoc::registration r {2020, 25, part_1};

    TEST_SUITE("day25" * doctest::description("Tests for the challenges for day 25.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day25:example" * doctest::description("Testing my solution on the example given in the challenge.")) {
            constexpr auto CARD_PUBLIC_KEY = 5764801ull;
            constexpr auto DOOR_PUBLIC_KEY = 17807724ull;

            const auto loop_sizes = find_loop_sizes(CARD_PUBLIC_KEY, DOOR_PUBLIC_KEY);
            REQUIRE_EQ(loop_sizes[0], 8);
            REQUIRE_EQ(loop_sizes[1], 11);

            CHECK_EQ(encrypt(DOOR_PUBLIC_KEY, loop_sizes[0]), 14897079ull);
            CHECK_EQ(encrypt(CARD_PUBLIC_KEY, loop_sizes[1]), 14897079ull);
        }
    }

} /* namespace */