//
// Created by Daniel Garcia on 12/2/20.
//

#include "day2.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <regex>

#include "utilities.h"

namespace fs = std::filesystem;

namespace aoc {

    namespace {

        std::regex entry_regex {"^\\s*(\\d+)-(\\d+)\\s+(\\w):\\s+(\\w+)\\s*$"};

        struct password_entry {
            int min;
            int max;
            char character;
            std::string password;
        };

        password_entry parse(const std::string& s) {
            int min = 0, max = 0;
            char ch = '\0';
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
            std::string line;
            auto input_file = input_dir / "day_2_input.txt";
            std::vector<std::string> input;
            std::ifstream in {input_file};
            while (std::getline(in, line) && !line.empty()) {
                input.push_back(std::move(trim(line)));
            }
            std::vector<password_entry> values;
            std::transform(input.begin(), input.end(), std::back_inserter(values), parse);
            return values;
        }

        /*
        The shopkeeper at the North Pole Toboggan Rental Shop is having a bad day. "Something's wrong with our computers; we can't log in!" You ask if you can take a look.

        Their password database seems to be a little corrupted: some of the passwords wouldn't have been allowed by the Official Toboggan Corporate Policy that was in effect when they were chosen.

        To try to debug the problem, they have created a list (your puzzle input) of passwords (according to the corrupted database) and the corporate policy when that password was set.

        For example, suppose you have the following list:

        1-3 a: abcde
        1-3 b: cdefg
        2-9 c: ccccccccc
        Each line gives the password policy and then the password. The password policy indicates the lowest and highest number of times a given letter must appear for the password to be valid. For example, 1-3 a means that the password must contain a at least 1 time and at most 3 times.

        In the above example, 2 passwords are valid. The middle password, cdefg, is not; it contains no instances of b, but needs at least 1. The first and third passwords are valid: they contain one a or nine c, both within the limits of their respective policies.
        */
        int solve_day_2_1_impl(const std::vector<password_entry>& input) {
            return std::count_if(input.begin(), input.end(), is_valid_old_job);
        }

        int solve_day_2_2_impl(const std::vector<password_entry>& input) {
            return std::count_if(input.begin(), input.end(), is_valid_new_job);
        }

        template <typename F>
        void solve_day_2(F func, const std::vector<password_entry>& input) {
            auto val = func(input);
            std::cout << '\t' << val << std::endl;
        }

    }

    void solve_day_2_1(const fs::path& input_dir) {
        solve_day_2(solve_day_2_1_impl, get_input(input_dir));
    }

    void solve_day_2_2(const fs::path& input_dir) {
        solve_day_2(solve_day_2_2_impl, get_input(input_dir));
    }

} /* namespace aoc */