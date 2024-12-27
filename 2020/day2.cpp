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

        std::vector<password_entry> get_input(const std::vector<std::string>& lines) {
            std::vector<password_entry> values;
            std::transform(lines.begin(), lines.end(), std::back_inserter(values), parse);
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

    /************************* Part 1 *************************/
    std::string solve_day_2_1(const std::vector<std::string>& lines) {
        return solve_day_2(solve_day_2_1_impl, get_input(lines));
    }

    /************************* Part 2 *************************/
    std::string solve_day_2_2(const std::vector<std::string>& lines) {
        return solve_day_2(solve_day_2_2_impl, get_input(lines));
    }

    static registration r {2020, 2, solve_day_2_1, solve_day_2_2};

} /* namespace aoc2020 */