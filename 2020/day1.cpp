//
// Created by Daniel Garcia on 12/1/20.
//

#include "registration.h"

#include <vector>
#include <cstdint>
#include <iostream>

#include "utilities.h"

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        std::vector<int> get_input(const std::vector<std::string>& lines) {
            std::vector<int> values;
            std::transform(lines.begin(), lines.end(), std::back_inserter(values), [](const std::string& s){ return std::stoi(s); });
            return values;
        }

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
        std::string solve_day_1(F func, const std::vector<int>& input) {
            auto val = func(input);
            return std::to_string(val);
        }

    }

    /*
    After saving Christmas five years in a row, you've decided to take a vacation at a nice resort on a tropical island. Surely, Christmas will go on without you.

    The tropical island has its own currency and is entirely cash-only. The gold coins used there have a little picture of a starfish; the locals just call them stars. None of the currency exchanges seem to have heard of them, but somehow, you'll need to find fifty of these coins by the time you arrive so you can pay the deposit on your room.

    To save your vacation, you need to get all fifty stars by December 25th.

    Collect stars by solving puzzles. Two puzzles will be made available on each day in the Advent calendar; the second puzzle is unlocked when you complete the first. Each puzzle grants one star. Good luck!

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
    std::string solve_day_1_1(const std::vector<std::string>& lines) {
        return solve_day_1(solve_day_1_1_impl, get_input(lines));
    }

    /*
    The Elves in accounting are thankful for your help; one of them even offers you a starfish coin they had left over from a past vacation. They offer you a second one if you can find three numbers in your expense report that meet the same criteria.

    Using the above example again, the three entries that sum to 2020 are 979, 366, and 675. Multiplying them together produces the answer, 241861950.

    In your expense report, what is the product of the three entries that sum to 2020?
    */
    std::string solve_day_1_2(const std::vector<std::string>& lines) {
        return solve_day_1(solve_day_1_2_impl, get_input(lines));
    }

    static registration r {2020, 1, solve_day_1_1, solve_day_1_2};

} /* namespace aoc2020 */