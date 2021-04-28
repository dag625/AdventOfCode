//
// Created by Daniel Garcia on 4/27/21.
//

#include "registration.h"
#include "utilities.h"

#include <doctest/doctest.h>

#include <string_view>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    std::vector<std::string> get_input(const std::filesystem::path& input_dir) {
        return aoc::read_file_lines(input_dir / "2015" / "day_5_input.txt");
    }

    bool is_nice_part1(const std::string& s) {
        constexpr auto VOWELS = "aeiou"sv;
        const auto num_vowels = std::count_if(s.begin(), s.end(),
                                              [&VOWELS](char c){ return VOWELS.find(c) != std::string_view::npos; });
        const bool has_bad_subs = s.find("ab") != std::string::npos ||
                s.find("cd") != std::string::npos ||
                s.find("pq") != std::string::npos ||
                s.find("xy") != std::string::npos;
        bool has_double_char = false;
        char last = '\0';
        for (char c : s) {
            if (c == last) {
                has_double_char = true;
                break;
            }
            last = c;
        }
        return num_vowels >= 3 && has_double_char && !has_bad_subs;
    }

    bool is_nice_part2(const std::string& s) {
        bool has_double_char = false;
        char last = '\0', mid = '\0';
        for (char c : s) {
            if (c == last) {
                has_double_char = true;
                break;
            }
            last = mid;
            mid = c;
        }
        bool has_repeated = false;
        std::string_view view {s};
        while (view.size() > 1) {
            auto candidate = view.substr(0, 2);
            auto rest = view.substr(2);
            if (rest.find(candidate) != std::string_view::npos) {
                has_repeated = true;
                break;
            }
            view.remove_prefix(1);
        }
        return has_repeated && has_double_char;
    }

    /*
    --- Day 5: Doesn't He Have Intern-Elves For This? ---

    Santa needs help figuring out which strings in his text file are naughty or nice.

    A nice string is one with all of the following properties:

    It contains at least three vowels (aeiou only), like aei, xazegov, or aeiouaeiouaeiou.
    It contains at least one letter that appears twice in a row, like xx, abcdde (dd), or aabbccdd (aa, bb, cc, or dd).
    It does not contain the strings ab, cd, pq, or xy, even if they are part of one of the other requirements.
    For example:

    ugknbfddgicrmopn is nice because it has at least three vowels (u...i...o...), a double letter (...dd...), and none of the disallowed substrings.
    aaa is nice because it has at least three vowels and a double letter, even though the letters used by different rules overlap.
    jchzalrnumimnmhp is naughty because it has no double letter.
    haegwjzuvuyypxyu is naughty because it contains the string xy.
    dvszwmarrgswjxmb is naughty because it contains only one vowel.
    How many strings are nice?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        auto strs = get_input(input_dir);
        return std::to_string(std::count_if(strs.begin(), strs.end(), is_nice_part1));
    }

    /*
    Realizing the error of his ways, Santa has switched to a better model of determining whether a string is naughty or nice. None of the old rules apply, as they are all clearly ridiculous.

    Now, a nice string is one with all of the following properties:

    It contains a pair of any two letters that appears at least twice in the string without overlapping, like xyxy (xy) or aabcdefgaa (aa), but not like aaa (aa, but it overlaps).
    It contains at least one letter which repeats with exactly one letter between them, like xyx, abcdefeghi (efe), or even aaa.
    For example:

    qjhvhtzxzqqjkmpb is nice because is has a pair that appears twice (qj) and a letter that repeats with exactly one letter between them (zxz).
    xxyxx is nice because it has a pair that appears twice and a letter that repeats with one between, even though the letters used by each rule overlap.
    uurcxstgmygtbstg is naughty because it has a pair (tg) but no repeat with a single letter between them.
    ieodomkazucvgmuy is naughty because it has a repeating letter with one between (odo), but no pair that appears twice.
    How many strings are nice under these new rules?
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto strs = get_input(input_dir);
        return std::to_string(std::count_if(strs.begin(), strs.end(), is_nice_part2));
    }

    aoc::registration r {2015, 5, part_1, part_2};

    TEST_SUITE("2015_day5") {
        TEST_CASE("2015_day5:example") {
//        REQUIRE_EQ(count_visits(calculate_visits(">")), 2);
//        REQUIRE_EQ(count_visits(calculate_visits("^>v<")), 4);
//        REQUIRE_EQ(count_visits(calculate_visits("^v^v^v^v^v")), 2);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^v")), 3);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^>v<")), 3);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^v^v^v^v^v")), 11);
    }

}

}

