#include <iostream>
#include <filesystem>
#include <string>
#include <utility>
#include <algorithm>
#include <sstream>
#include <array>
#include <chrono>

#include "time_format.h"

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

namespace fs = std::filesystem;

namespace {

    using challenge_function = void (*)(const std::filesystem::path&);

    struct challenge {
        int year;
        int day;
        int num;
        challenge_function function;

        constexpr challenge(int y, int d, int c, challenge_function f) : year{y}, day{d}, num{c}, function{f} {}

        [[nodiscard]] bool matches(int ryear, int rday, int rnum) const {
            if (ryear <= 0) {
                return true;
            }
            else if (ryear == year) {
                if (rday <= 0) {
                    return true;
                } else if (rday == day) {
                    if (rnum <= 0 || rnum == num) {
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
            challenge{2020, 10, 2, aoc2020::solve_day_10_2}
    };

}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Need directory containing input files." << std::endl;
            return 1;
        }
        auto input_dir = fs::path{argv[1]};
        if (!fs::exists(input_dir)) {
            std::cerr << "Input directory '" << input_dir << "' does not exist." << std::endl;
            return 1;
        }
        else if (!fs::is_directory(input_dir)) {
            std::cerr << "Input directory '" << input_dir << "' is not a directory." << std::endl;
            return 1;
        }

        int year = 0, day = 0, chal = 0;
        if (argc > 2) {
            year = std::stoi(argv[2]);
        }
        if (argc > 3) {
            day = std::stoi(argv[3]);
        }
        if (argc > 4) {
            chal = std::stoi(argv[4]);
        }

        const auto start = std::chrono::system_clock::now();
        for (const auto& c : challenge_solutions) {
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
    catch (const std::exception& e) {
        std::cerr << "Exception:  " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception." << std::endl;
        return 1;
    }
}
