//
// Created by Dan on 12/11/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    enum class op {
        add,
        multiply,
        square
    };

    using item_type = int64_t;

    struct monkey {
        std::vector<item_type> items;
        op operation;
        int op_num;
        int test_num;
        int true_dest;
        int false_dest;
        int64_t inspect_count = 0;
    };

    item_type parse_int(std::string_view s) { return parse<item_type>(s); }

    monkey parse_monkey(const std::vector<std::string>::const_iterator& line) {
        std::string_view items {*(line + 1)};
        std::string_view op_line {*(line + 2)};
        std::string_view d_line {*(line + 3)};
        std::string_view t_line {*(line + 4)};
        std::string_view f_line {*(line + 5)};

        items.remove_prefix(16);
        op_line.remove_prefix(21);
        d_line.remove_prefix(19);
        t_line.remove_prefix(25);
        f_line.remove_prefix(26);

        const auto item_strs = split(items, ", ");
        auto item_list = item_strs | std::views::transform(parse_int) | to<std::vector<item_type>>();

        op the_op{};
        int op_num = 0;
        if (op_line.front() == '+') {
            the_op = op::add;
            op_line.remove_prefix(2);
            op_num = parse<int>(op_line);
        }
        else {
            op_line.remove_prefix(2);
            if (op_line.front() == 'o') {
                the_op = op::square;
            }
            else {
                the_op = op::multiply;
                op_num = parse<int>(op_line);
            }
        }

        return {std::move(item_list), the_op, op_num, parse<int>(d_line), parse<int>(t_line), parse<int>(f_line)};
    }

    std::vector<monkey> get_input(const std::vector<std::string>& lines) {
        auto trimmed = trim_lines(lines);
        std::vector<monkey> retval;
        for (auto it = trimmed.begin(); it != trimmed.end(); it += 6) {
            if (it->empty()) {
                ++it;
            }
            retval.push_back(parse_monkey(it));
        }
        return retval;
    }

    item_type do_op(const op the_op, const int op_num, const item_type val) {
        if (the_op == op::add) {
            return val + op_num;
        }
        else if (the_op == op::multiply) {
            return val * op_num;
        }
        else {
            return val * val;
        }
    }

    void do_turn(std::vector<monkey>& monkeys, const int idx, const item_type worry_divisor, const item_type lcm) {
        std::vector<item_type> tmp;
        tmp.swap(monkeys[idx].items);
        for (item_type item : tmp) {
            item = do_op(monkeys[idx].operation, monkeys[idx].op_num, item);
            item /= worry_divisor;
            if (lcm) {
                //We need to cap the item's worry level without changing the outcome of any divisor tests.
                //So we modulo the worry level with the product of the test numbers (which are all mutually
                //prime).
                item %= lcm;
            }
            if (item % monkeys[idx].test_num == 0) {
                monkeys[monkeys[idx].true_dest].items.push_back(item);
            }
            else {
                monkeys[monkeys[idx].false_dest].items.push_back(item);
            }
            ++monkeys[idx].inspect_count;
        }
    }

    void do_round(std::vector<monkey>& monkeys, const item_type worry_divisor, const item_type lcm) {
        for (int i = 0; i < monkeys.size(); ++i) {
            do_turn(monkeys, i, worry_divisor, lcm);
        }
    }

    /*
    --- Day 11: Monkey in the Middle ---
    As you finally start making your way upriver, you realize your pack is much lighter than you remember. Just then, one of the items from your pack goes flying overhead. Monkeys are playing Keep Away with your missing things!

    To get your stuff back, you need to be able to predict where the monkeys will throw your items. After some careful observation, you realize the monkeys operate based on how worried you are about each item.

    You take some notes (your puzzle input) on the items each monkey currently has, how worried you are about those items, and how the monkey makes decisions based on your worry level. For example:

    Monkey 0:
      Starting items: 79, 98
      Operation: new = old * 19
      Test: divisible by 23
        If true: throw to monkey 2
        If false: throw to monkey 3

    Monkey 1:
      Starting items: 54, 65, 75, 74
      Operation: new = old + 6
      Test: divisible by 19
        If true: throw to monkey 2
        If false: throw to monkey 0

    Monkey 2:
      Starting items: 79, 60, 97
      Operation: new = old * old
      Test: divisible by 13
        If true: throw to monkey 1
        If false: throw to monkey 3

    Monkey 3:
      Starting items: 74
      Operation: new = old + 3
      Test: divisible by 17
        If true: throw to monkey 0
        If false: throw to monkey 1
    Each monkey has several attributes:

    Starting items lists your worry level for each item the monkey is currently holding in the order they will be inspected.
    Operation shows how your worry level changes as that monkey inspects an item. (An operation like new = old * 5 means that your worry level after the monkey inspected the item is five times whatever your worry level was before inspection.)
    Test shows how the monkey uses your worry level to decide where to throw an item next.
    If true shows what happens with an item if the Test was true.
    If false shows what happens with an item if the Test was false.
    After each monkey inspects an item but before it tests your worry level, your relief that the monkey's inspection didn't damage the item causes your worry level to be divided by three and rounded down to the nearest integer.

    The monkeys take turns inspecting and throwing items. On a single monkey's turn, it inspects and throws all of the items it is holding one at a time and in the order listed. Monkey 0 goes first, then monkey 1, and so on until each monkey has had one turn. The process of each monkey taking a single turn is called a round.

    When a monkey throws an item to another monkey, the item goes on the end of the recipient monkey's list. A monkey that starts a round with no items could end up inspecting and throwing many items by the time its turn comes around. If a monkey is holding no items at the start of its turn, its turn ends.

    In the above example, the first round proceeds as follows:

    Monkey 0:
      Monkey inspects an item with a worry level of 79.
        Worry level is multiplied by 19 to 1501.
        Monkey gets bored with item. Worry level is divided by 3 to 500.
        Current worry level is not divisible by 23.
        Item with worry level 500 is thrown to monkey 3.
      Monkey inspects an item with a worry level of 98.
        Worry level is multiplied by 19 to 1862.
        Monkey gets bored with item. Worry level is divided by 3 to 620.
        Current worry level is not divisible by 23.
        Item with worry level 620 is thrown to monkey 3.
    Monkey 1:
      Monkey inspects an item with a worry level of 54.
        Worry level increases by 6 to 60.
        Monkey gets bored with item. Worry level is divided by 3 to 20.
        Current worry level is not divisible by 19.
        Item with worry level 20 is thrown to monkey 0.
      Monkey inspects an item with a worry level of 65.
        Worry level increases by 6 to 71.
        Monkey gets bored with item. Worry level is divided by 3 to 23.
        Current worry level is not divisible by 19.
        Item with worry level 23 is thrown to monkey 0.
      Monkey inspects an item with a worry level of 75.
        Worry level increases by 6 to 81.
        Monkey gets bored with item. Worry level is divided by 3 to 27.
        Current worry level is not divisible by 19.
        Item with worry level 27 is thrown to monkey 0.
      Monkey inspects an item with a worry level of 74.
        Worry level increases by 6 to 80.
        Monkey gets bored with item. Worry level is divided by 3 to 26.
        Current worry level is not divisible by 19.
        Item with worry level 26 is thrown to monkey 0.
    Monkey 2:
      Monkey inspects an item with a worry level of 79.
        Worry level is multiplied by itself to 6241.
        Monkey gets bored with item. Worry level is divided by 3 to 2080.
        Current worry level is divisible by 13.
        Item with worry level 2080 is thrown to monkey 1.
      Monkey inspects an item with a worry level of 60.
        Worry level is multiplied by itself to 3600.
        Monkey gets bored with item. Worry level is divided by 3 to 1200.
        Current worry level is not divisible by 13.
        Item with worry level 1200 is thrown to monkey 3.
      Monkey inspects an item with a worry level of 97.
        Worry level is multiplied by itself to 9409.
        Monkey gets bored with item. Worry level is divided by 3 to 3136.
        Current worry level is not divisible by 13.
        Item with worry level 3136 is thrown to monkey 3.
    Monkey 3:
      Monkey inspects an item with a worry level of 74.
        Worry level increases by 3 to 77.
        Monkey gets bored with item. Worry level is divided by 3 to 25.
        Current worry level is not divisible by 17.
        Item with worry level 25 is thrown to monkey 1.
      Monkey inspects an item with a worry level of 500.
        Worry level increases by 3 to 503.
        Monkey gets bored with item. Worry level is divided by 3 to 167.
        Current worry level is not divisible by 17.
        Item with worry level 167 is thrown to monkey 1.
      Monkey inspects an item with a worry level of 620.
        Worry level increases by 3 to 623.
        Monkey gets bored with item. Worry level is divided by 3 to 207.
        Current worry level is not divisible by 17.
        Item with worry level 207 is thrown to monkey 1.
      Monkey inspects an item with a worry level of 1200.
        Worry level increases by 3 to 1203.
        Monkey gets bored with item. Worry level is divided by 3 to 401.
        Current worry level is not divisible by 17.
        Item with worry level 401 is thrown to monkey 1.
      Monkey inspects an item with a worry level of 3136.
        Worry level increases by 3 to 3139.
        Monkey gets bored with item. Worry level is divided by 3 to 1046.
        Current worry level is not divisible by 17.
        Item with worry level 1046 is thrown to monkey 1.
    After round 1, the monkeys are holding items with these worry levels:

    Monkey 0: 20, 23, 27, 26
    Monkey 1: 2080, 25, 167, 207, 401, 1046
    Monkey 2:
    Monkey 3:
    Monkeys 2 and 3 aren't holding any items at the end of the round; they both inspected items during the round and threw them all before the round ended.

    This process continues for a few more rounds:

    After round 2, the monkeys are holding items with these worry levels:
    Monkey 0: 695, 10, 71, 135, 350
    Monkey 1: 43, 49, 58, 55, 362
    Monkey 2:
    Monkey 3:

    After round 3, the monkeys are holding items with these worry levels:
    Monkey 0: 16, 18, 21, 20, 122
    Monkey 1: 1468, 22, 150, 286, 739
    Monkey 2:
    Monkey 3:

    After round 4, the monkeys are holding items with these worry levels:
    Monkey 0: 491, 9, 52, 97, 248, 34
    Monkey 1: 39, 45, 43, 258
    Monkey 2:
    Monkey 3:

    After round 5, the monkeys are holding items with these worry levels:
    Monkey 0: 15, 17, 16, 88, 1037
    Monkey 1: 20, 110, 205, 524, 72
    Monkey 2:
    Monkey 3:

    After round 6, the monkeys are holding items with these worry levels:
    Monkey 0: 8, 70, 176, 26, 34
    Monkey 1: 481, 32, 36, 186, 2190
    Monkey 2:
    Monkey 3:

    After round 7, the monkeys are holding items with these worry levels:
    Monkey 0: 162, 12, 14, 64, 732, 17
    Monkey 1: 148, 372, 55, 72
    Monkey 2:
    Monkey 3:

    After round 8, the monkeys are holding items with these worry levels:
    Monkey 0: 51, 126, 20, 26, 136
    Monkey 1: 343, 26, 30, 1546, 36
    Monkey 2:
    Monkey 3:

    After round 9, the monkeys are holding items with these worry levels:
    Monkey 0: 116, 10, 12, 517, 14
    Monkey 1: 108, 267, 43, 55, 288
    Monkey 2:
    Monkey 3:

    After round 10, the monkeys are holding items with these worry levels:
    Monkey 0: 91, 16, 20, 98
    Monkey 1: 481, 245, 22, 26, 1092, 30
    Monkey 2:
    Monkey 3:

    ...

    After round 15, the monkeys are holding items with these worry levels:
    Monkey 0: 83, 44, 8, 184, 9, 20, 26, 102
    Monkey 1: 110, 36
    Monkey 2:
    Monkey 3:

    ...

    After round 20, the monkeys are holding items with these worry levels:
    Monkey 0: 10, 12, 14, 26, 34
    Monkey 1: 245, 93, 53, 199, 115
    Monkey 2:
    Monkey 3:
    Chasing all of the monkeys at once is impossible; you're going to have to focus on the two most active monkeys if you want any hope of getting your stuff back. Count the total number of times each monkey inspects items over 20 rounds:

    Monkey 0 inspected items 101 times.
    Monkey 1 inspected items 95 times.
    Monkey 2 inspected items 7 times.
    Monkey 3 inspected items 105 times.
    In this example, the two most active monkeys inspected items 101 and 105 times. The level of monkey business in this situation can be found by multiplying these together: 10605.

    Figure out which monkeys to chase by counting how many items they inspect over 20 rounds. What is the level of monkey business after 20 rounds of stuff-slinging simian shenanigans?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        for (int i = 0; i < 20; ++i) {
            do_round(input, 3, 0);
        }
        std::sort(input.begin(), input.end(), [](const monkey& a, const monkey& b){ return a.inspect_count > b.inspect_count; });
        return std::to_string(input[0].inspect_count * input[1].inspect_count);
    }

    /*
    --- Part Two ---
    You're worried you might not ever get your items back. So worried, in fact, that your relief that a monkey's inspection didn't damage an item no longer causes your worry level to be divided by three.

    Unfortunately, that relief was all that was keeping your worry levels from reaching ridiculous levels. You'll need to find another way to keep your worry levels manageable.

    At this rate, you might be putting up with these monkeys for a very long time - possibly 10000 rounds!

    With these new rules, you can still figure out the monkey business after 10000 rounds. Using the same example above:

    == After round 1 ==
    Monkey 0 inspected items 2 times.
    Monkey 1 inspected items 4 times.
    Monkey 2 inspected items 3 times.
    Monkey 3 inspected items 6 times.

    == After round 20 ==
    Monkey 0 inspected items 99 times.
    Monkey 1 inspected items 97 times.
    Monkey 2 inspected items 8 times.
    Monkey 3 inspected items 103 times.

    == After round 1000 ==
    Monkey 0 inspected items 5204 times.
    Monkey 1 inspected items 4792 times.
    Monkey 2 inspected items 199 times.
    Monkey 3 inspected items 5192 times.

    == After round 2000 ==
    Monkey 0 inspected items 10419 times.
    Monkey 1 inspected items 9577 times.
    Monkey 2 inspected items 392 times.
    Monkey 3 inspected items 10391 times.

    == After round 3000 ==
    Monkey 0 inspected items 15638 times.
    Monkey 1 inspected items 14358 times.
    Monkey 2 inspected items 587 times.
    Monkey 3 inspected items 15593 times.

    == After round 4000 ==
    Monkey 0 inspected items 20858 times.
    Monkey 1 inspected items 19138 times.
    Monkey 2 inspected items 780 times.
    Monkey 3 inspected items 20797 times.

    == After round 5000 ==
    Monkey 0 inspected items 26075 times.
    Monkey 1 inspected items 23921 times.
    Monkey 2 inspected items 974 times.
    Monkey 3 inspected items 26000 times.

    == After round 6000 ==
    Monkey 0 inspected items 31294 times.
    Monkey 1 inspected items 28702 times.
    Monkey 2 inspected items 1165 times.
    Monkey 3 inspected items 31204 times.

    == After round 7000 ==
    Monkey 0 inspected items 36508 times.
    Monkey 1 inspected items 33488 times.
    Monkey 2 inspected items 1360 times.
    Monkey 3 inspected items 36400 times.

    == After round 8000 ==
    Monkey 0 inspected items 41728 times.
    Monkey 1 inspected items 38268 times.
    Monkey 2 inspected items 1553 times.
    Monkey 3 inspected items 41606 times.

    == After round 9000 ==
    Monkey 0 inspected items 46945 times.
    Monkey 1 inspected items 43051 times.
    Monkey 2 inspected items 1746 times.
    Monkey 3 inspected items 46807 times.

    == After round 10000 ==
    Monkey 0 inspected items 52166 times.
    Monkey 1 inspected items 47830 times.
    Monkey 2 inspected items 1938 times.
    Monkey 3 inspected items 52013 times.
    After 10000 rounds, the two most active monkeys inspected items 52166 and 52013 times. Multiplying these together, the level of monkey business in this situation is now 2713310158.

    Worry levels are no longer divided by three after each item is inspected; you'll need to find another way to keep your worry levels manageable. Starting again from the initial state in your puzzle input, what is the level of monkey business after 10000 rounds?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        const item_type lcm = std::accumulate(input.begin(), input.end(), 1, [](int v, const monkey& m){ return v * m.test_num; });
        for (int i = 0; i < 10000; ++i) {
            do_round(input, 1, lcm);
        }
        std::sort(input.begin(), input.end(), [](const monkey& a, const monkey& b){ return a.inspect_count > b.inspect_count; });
        return std::to_string(input[0].inspect_count * input[1].inspect_count);
    }

    aoc::registration r{2022, 11, part_1, part_2};

    TEST_SUITE("2022_day11") {
        TEST_CASE("2022_day11:example_p1") {
            std::vector<monkey> input {
                    {{79, 98}, op::multiply, 19, 23, 2, 3},
                    {{54, 65, 75, 74}, op::add, 6, 19, 2, 0},
                    {{79, 60, 97}, op::square, 0, 13, 1, 3},
                    {{74}, op::add, 3, 17, 0, 1}
            };
            for (int i = 0; i < 20; ++i) {
                do_round(input, 3, 0);
            }
            CHECK_EQ(input[0].inspect_count, 101);
            CHECK_EQ(input[1].inspect_count, 95);
            CHECK_EQ(input[2].inspect_count, 7);
            CHECK_EQ(input[3].inspect_count, 105);
        }
        TEST_CASE("2022_day11:example_p2") {
            std::vector<monkey> input {
                    {{79, 98}, op::multiply, 19, 23, 2, 3},
                    {{54, 65, 75, 74}, op::add, 6, 19, 2, 0},
                    {{79, 60, 97}, op::square, 0, 13, 1, 3},
                    {{74}, op::add, 3, 17, 0, 1}
            };
            const int lcm = std::accumulate(input.begin(), input.end(), 1, [](int v, const monkey& m){ return v * m.test_num; });
            for (int i = 0; i < 20; ++i) {
                do_round(input, 1, lcm);
            }
            CHECK_EQ(input[0].inspect_count, 99);
            CHECK_EQ(input[1].inspect_count, 97);
            CHECK_EQ(input[2].inspect_count, 8);
            CHECK_EQ(input[3].inspect_count, 103);

            for (int i = 0; i < 980; ++i) {
                do_round(input, 1, lcm);
            }
            CHECK_EQ(input[0].inspect_count, 5204);
            CHECK_EQ(input[1].inspect_count, 4792);
            CHECK_EQ(input[2].inspect_count, 199);
            CHECK_EQ(input[3].inspect_count, 5192);

            for (int i = 0; i < 1000; ++i) {
                do_round(input, 1, lcm);
            }
            CHECK_EQ(input[0].inspect_count, 10419);
            CHECK_EQ(input[1].inspect_count, 9577);
            CHECK_EQ(input[2].inspect_count, 392);
            CHECK_EQ(input[3].inspect_count, 10391);

            for (int i = 0; i < 8000; ++i) {
                do_round(input, 1, lcm);
            }
            CHECK_EQ(input[0].inspect_count, 52166);
            CHECK_EQ(input[1].inspect_count, 47830);
            CHECK_EQ(input[2].inspect_count, 1938);
            CHECK_EQ(input[3].inspect_count, 52013);
        }
    }

}