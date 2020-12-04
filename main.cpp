#include <iostream>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <array>

#include "day1.h"
#include "day2.h"
#include "day3.h"

namespace fs = std::filesystem;

namespace {

    using challenge_function = void (*)(const std::filesystem::path&);

    struct challenge {
        int day;
        int num;
        challenge_function function;

        constexpr challenge(int d, int c, challenge_function f) : day{d}, num{c}, function{f} {}

        [[nodiscard]] bool matches(int rday, int rnum) const {
            if (rday <= 0) {
                return true;
            }
            else if (rday == day) {
                if (rnum <= 0 || rnum == num) {
                    return true;
                }
                else {
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
            ss << "Day " << day << " - Challenge " << num;
            return ss.str();
        }
    };

    constexpr auto challenge_solutions = std::array{
            challenge{1, 1, aoc::solve_day_1_1},
            challenge{1, 2, aoc::solve_day_1_2},
            challenge{2, 1, aoc::solve_day_2_1},
            challenge{2, 2, aoc::solve_day_2_2},
            challenge{3, 1, aoc::solve_day_3_1},
            challenge{3, 2, aoc::solve_day_3_2}
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

        int day = 0, chal = 0;
        if (argc > 2) {
            day = std::stoi(argv[2]);
        }
        if (argc > 3) {
            chal = std::stoi(argv[3]);
        }

        for (const auto& c : challenge_solutions) {
            if (c.matches(day, chal)) {
                std::cout << c.str() << '\n';
                c.run_challenge(input_dir);
            }
        }
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
