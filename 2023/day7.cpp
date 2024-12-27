//
// Created by Dan on 12/6/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    enum class type : int {
        five_of_a_kind = 7,
        four_of_a_kind = 6,
        full_house = 5,
        three_of_a_kind = 4,
        two_pair = 3,
        one_pair = 2,
        high = 1
    };

    constexpr int NUM_CARDS = 14;

    int to_value(char c, bool is_joker) {
        if (c >= '2' && c <= '9') {
            return static_cast<int>(c - '1');
        }
        else if (c == 'T') {
            return 9;
        }
        else if (c == 'J') {
            return is_joker ? 0 : 10;
        }
        else if (c == 'Q') {
            return 11;
        }
        else if (c == 'K') {
            return 12;
        }
        else {
            return 13;
        }
    }

    type get_type(std::string_view cs, bool have_jokers) {
        std::array<int, NUM_CARDS> vals{};
        for (const char c : cs) {
            ++vals[to_value(c, have_jokers)];
        }
        if (have_jokers) {
            auto max = std::max_element(vals.begin() + 1, vals.end());
            *max += vals[0];
            vals[0] = 0;
        }
        std::sort(vals.begin(), vals.end(), std::greater<>{});
        if (vals.front() == 5) {
            return type::five_of_a_kind;
        }
        else if (vals.front() == 4) {
            return type::four_of_a_kind;
        }
        else if (vals[0] == 3 && vals[1] == 2) {
            return type::full_house;
        }
        else if (vals.front() == 3) {
            return type::three_of_a_kind;
        }
        else if (vals[0] == 2 && vals[1] == 2) {
            return type::two_pair;
        }
        else if (vals.front() == 2) {
            return type::one_pair;
        }
        else {
            return type::high;
        }
    }

    struct hand {
        std::string cards;
        int bid;
        type hand_type;
        bool have_jokers;

        hand(std::string_view c, int b, bool j) : cards{c}, bid{b}, hand_type{get_type(c, j)}, have_jokers{j} {}
        bool operator<(const hand& rhs) const noexcept {
            return std::make_tuple(hand_type, to_value(cards[0], have_jokers), to_value(cards[1], have_jokers), to_value(cards[2], have_jokers), to_value(cards[3], have_jokers), to_value(cards[4], have_jokers)) <
                    std::make_tuple(rhs.hand_type, to_value(rhs.cards[0], have_jokers), to_value(rhs.cards[1], have_jokers), to_value(rhs.cards[2], have_jokers), to_value(rhs.cards[3], have_jokers), to_value(rhs.cards[4], have_jokers));
        }
    };

    hand parse_hand(std::string_view s, bool have_jokers) {
        const auto cards = s.substr(0, 5);
        s.remove_prefix(6);
        return {cards, parse<int>(s), have_jokers};
    }

    std::vector<hand> get_input(const std::vector<std::string>& lines, bool have_jokers) {
        return lines | std::views::transform([have_jokers](std::string_view s){ return parse_hand(s, have_jokers); }) | to<std::vector<hand>>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines, false);
        std::sort(input.begin(), input.end());
        int64_t sum = 0;
        for (const auto [idx, hand] : std::views::enumerate(input)) {
            sum += (idx + 1) * hand.bid;
        }
        return std::to_string(sum);
    }

    /************************* Part 1 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines, true);
        std::sort(input.begin(), input.end());
        int64_t sum = 0;
        for (const auto [idx, hand] : std::views::enumerate(input)) {
            sum += (idx + 1) * hand.bid;
        }
        return std::to_string(sum);
    }

    aoc::registration r{2023, 7, part_1, part_2};

//    TEST_SUITE("2023_day07") {
//        TEST_CASE("2023_day07:example") {
//
//        }
//    }

}