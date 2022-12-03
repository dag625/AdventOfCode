//
// Created by Dan on 12/3/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    std::vector<std::string> get_input(const fs::path &input_dir) {
        return read_file_lines(input_dir / "2022" / "day_3_input.txt");
    }

    std::pair<std::string_view, std::string_view> get_compartments(const std::string& s) {
        const auto midpoint = static_cast<std::string::difference_type>(s.size() / 2);
        return {{s.begin(), s.begin() + midpoint}, {s.begin() + midpoint, s.end()}};
    }

    char in_both(const std::pair<std::string_view, std::string_view>& ruck) {
        std::unordered_set<char> in_first;
        for (char c : ruck.first) {
            in_first.insert(c);
        }
        for (char c : ruck.second) {
            if (in_first.find(c) != in_first.end()) {
                return c;
            }
        }
    }

    int priority(char c) {
        if (c >= 'A' && c <= 'Z') {
            return static_cast<int>(c - 'A') + 27;
        }
        else {
            return static_cast<int>(c - 'a') + 1;
        }
    }

    char find_common(std::vector<std::string>::const_iterator current, const std::vector<std::string>::const_iterator& end) {
        std::unordered_map<char, uint8_t> found;
        uint8_t mask = 0x1u;
        for (; current != end; ++current) {
            for (char c : *current) {
                found[c] |= mask;
                if (found[c] == 0x7u) {
                    return c;
                }
            }
            mask <<= 1;
        }
        return '~';
    }

    /*
    --- Day 3: Rucksack Reorganization ---
    One Elf has the important job of loading all of the rucksacks with supplies for the jungle journey. Unfortunately, that Elf didn't quite follow the packing instructions, and so a few items now need to be rearranged.

    Each rucksack has two large compartments. All items of a given type are meant to go into exactly one of the two compartments. The Elf that did the packing failed to follow this rule for exactly one item type per rucksack.

    The Elves have made a list of all of the items currently in each rucksack (your puzzle input), but they need your help finding the errors. Every item type is identified by a single lowercase or uppercase letter (that is, a and A refer to different types of items).

    The list of items for each rucksack is given as characters all on a single line. A given rucksack always has the same number of items in each of its two compartments, so the first half of the characters represent items in the first compartment, while the second half of the characters represent items in the second compartment.

    For example, suppose you have the following list of contents from six rucksacks:

    vJrwpWtwJgWrhcsFMMfFFhFp
    jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL
    PmmdzqPrVvPwwTWBwg
    wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn
    ttgJtRGJQctTZtZT
    CrZsJsPPZsGzwwsLwLmpwMDw
    The first rucksack contains the items vJrwpWtwJgWrhcsFMMfFFhFp, which means its first compartment contains the items vJrwpWtwJgWr, while the second compartment contains the items hcsFMMfFFhFp. The only item type that appears in both compartments is lowercase p.
    The second rucksack's compartments contain jqHRNqRjqzjGDLGL and rsFMfFZSrLrFZsSL. The only item type that appears in both compartments is uppercase L.
    The third rucksack's compartments contain PmmdzqPrV and vPwwTWBwg; the only common item type is uppercase P.
    The fourth rucksack's compartments only share item type v.
    The fifth rucksack's compartments only share item type t.
    The sixth rucksack's compartments only share item type s.
    To help prioritize item rearrangement, every item type can be converted to a priority:

    Lowercase item types a through z have priorities 1 through 26.
    Uppercase item types A through Z have priorities 27 through 52.
    In the above example, the priority of the item type that appears in both compartments of each rucksack is 16 (p), 38 (L), 42 (P), 22 (v), 20 (t), and 19 (s); the sum of these is 157.

    Find the item type that appears in both compartments of each rucksack. What is the sum of the priorities of those item types?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        int sum = 0;
        for (const auto& ruck : input) {
            const auto comps = get_compartments(ruck);
            const auto dup = in_both(comps);
            sum += priority(dup);
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    As you finish identifying the misplaced items, the Elves come to you with another issue.

    For safety, the Elves are divided into groups of three. Every Elf carries a badge that identifies their group. For efficiency, within each group of three Elves, the badge is the only item type carried by all three Elves. That is, if a group's badge is item type B, then all three Elves will have item type B somewhere in their rucksack, and at most two of the Elves will be carrying any other item type.

    The problem is that someone forgot to put this year's updated authenticity sticker on the badges. All of the badges need to be pulled out of the rucksacks so the new authenticity stickers can be attached.

    Additionally, nobody wrote down which item type corresponds to each group's badges. The only way to tell which item type is the right one is by finding the one item type that is common between all three Elves in each group.

    Every set of three lines in your list corresponds to a single group, but each group can have a different badge item type. So, in the above example, the first group's rucksacks are the first three lines:

    vJrwpWtwJgWrhcsFMMfFFhFp
    jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL
    PmmdzqPrVvPwwTWBwg
    And the second group's rucksacks are the next three lines:

    wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn
    ttgJtRGJQctTZtZT
    CrZsJsPPZsGzwwsLwLmpwMDw
    In the first group, the only item type that appears in all three rucksacks is lowercase r; this must be their badges. In the second group, their badge item type must be Z.

    Priorities for these items must still be found to organize the sticker attachment efforts: here, they are 18 (r) for the first group and 52 (Z) for the second group. The sum of these is 70.

    Find the item type that corresponds to the badges of each three-Elf group. What is the sum of the priorities of those item types?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        int sum = 0;
        for (auto iter = input.begin(); iter != input.end(); iter += 3) {
            const auto cmn = find_common(iter, iter + 3);
            sum += priority(cmn);
        }
        return std::to_string(sum);
    }

    aoc::registration r{2022, 3, part_1, part_2};

//    TEST_SUITE("2022_day03") {
//        TEST_CASE("2022_day03:example") {
//
//        }
//    }

}