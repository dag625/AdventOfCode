//
// Created by Daniel Garcia on 12/2/20.
//

#include "registration.h"

#include <vector>
#include <string>
#include <algorithm>
#include <regex>

#include "utilities.h"

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        std::regex entry_regex {R"(^\s*(\d+)-(\d+)\s+(\w):\s+(\w+)\s*$)"};

        struct password_entry {
            int min;
            int max;
            char character;
            std::string password;
        };

        password_entry parse(const std::string& s) {
            int min, max;
            char ch;
            std::string pw;
            std::match_results<std::string::const_iterator> match;
            if (std::regex_match(s, match, entry_regex, std::regex_constants::match_not_null) && match.size() == 5) {
                min = std::stoi(match[1]);
                max = std::stoi(match[2]);
                ch = match[3].str().at(0);
                pw = match[4];
            }
            else {
                throw std::runtime_error{"Failed to parse password entry."};
            }
            return {min, max, ch, pw};
        }

        bool is_valid_old_job(const password_entry& entry) {
            auto num = std::count(entry.password.begin(), entry.password.end(), entry.character);
            return num >= entry.min && num <= entry.max;
        }

        bool is_valid_new_job(const password_entry& entry) {
            auto idx1 = entry.min - 1, idx2 = entry.max - 1;
            auto matching =
                    (entry.password[idx1] == entry.character ? 1 : 0) +
                    (entry.password[idx2] == entry.character ? 1 : 0);
            return matching == 1;
        }

        std::vector<password_entry> get_input(const fs::path& input_dir) {
            auto input = aoc2020::read_file_lines(input_dir / "2020" / "day_2_input.txt");
            std::vector<password_entry> values;
            std::transform(input.begin(), input.end(), std::back_inserter(values), parse);
            return values;
        }

        int solve_day_2_1_impl(const std::vector<password_entry>& input) {
            return std::count_if(input.begin(), input.end(), is_valid_old_job);
        }

        int solve_day_2_2_impl(const std::vector<password_entry>& input) {
            return std::count_if(input.begin(), input.end(), is_valid_new_job);
        }

        template <typename F>
        std::string solve_day_2(F func, const std::vector<password_entry>& input) {
            auto val = func(input);
            return std::to_string(val);
        }

    }

    /*
    Your flight departs in a few days from the coastal airport; the easiest way down to the coast from here is via toboggan.

    The shopkeeper at the North Pole Toboggan Rental Shop is having a bad day. "Something's wrong with our computers; we can't log in!" You ask if you can take a look.

    Their password database seems to be a little corrupted: some of the passwords wouldn't have been allowed by the Official Toboggan Corporate Policy that was in effect when they were chosen.

    To try to debug the problem, they have created a list (your puzzle input) of passwords (according to the corrupted database) and the corporate policy when that password was set.

    For example, suppose you have the following list:

    1-3 a: abcde
    1-3 b: cdefg
    2-9 c: ccccccccc
    Each line gives the password policy and then the password. The password policy indicates the lowest and highest number of times a given letter must appear for the password to be valid. For example, 1-3 a means that the password must contain a at least 1 time and at most 3 times.

    In the above example, 2 passwords are valid. The middle password, cdefg, is not; it contains no instances of b, but needs at least 1. The first and third passwords are valid: they contain one a or nine c, both within the limits of their respective policies.

    How many passwords are valid according to their policies?
    */
    std::string solve_day_2_1(const fs::path& input_dir) {
        return solve_day_2(solve_day_2_1_impl, get_input(input_dir));
    }

    /*
    While it appears you validated the passwords correctly, they don't seem to be what the Official Toboggan Corporate Authentication System is expecting.

    The shopkeeper suddenly realizes that he just accidentally explained the password policy rules from his old job at the sled rental place down the street! The Official Toboggan Corporate Policy actually works a little differently.

    Each policy actually describes two positions in the password, where 1 means the first character, 2 means the second character, and so on. (Be careful; Toboggan Corporate Policies have no concept of "index zero"!) Exactly one of these positions must contain the given letter. Other occurrences of the letter are irrelevant for the purposes of policy enforcement.

    Given the same example list from above:

    1-3 a: abcde is valid: position 1 contains a and position 3 does not.
    1-3 b: cdefg is invalid: neither position 1 nor position 3 contains b.
    2-9 c: ccccccccc is invalid: both position 2 and position 9 contain c.
    How many passwords are valid according to the new interpretation of the policies?
    */
    std::string solve_day_2_2(const fs::path& input_dir) {
        return solve_day_2(solve_day_2_2_impl, get_input(input_dir));
    }

    static registration r {2020, 2, solve_day_2_1, solve_day_2_2};

} /* namespace aoc2020 */