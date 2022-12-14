//
// Created by Dan on 12/13/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct value;

    using value_ptr = std::unique_ptr<value>;
    using list = std::vector<value_ptr>;

    struct value {
        std::variant<int, list> val;
    };

    value parse_value(std::string_view& s);

    list parse_list_consume(std::string_view& s) {
        int depth = 0, end_pos = 0;
        for (; end_pos < s.size(); ++end_pos) {
            if (s[end_pos] == '[') {
                ++depth;
            }
            else if (s[end_pos] == ']') {
                --depth;
            }
            if (depth == 0) {
                break;
            }
        }
        std::string_view contents {s.begin() + 1, s.begin() + end_pos};
        s.remove_prefix(end_pos + 1);
        list retval;
        while (!contents.empty()) {
            auto tmp = parse_value(contents);
            auto v = std::make_unique<value>(std::move(tmp));
            retval.push_back(std::move(v));
            if (!contents.empty() && contents.front() == ',') {
                contents.remove_prefix(1);
            }
        }
        return retval;
    }

    value parse_value(std::string_view& s) {
        if (s.front() == '[') {
            return {parse_list_consume(s)};
        }
        else {
            int val = 0;
            const auto res = std::from_chars(s.data(), s.data() + s.size(), val);
            s.remove_prefix(std::distance(s.data(), res.ptr));
            return {val};
        }
    }

    list parse_list(std::string_view s) {
        return parse_list_consume(s);
    }

    std::vector<list> get_input(const fs::path &input_dir) {
        auto lines = read_file_lines(input_dir / "2022" / "day_13_input.txt");
        lines.erase(std::remove(lines.begin(), lines.end(), std::string{}), lines.end());
        return lines | std::views::transform(parse_list) | to<std::vector<list>>();
    }

    bool is_less(const list& a, const list& b);

    inline bool is_less_value(const value_ptr& a, const value_ptr& b) {
        bool is_a_list = std::holds_alternative<list>(a->val);
        bool is_b_list = std::holds_alternative<list>(b->val);
        if (!is_a_list && !is_b_list) {
            return std::get<int>(a->val) < std::get<int>(b->val);
        }
        else {
            list alt_a, alt_b, *comp_a = &alt_a, *comp_b = &alt_b;
            if (is_a_list) {
                comp_a = std::addressof(std::get<list>(a->val));
            }
            else {
                alt_a.emplace_back(std::make_unique<value>(value{std::get<int>(a->val)}));
            }
            if (is_b_list) {
                comp_b = std::addressof(std::get<list>(b->val));
            }
            else {
                alt_b.emplace_back(std::make_unique<value>(value{std::get<int>(b->val)}));
            }
            return is_less(*comp_a, *comp_b);
        }
    }

    inline bool is_less(const list& a, const list& b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), is_less_value);
    }

    /*
    --- Day 13: Distress Signal ---
    You climb the hill and again try contacting the Elves. However, you instead receive a signal you weren't expecting: a distress signal.

    Your handheld device must still not be working properly; the packets from the distress signal got decoded out of order. You'll need to re-order the list of received packets (your puzzle input) to decode the message.

    Your list consists of pairs of packets; pairs are separated by a blank line. You need to identify how many pairs of packets are in the right order.

    For example:

    [1,1,3,1,1]
    [1,1,5,1,1]

    [[1],[2,3,4]]
    [[1],4]

    [9]
    [[8,7,6]]

    [[4,4],4,4]
    [[4,4],4,4,4]

    [7,7,7,7]
    [7,7,7]

    []
    [3]

    [[[]]]
    [[]]

    [1,[2,[3,[4,[5,6,7]]]],8,9]
    [1,[2,[3,[4,[5,6,0]]]],8,9]
    Packet data consists of lists and integers. Each list starts with [, ends with ], and contains zero or more comma-separated values (either integers or other lists). Each packet is always a list and appears on its own line.

    When comparing two values, the first value is called left and the second value is called right. Then:

    If both values are integers, the lower integer should come first. If the left integer is lower than the right integer, the inputs are in the right order. If the left integer is higher than the right integer, the inputs are not in the right order. Otherwise, the inputs are the same integer; continue checking the next part of the input.
    If both values are lists, compare the first value of each list, then the second value, and so on. If the left list runs out of items first, the inputs are in the right order. If the right list runs out of items first, the inputs are not in the right order. If the lists are the same length and no comparison makes a decision about the order, continue checking the next part of the input.
    If exactly one value is an integer, convert the integer to a list which contains that integer as its only value, then retry the comparison. For example, if comparing [0,0,0] and 2, convert the right value to [2] (a list containing 2); the result is then found by instead comparing [0,0,0] and [2].
    Using these rules, you can determine which of the pairs in the example are in the right order:

    == Pair 1 ==
    - Compare [1,1,3,1,1] vs [1,1,5,1,1]
      - Compare 1 vs 1
      - Compare 1 vs 1
      - Compare 3 vs 5
        - Left side is smaller, so inputs are in the right order

    == Pair 2 ==
    - Compare [[1],[2,3,4]] vs [[1],4]
      - Compare [1] vs [1]
        - Compare 1 vs 1
      - Compare [2,3,4] vs 4
        - Mixed types; convert right to [4] and retry comparison
        - Compare [2,3,4] vs [4]
          - Compare 2 vs 4
            - Left side is smaller, so inputs are in the right order

    == Pair 3 ==
    - Compare [9] vs [[8,7,6]]
      - Compare 9 vs [8,7,6]
        - Mixed types; convert left to [9] and retry comparison
        - Compare [9] vs [8,7,6]
          - Compare 9 vs 8
            - Right side is smaller, so inputs are not in the right order

    == Pair 4 ==
    - Compare [[4,4],4,4] vs [[4,4],4,4,4]
      - Compare [4,4] vs [4,4]
        - Compare 4 vs 4
        - Compare 4 vs 4
      - Compare 4 vs 4
      - Compare 4 vs 4
      - Left side ran out of items, so inputs are in the right order

    == Pair 5 ==
    - Compare [7,7,7,7] vs [7,7,7]
      - Compare 7 vs 7
      - Compare 7 vs 7
      - Compare 7 vs 7
      - Right side ran out of items, so inputs are not in the right order

    == Pair 6 ==
    - Compare [] vs [3]
      - Left side ran out of items, so inputs are in the right order

    == Pair 7 ==
    - Compare [[[]]] vs [[]]
      - Compare [[]] vs []
        - Right side ran out of items, so inputs are not in the right order

    == Pair 8 ==
    - Compare [1,[2,[3,[4,[5,6,7]]]],8,9] vs [1,[2,[3,[4,[5,6,0]]]],8,9]
      - Compare 1 vs 1
      - Compare [2,[3,[4,[5,6,7]]]] vs [2,[3,[4,[5,6,0]]]]
        - Compare 2 vs 2
        - Compare [3,[4,[5,6,7]]] vs [3,[4,[5,6,0]]]
          - Compare 3 vs 3
          - Compare [4,[5,6,7]] vs [4,[5,6,0]]
            - Compare 4 vs 4
            - Compare [5,6,7] vs [5,6,0]
              - Compare 5 vs 5
              - Compare 6 vs 6
              - Compare 7 vs 0
                - Right side is smaller, so inputs are not in the right order
    What are the indices of the pairs that are already in the right order? (The first pair has index 1, the second pair has index 2, and so on.) In the above example, the pairs in the right order are 1, 2, 4, and 6; the sum of these indices is 13.

    Determine which pairs of packets are already in the right order. What is the sum of the indices of those pairs?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        std::size_t sum = 0;
        for (std::size_t i = 0; i < input.size(); i += 2) {
            if (is_less(input[i], input[i+1])) {
                sum += i / 2 + 1;
            }
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    Now, you just need to put all of the packets in the right order. Disregard the blank lines in your list of received packets.

    The distress signal protocol also requires that you include two additional divider packets:

    [[2]]
    [[6]]
    Using the same rules as before, organize all packets - the ones in your list of received packets as well as the two divider packets - into the correct order.

    For the example above, the result of putting the packets in the correct order is:

    []
    [[]]
    [[[]]]
    [1,1,3,1,1]
    [1,1,5,1,1]
    [[1],[2,3,4]]
    [1,[2,[3,[4,[5,6,0]]]],8,9]
    [1,[2,[3,[4,[5,6,7]]]],8,9]
    [[1],4]
    [[2]]
    [3]
    [[4,4],4,4]
    [[4,4],4,4,4]
    [[6]]
    [7,7,7]
    [7,7,7,7]
    [[8,7,6]]
    [9]
    Afterward, locate the divider packets. To find the decoder key for this distress signal, you need to determine the indices of the two divider packets and multiply them together. (The first packet is at index 1, the second packet is at index 2, and so on.) In this example, the divider packets are 10th and 14th, and so the decoder key is 140.

    Organize all of the packets into the correct order. What is the decoder key for the distress signal?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        const auto divider1 = parse_list("[[2]]");
        const auto divider2 = parse_list("[[6]]");
        input.push_back(parse_list("[[2]]"));
        input.push_back(parse_list("[[6]]"));
        std::sort(input.begin(), input.end(), is_less);
        const auto found1 = std::lower_bound(input.begin(), input.end(), divider1, is_less);
        const auto found2 = std::lower_bound(input.begin(), input.end(), divider2, is_less);
        const auto index1 = std::distance(input.begin(), found1) + 1;
        const auto index2 = std::distance(input.begin(), found2) + 1;
        return std::to_string(index1 * index2);
    }

    aoc::registration r{2022, 13, part_1, part_2};

//    TEST_SUITE("2022_day13") {
//        TEST_CASE("2022_day13:example") {
//
//        }
//    }

}