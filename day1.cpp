//
// Created by Daniel Garcia on 12/1/20.
//

#include "day1.h"

#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "utilities.h"

namespace fs = std::filesystem;

namespace aoc {

    namespace {

        std::vector<int> get_input(const fs::path& input_dir) {
            std::string line;
            auto input_file = input_dir / "day_1_input.txt";
            std::vector<std::string> input;
            std::ifstream in {input_file};
            while (std::getline(in, line) && !line.empty()) {
                input.push_back(std::move(trim(line)));
            }
            std::vector<int> values;
            std::transform(input.begin(), input.end(), std::back_inserter(values), [](const std::string& s){ return std::stoi(s); });
            return values;
        }

        /*
        Before you leave, the Elves in accounting just need you to fix your expense report (your puzzle input); apparently, something isn't quite adding up.

        Specifically, they need you to find the two entries that sum to 2020 and then multiply those two numbers together.

        For example, suppose your expense report contained the following:

        1721
        979
        366
        299
        675
        1456
        In this list, the two entries that sum to 2020 are 1721 and 299. Multiplying them together produces 1721 * 299 = 514579, so the correct answer is 514579.

        Of course, your expense report is much larger. Find the two entries that sum to 2020; what do you get if you multiply them together?
        */
        int solve_day_1_1_impl(const std::vector<int>& input) {
            const int sum = 2020;
            for (int i = 0; i < input.size(); ++i) {
                for (int j = i + 1; j < input.size(); ++j) {
                    if (input[i] + input[j] == sum) {
                        return input[i] * input[j];
                    }
                }
            }
            return -1;
        }

        /*
        The Elves in accounting are thankful for your help; one of them even offers you a starfish coin they had left over from a past vacation. They offer you a second one if you can find three numbers in your expense report that meet the same criteria.

        Using the above example again, the three entries that sum to 2020 are 979, 366, and 675. Multiplying them together produces the answer, 241861950.

        In your expense report, what is the product of the three entries that sum to 2020?
        */
        int64_t solve_day_1_2_impl(const std::vector<int>& input) {
            const int sum = 2020;
            for (int i = 0; i < input.size(); ++i) {
                for (int j = i + 1; j < input.size(); ++j) {
                    for (int k = j + 1; k < input.size(); ++k) {
                        if (input[i] + input[j] + input[k] == sum) {
                            return static_cast<int64_t>(input[i]) * input[j] * input[k];
                        }
                    }
                }
            }
            return -1;
        }

        template <typename F>
        void solve_day_1(F func, const std::vector<int>& input) {
            auto val = func(input);
            std::cout << '\t' << val << std::endl;
        }

    }

    void solve_day_1_1(const std::filesystem::path& input_dir) {
        solve_day_1(solve_day_1_1_impl, get_input(input_dir));
    }

    void solve_day_1_2(const std::filesystem::path& input_dir) {
        solve_day_1(solve_day_1_2_impl, get_input(input_dir));
    }

} /* namespace aoc */