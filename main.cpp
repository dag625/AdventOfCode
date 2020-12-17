#include <iostream>
#include <filesystem>
#include <string>
#include <utility>
#include <algorithm>
#include <chrono>
#include <array>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "time_format.h"
#include "arguments.h"

#include "2020/day1.h"
#include "2020/day2.h"
#include "2020/day3.h"
#include "2020/day4.h"
#include "2020/day5.h"
#include "2020/day6.h"
#include "2020/day7.h"
#include "2020/day8.h"
#include "2020/day9.h"
#include "2020/day10.h"
#include "2020/day11.h"
#include "2020/day12.h"
#include "2020/day13.h"
#include "2020/day14.h"
#include "2020/day15.h"
#include "2020/day16.h"

namespace fs = std::filesystem;

namespace {

    using challenge_function = void (*)(const std::filesystem::path&);

    struct challenge {
        int year;
        int day;
        int num;
        challenge_function function;

        constexpr challenge(int y, int d, int c, challenge_function f) : year{y}, day{d}, num{c}, function{f} {}

        [[nodiscard]] bool matches(std::optional<int> ryear, std::optional<int> rday, std::optional<int> rnum) const {
            if (!ryear.has_value()) {
                return true;
            }
            else if (ryear == year) {
                if (!rday.has_value()) {
                    return true;
                } else if (rday == day) {
                    if (!rnum.has_value() || rnum == num) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        void run_challenge(const fs::path& input_dir) const noexcept {
            try {
                function(input_dir);
            }
            catch (const std::exception& e) {
                std::cerr << '\t' << str() << " failed due to an exception:  " << e.what() << '\n';
            }
            catch (...) {
                std::cerr << '\t' << str() << " failed due to an unknown exception.\n";
            }
        }

        [[nodiscard]] std::string str() const {
            std::stringstream ss;
            ss << "Year " << year << " - Day " << day << " - Challenge " << num;
            return ss.str();
        }
    };

    constexpr auto challenge_solutions = std::array{
            challenge{2020, 1, 1, aoc2020::solve_day_1_1},
            challenge{2020, 1, 2, aoc2020::solve_day_1_2},
            challenge{2020, 2, 1, aoc2020::solve_day_2_1},
            challenge{2020, 2, 2, aoc2020::solve_day_2_2},
            challenge{2020, 3, 1, aoc2020::solve_day_3_1},
            challenge{2020, 3, 2, aoc2020::solve_day_3_2},
            challenge{2020, 4, 1, aoc2020::solve_day_4_1},
            challenge{2020, 4, 2, aoc2020::solve_day_4_2},
            challenge{2020, 5, 1, aoc2020::solve_day_5_1},
            challenge{2020, 5, 2, aoc2020::solve_day_5_2},
            challenge{2020, 6, 1, aoc2020::solve_day_6_1},
            challenge{2020, 6, 2, aoc2020::solve_day_6_2},
            challenge{2020, 7, 1, aoc2020::solve_day_7_1},
            challenge{2020, 7, 2, aoc2020::solve_day_7_2},
            challenge{2020, 8, 1, aoc2020::solve_day_8_1},
            challenge{2020, 8, 2, aoc2020::solve_day_8_2},
            challenge{2020, 9, 1, aoc2020::solve_day_9_1},
            challenge{2020, 9, 2, aoc2020::solve_day_9_2},
            challenge{2020, 10, 1, aoc2020::solve_day_10_1},
            challenge{2020, 10, 2, aoc2020::solve_day_10_2},
            challenge{2020, 11, 1, aoc2020::solve_day_11_1},
            challenge{2020, 11, 2, aoc2020::solve_day_11_2},
            challenge{2020, 12, 1, aoc2020::solve_day_12_1},
            challenge{2020, 12, 2, aoc2020::solve_day_12_2},
            challenge{2020, 13, 1, aoc2020::solve_day_13_1},
            challenge{2020, 13, 2, aoc2020::solve_day_13_2},
            challenge{2020, 14, 1, aoc2020::solve_day_14_1},
            challenge{2020, 14, 2, aoc2020::solve_day_14_2},
            challenge{2020, 15, 1, aoc2020::solve_day_15_1},
            challenge{2020, 15, 2, aoc2020::solve_day_15_2},
            challenge{2020, 16, 1, aoc2020::solve_day_16_1},
            challenge{2020, 16, 2, aoc2020::solve_day_16_2}
    };

}

int non_test_main(int argc, char** argv) {
    using namespace std::string_view_literals;
    fs::path input_dir;
    bool have_required = false, run_challenges = true;
    std::optional<int> year, day, chal;
    parse_arguments(argc, argv)
            .add('i', "input_dir"sv, input_dir)
            .add_flag( "run"sv, run_challenges)
            .add_opt("year"sv, year)
            .add_opt("day"sv, day)
            .add_opt("challenge"sv, chal)
            .run(have_required);

    if (!have_required) {
        std::cerr << "Missing required arguments.\n";
        return 1;
    }
    if (!run_challenges) {
        return 0;
    }

    const auto start = std::chrono::system_clock::now();
    for (const auto &c : challenge_solutions) {
        if (c.matches(year, day, chal)) {
            const auto cstart = std::chrono::system_clock::now();
            std::cout << c.str() << '\n';
            c.run_challenge(input_dir);
            std::cout << "\tChallenge time:  " << (std::chrono::system_clock::now() - cstart) << '\n';
        }
    }
    auto dur = std::chrono::system_clock::now() - start;
    std::cout << "Finished solutions in:  " << dur << '\n';
    return 0;
}

int main(int argc, char** argv) {
    try {
        doctest::Context ctxt {argc, argv};
        int retval = ctxt.run();

        if(ctxt.shouldExit()) {
            return retval;
        }

        return retval + non_test_main(argc, argv); // combine the 2 results
    }
    catch (const std::exception& e) {
        std::cerr << "Exception:  " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception." << std::endl;
        return 1;
    }
}
