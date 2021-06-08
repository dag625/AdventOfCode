//
// Created by Daniel Garcia on 6/7/21.
//

#include "registration.h"
#include "utilities.h"

#include <doctest/doctest.h>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    std::pair<int, int> get_lengths(const std::string_view s) {
        auto contents = s;
        contents.remove_prefix(1);
        contents.remove_suffix(1);
        int sub = 0;
        for (auto idx = 0u; idx < contents.size(); ++idx) {
            if (contents[idx] == '\\') {
                ++idx;
                if (contents[idx] == '\\' || contents[idx] == '"') {
                    ++sub;
                }
                else if (contents[idx] == 'x') {
                    sub += 3;
                    idx += 2;
                }
            }
        }
        return {s.size(), contents.size() - sub};
    }

    std::pair<int, int> get_expanded_lengths(const std::string_view s) {
        int add = 2;
        for (char c : s) {
            if (c == '\"' || c == '\\') {
                ++add;
            }
        }
        return {s.size(), s.size() + add};
    }

    std::vector<std::string> get_input(const std::filesystem::path& input_dir) {
        return aoc::read_file_lines(input_dir / "2015" / "day_8_input.txt");
    }

    /*
    Space on the sleigh is limited this year, and so Santa will be bringing his list as a digital copy. He needs to know how much space it will take up when stored.

    It is common in many programming languages to provide a way to escape special characters in strings. For example, C, JavaScript, Perl, Python, and even PHP handle special characters in very similar ways.

    However, it is important to realize the difference between the number of characters in the code representation of the string literal and the number of characters in the in-memory string itself.

    For example:

    "" is 2 characters of code (the two double quotes), but the string contains zero characters.
    "abc" is 5 characters of code, but 3 characters in the string data.
    "aaa\"aaa" is 10 characters of code, but the string itself contains six "a" characters and a single, escaped quote character, for a total of 7 characters in the string data.
    "\x27" is 6 characters of code, but the string itself contains just one - an apostrophe ('), escaped using hexadecimal notation.
    Santa's list is a file that contains many double-quoted string literals, one on each line. The only escape sequences used are \\ (which represents a single backslash), \" (which represents a lone double-quote character), and \x plus two hexadecimal characters (which represents a single character with that ASCII code).

    Disregarding the whitespace in the file, what is the number of characters of code for string literals minus the number of characters in memory for the values of the strings in total for the entire file?

    For example, given the four strings above, the total number of characters of string code (2 + 5 + 10 + 6 = 23) minus the total number of characters in memory for string values (0 + 3 + 7 + 1 = 11) is 23 - 11 = 12.
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto strings = get_input(input_dir);
        int sum = 0;
        for (const auto& s : strings) {
            auto [total, display] = get_lengths(s);
            sum += (total - display);
        }
        return std::to_string(sum);
    }

    /*
    Now, let's go the other way. In addition to finding the number of characters of code, you should now encode each code representation as a new string and find the number of characters of the new encoded representation, including the surrounding double quotes.

    For example:

    "" encodes to "\"\"", an increase from 2 characters to 6.
    "abc" encodes to "\"abc\"", an increase from 5 characters to 9.
    "aaa\"aaa" encodes to "\"aaa\\\"aaa\"", an increase from 10 characters to 16.
    "\x27" encodes to "\"\\x27\"", an increase from 6 characters to 11.
    Your task is to find the total number of characters to represent the newly encoded strings minus the number of characters of code in each original string literal. For example, for the strings above, the total encoded length (6 + 9 + 16 + 11 = 42) minus the characters in the original code representation (23, just like in the first part of this puzzle) is 42 - 23 = 19.
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto strings = get_input(input_dir);
        int sum = 0;
        for (const auto& s : strings) {
            auto [orig, display] = get_expanded_lengths(s);
            sum += (display - orig);
        }
        return std::to_string(sum);
    }

    aoc::registration r {2015, 8, part_1, part_2};

    TEST_SUITE("2015_day8") {
        TEST_CASE ("2015_day8:example") {
            auto [t1, d1] = get_lengths("\"\"");
            REQUIRE_EQ(t1, 2);
            REQUIRE_EQ(d1, 0);

            auto [t2, d2] = get_lengths("\"abc\"");
            REQUIRE_EQ(t2, 5);
            REQUIRE_EQ(d2, 3);

            auto [t3, d3] = get_lengths("\"aaa\\\"aaa\"");
            REQUIRE_EQ(t3, 10);
            REQUIRE_EQ(d3, 7);

            auto [t4, d4] = get_lengths("\"\\x27\"");
            REQUIRE_EQ(t4, 6);
            REQUIRE_EQ(d4, 1);
        }
    }

}

