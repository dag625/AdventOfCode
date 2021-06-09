//
// Created by Daniel Garcia on 6/8/21.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <string_view>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr auto INPUT = "hxbxwxba"sv;

    bool is_consecutive(std::string_view s) {
        return s.size() == 3 &&
               s[0] + 1 == s[1] &&
               s[1] + 1 == s[2];
    }

    bool is_valid(std::string_view pw) {
        bool any_consecutive = false;
        for (int i = 0; i < 5 && !any_consecutive; ++i) {
            any_consecutive = is_consecutive(pw.substr(i, 3));
        }
        if (!any_consecutive) {
            return false;
        }
        if (pw.find_first_of("iol") != std::string_view::npos) {
            return false;
        }

        char last_double = '\0';
        char last = '\0';
        bool have_two_doubles = false;
        for (char c : pw) {
            if (c == last) {
                if (last_double != c && last_double != '\0') {
                    have_two_doubles = true;
                    break;
                }
                last_double = c;
            }
            last = c;
        }
        return have_two_doubles;
    }

    void increment(std::string &next) {
        for (auto c = next.rbegin(); c != next.rend(); ++c) {
            if (*c == 'z') {
                *c = 'a';
            } else {
                ++*c;
                break;
            }
        }
    }

    std::string iterate(std::string_view prev) {
        std::string next{prev};
        do {
            increment(next);
        } while (!is_valid(next));
        return next;
    }

    /*
    Santa's previous password expired, and he needs help choosing a new one.

    To help him remember his new password after the old one expires, Santa has devised a method of coming up with a password based on the previous one. Corporate policy dictates that passwords must be exactly eight lowercase letters (for security reasons), so he finds his new password by incrementing his old password string repeatedly until it is valid.

    Incrementing is just like counting with numbers: xx, xy, xz, ya, yb, and so on. Increase the rightmost letter one step; if it was z, it wraps around to a, and repeat with the next letter to the left until one doesn't wrap around.

    Unfortunately for Santa, a new Security-Elf recently started, and he has imposed some additional password requirements:

    Passwords must include one increasing straight of at least three letters, like abc, bcd, cde, and so on, up to xyz. They cannot skip letters; abd doesn't count.
    Passwords may not contain the letters i, o, or l, as these letters can be mistaken for other characters and are therefore confusing.
    Passwords must contain at least two different, non-overlapping pairs of letters, like aa, bb, or zz.
    For example:

    hijklmmn meets the first requirement (because it contains the straight hij) but fails the second requirement requirement (because it contains i and l).
    abbceffg meets the third requirement (because it repeats bb and ff) but fails the first requirement.
    abbcegjk fails the third requirement, because it only has one double letter (bb).
    The next password after abcdefgh is abcdffaa.
    The next password after ghijklmn is ghjaabcc, because you eventually skip all the passwords that start with ghi..., since i is not allowed.
    Given Santa's current password (your puzzle input), what should his next password be?
     */
    std::string part_1(const std::filesystem::path &input_dir) {
        return iterate(INPUT);
    }

    /*
    Santa's password expired again. What's the next one?
     */
    std::string part_2(const std::filesystem::path &input_dir) {
        return iterate(iterate(INPUT));
    }

    aoc::registration r{2015, 11, part_1, part_2};

    TEST_SUITE ("2015_day11") {
        TEST_CASE ("2015_day11:example") {

        }
    }

}