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

namespace fs = std::filesystem;

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

    std::vector<hand> get_input(const fs::path &input_dir, bool have_jokers) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_7_input.txt");
        return lines | std::views::transform([have_jokers](std::string_view s){ return parse_hand(s, have_jokers); }) | to<std::vector<hand>>();
    }

    /*
    --- Day 7: Camel Cards ---
    Your all-expenses-paid trip turns out to be a one-way, five-minute ride in an airship. (At least it's a cool airship!) It drops you off at the edge of a vast desert and descends back to Island Island.

    "Did you bring the parts?"

    You turn around to see an Elf completely covered in white clothing, wearing goggles, and riding a large camel.

    "Did you bring the parts?" she asks again, louder this time. You aren't sure what parts she's looking for; you're here to figure out why the sand stopped.

    "The parts! For the sand, yes! Come with me; I will show you." She beckons you onto the camel.

    After riding a bit across the sands of Desert Island, you can see what look like very large rocks covering half of the horizon. The Elf explains that the rocks are all along the part of Desert Island that is directly above Island Island, making it hard to even get there. Normally, they use big machines to move the rocks and filter the sand, but the machines have broken down because Desert Island recently stopped receiving the parts they need to fix the machines.

    You've already assumed it'll be your job to figure out why the parts stopped when she asks if you can help. You agree automatically.

    Because the journey will take a few days, she offers to teach you the game of Camel Cards. Camel Cards is sort of similar to poker except it's designed to be easier to play while riding a camel.

    In Camel Cards, you get a list of hands, and your goal is to order them based on the strength of each hand. A hand consists of five cards labeled one of A, K, Q, J, T, 9, 8, 7, 6, 5, 4, 3, or 2. The relative strength of each card follows this order, where A is the highest and 2 is the lowest.

    Every hand is exactly one type. From strongest to weakest, they are:

    Five of a kind, where all five cards have the same label: AAAAA
    Four of a kind, where four cards have the same label and one card has a different label: AA8AA
    Full house, where three cards have the same label, and the remaining two cards share a different label: 23332
    Three of a kind, where three cards have the same label, and the remaining two cards are each different from any other card in the hand: TTT98
    Two pair, where two cards share one label, two other cards share a second label, and the remaining card has a third label: 23432
    One pair, where two cards share one label, and the other three cards have a different label from the pair and each other: A23A4
    High card, where all cards' labels are distinct: 23456
    Hands are primarily ordered based on type; for example, every full house is stronger than any three of a kind.

    If two hands have the same type, a second ordering rule takes effect. Start by comparing the first card in each hand. If these cards are different, the hand with the stronger first card is considered stronger. If the first card in each hand have the same label, however, then move on to considering the second card in each hand. If they differ, the hand with the higher second card wins; otherwise, continue with the third card in each hand, then the fourth, then the fifth.

    So, 33332 and 2AAAA are both four of a kind hands, but 33332 is stronger because its first card is stronger. Similarly, 77888 and 77788 are both a full house, but 77888 is stronger because its third card is stronger (and both hands have the same first and second card).

    To play Camel Cards, you are given a list of hands and their corresponding bid (your puzzle input). For example:

    32T3K 765
    T55J5 684
    KK677 28
    KTJJT 220
    QQQJA 483
    This example shows five hands; each hand is followed by its bid amount. Each hand wins an amount equal to its bid multiplied by its rank, where the weakest hand gets rank 1, the second-weakest hand gets rank 2, and so on up to the strongest hand. Because there are five hands in this example, the strongest hand will have rank 5 and its bid will be multiplied by 5.

    So, the first step is to put the hands in order of strength:

    32T3K is the only one pair and the other hands are all a stronger type, so it gets rank 1.
    KK677 and KTJJT are both two pair. Their first cards both have the same label, but the second card of KK677 is stronger (K vs T), so KTJJT gets rank 2 and KK677 gets rank 3.
    T55J5 and QQQJA are both three of a kind. QQQJA has a stronger first card, so it gets rank 5 and T55J5 gets rank 4.
    Now, you can determine the total winnings of this set of hands by adding up the result of multiplying each hand's bid with its rank (765 * 1 + 220 * 2 + 28 * 3 + 684 * 4 + 483 * 5). So the total winnings in this example are 6440.

    Find the rank of every hand in your set. What are the total winnings?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir, false);
        std::sort(input.begin(), input.end());
        int64_t sum = 0;
        for (const auto [idx, hand] : std::views::enumerate(input)) {
            sum += (idx + 1) * hand.bid;
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    To make things a little more interesting, the Elf introduces one additional rule. Now, J cards are jokers - wildcards that can act like whatever card would make the hand the strongest type possible.

    To balance this, J cards are now the weakest individual cards, weaker even than 2. The other cards stay in the same order: A, K, Q, T, 9, 8, 7, 6, 5, 4, 3, 2, J.

    J cards can pretend to be whatever card is best for the purpose of determining hand type; for example, QJJQ2 is now considered four of a kind. However, for the purpose of breaking ties between two hands of the same type, J is always treated as J, not the card it's pretending to be: JKKK2 is weaker than QQQQ2 because J is weaker than Q.

    Now, the above example goes very differently:

    32T3K 765
    T55J5 684
    KK677 28
    KTJJT 220
    QQQJA 483
    32T3K is still the only one pair; it doesn't contain any jokers, so its strength doesn't increase.
    KK677 is now the only two pair, making it the second-weakest hand.
    T55J5, KTJJT, and QQQJA are now all four of a kind! T55J5 gets rank 3, QQQJA gets rank 4, and KTJJT gets rank 5.
    With the new joker rule, the total winnings in this example are 5905.

    Using the new joker rule, find the rank of every hand in your set. What are the new total winnings?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir, true);
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