//
// Created by Dan on 12/27/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <set>
#include <unordered_map>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct conversion {
        std::string from;
        std::string to;
    };

    conversion to_conversion(std::string_view s) {
        const auto parts = split(s, " => ");
        return {std::string{parts[0]}, std::string{parts[1]}};
    }

    std::pair<std::vector<conversion>, std::string> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2015" / "day_19_input.txt");
        return {lines | std::views::take(lines.size() - 2) | std::views::transform(&to_conversion) | to<std::vector<conversion>>(), lines.back()};
    }

    std::set<std::string> convert_once(const std::string& orig, const conversion& c) {
        std::set<std::string> retval;
        std::string::size_type pos = orig.find(c.from, 0);
        while (pos != std::string::npos) {
            auto repl = orig;
            repl.replace(pos, c.from.size(), c.to, 0);
            retval.insert(repl);
            pos = orig.find(c.from, pos + 1);
        }
        return retval;
    }

    int try_create(int step, int best, const std::string& current, const std::string& dest, const std::vector<conversion>& conversions) {
        if (step > best || current.size() > dest.size()) {
            return -1;
        }
        else if (current == dest) {
            return step;
        }
        bool have_valid = false;
        int min = std::numeric_limits<int>::max();
        for (const auto& c : conversions) {
            auto next = convert_once(current, c);
            for (const auto& v : next) {
                const auto val = try_create(step + 1, best, v, dest, conversions);
                if (val != -1) {
                    have_valid = true;
                    if (val < min) {
                        min = val;
                        if (val < best) {
                            best = val;
                        }
                    }
                }
            }
        }
        return have_valid ? min : -1;
    }

    /*
    --- Day 19: Medicine for Rudolph ---
    Rudolph the Red-Nosed Reindeer is sick! His nose isn't shining very brightly, and he needs medicine.

    Red-Nosed Reindeer biology isn't similar to regular reindeer biology; Rudolph is going to need custom-made medicine. Unfortunately, Red-Nosed Reindeer chemistry isn't similar to regular reindeer chemistry, either.

    The North Pole is equipped with a Red-Nosed Reindeer nuclear fusion/fission plant, capable of constructing any Red-Nosed Reindeer molecule you need. It works by starting with some input molecule and then doing a series of replacements, one per step, until it has the right molecule.

    However, the machine has to be calibrated before it can be used. Calibration involves determining the number of molecules that can be generated in one step from a given starting point.

    For example, imagine a simpler machine that supports only the following replacements:

    H => HO
    H => OH
    O => HH
    Given the replacements above and starting with HOH, the following molecules could be generated:

    HOOH (via H => HO on the first H).
    HOHO (via H => HO on the second H).
    OHOH (via H => OH on the first H).
    HOOH (via H => OH on the second H).
    HHHH (via O => HH).
    So, in the example above, there are 4 distinct molecules (not five, because HOOH appears twice) after one replacement from HOH. Santa's favorite molecule, HOHOHO, can become 7 distinct molecules (over nine replacements: six from H, and three from O).

    The machine replaces without regard for the surrounding characters. For example, given the string H2O, the transition H => OO would result in OO2O.

    Your puzzle input describes all of the possible replacements and, at the bottom, the medicine molecule for which you need to calibrate the machine. How many distinct molecules can be created after all the different ways you can do one replacement on the medicine molecule?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto [conversions, orig] = get_input(input_dir);
        std::set<std::string> results;
        for (const auto& c : conversions) {
            auto converted = convert_once(orig, c);
            results.insert(converted.begin(), converted.end());
        }
        return std::to_string(results.size());
    }

    /*
    --- Part Two ---
    Now that the machine is calibrated, you're ready to begin molecule fabrication.

    Molecule fabrication always begins with just a single electron, e, and applying replacements one at a time, just like the ones during calibration.

    For example, suppose you have the following replacements:

    e => H
    e => O
    H => HO
    H => OH
    O => HH
    If you'd like to make HOH, you start with e, and then make the following replacements:

    e => O to get O
    O => HH to get HH
    H => OH (on the second H) to get HOH
    So, you could make HOH after 3 steps. Santa's favorite molecule, HOHOHO, can be made in 6 steps.

    How long will it take to make the medicine? Given the available replacements and the medicine molecule in your puzzle input, what is the fewest number of steps to go from e to the medicine molecule?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto [conversions, orig] = get_input(input_dir);
        /*
         * See:  https://www.reddit.com/r/adventofcode/comments/3xflz8/comment/cy4etju/?utm_source=share&utm_medium=web2x&context=3
         *
         * The rules all map tokens that are either a single upper case character or an upper case character followed
         * by a lower case character.  The number of tokens is thus the number of upper case characters.  Note that
         * some tokens share the same upper case character.
         *
         * Basically there are two types of non-terminal rules (with A, B, C, D, and E being any other token):
         *      A => BC
         *      A => BRn...Ar
         * The '...' will have one of the following forms:
         *      C
         *      CYD
         *      CYDYE
         *      etc.
         * This can be thought of by mapping the 'Rn', 'Y', and 'Ar' sequences to '(', ',', and ')'.  E.g. they
         * are basically parenthesized statements.
         *
         * So to go backwards from the 'orig' destination "medicine" to the value 'e' (which is simpler to work
         * with and think about but which should give the same answer) we note the following things.
         *
         * First the "A => BC" rules will decrease the number of tokens by 1.  So to reduce the answer using only
         * conversions of this form will take 'orig.size() - 1' steps.
         *
         * Second the "A => BRnCAr" rules (e.g. with no 'Y's) will behave the same as the "A => BC" but also reduce
         * two other tokens for free (the 'Rn' and 'Ar'). So the number steps needed can be reduced by the number of
         * 'Rn' tokens ('Ar' is hard to countbecause there is also a 'Al' token; there is no other token starting
         * with 'R') times two (to count both the 'Rn' and 'Ar' tokens).
         *
         * Third the number of 'Y' tokens reduces the number of steps further by two for each 'Y'.  This is because 'Y'
         * acts as the ',' to the 'Rn' and 'Ar' tokens' '(' and ')' for adding extra tokens inside the parentheses.
         *
         * So the equation is:
         *      steps = num_tokens - num_Rs * 2 - num_Ys * 2 - 1
         */
        long long num_toks = std::count_if(orig.begin(), orig.end(), [](char c){ return isupper(c); }),
            num_parens = std::count_if(orig.begin(), orig.end(), [](char c){ return c == 'R'; }),
            num_commas = std::count_if(orig.begin(), orig.end(), [](char c){ return c == 'Y'; });
        return std::to_string(num_toks - num_parens * 2 - num_commas * 2 - 1);
    }

    aoc::registration r {2015, 19, part_1, part_2};

    TEST_SUITE("2015_day19") {
        TEST_CASE("2015_day19:example") {
            std::vector<conversion> conversions = {
                    {"H", "HO"},
                    {"H", "OH"},
                    {"O", "HH"}
            };
            std::string test1 = "HOH";
            std::set<std::string> results1;
            for (const auto& c : conversions) {
                auto converted = convert_once(test1, c);
                results1.insert(converted.begin(), converted.end());
            }
            REQUIRE_EQ(results1.size(), 4);
        }
        TEST_CASE("2015_day19:test1") {
            std::vector<conversion> conversions = {
                    {"Ho", "Heh"}
            };
            std::string test1 = "oHoHHoTHo";
            std::set<std::string> results1;
            auto converted = convert_once(test1, conversions[0]);
            REQUIRE_EQ(converted.size(), 3);
        }
        TEST_CASE("2015_day19:test2") {
            std::vector<conversion> conversions = {
                    {"Ho", "Heh"}
            };
            std::string test1 = "xoxoxHoooxox";
            std::set<std::string> results1;
            auto converted = convert_once(test1, conversions[0]);
            REQUIRE_EQ(converted.size(), 1);
            REQUIRE_EQ(*converted.begin(), "xoxoxHehooxox");
        }
        TEST_CASE("2015_day19:example_pt2") {
            std::vector<conversion> conversions = {
                    {"e", "H"},
                    {"e", "O"},
                    {"H", "HO"},
                    {"H", "OH"},
                    {"O", "HH"}
            };
            std::unordered_map<std::string, int> cache;
            const int steps1a = try_create(0, std::numeric_limits<int>::max(), "e", "HOH", conversions);
            const int steps2a = try_create(0, std::numeric_limits<int>::max(), "e", "HOHOHO", conversions);
            REQUIRE_EQ(steps1a, 3);
            REQUIRE_EQ(steps2a, 6);
        }
    }

}