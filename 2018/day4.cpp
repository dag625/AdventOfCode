//
// Created by Dan on 12/15/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2018/day/4
     */

    struct shift {
        int id = 0;
        int month = 0;
        int day = 0;
        int start = 0; //diff from midnight
        std::vector<int> sleeps;
        std::vector<int> wakes;
    };

    std::optional<shift> is_shift_start(std::string_view s) {
        const auto parts = split_by_all_no_empty(s, "[]-: #");
        //From example:  [1518-11-01 00:00] Guard #10 begins shift
        if (parts[5] == "Guard") {
            int mins = parse<int>(parts[4]);
            if (mins > 30) {
                //They're early, this covers all cases in my input.
                mins = 60 - mins;
            }
            return shift{parse<int>(parts[6]), parse<int>(parts[1]), parse<int>(parts[2]), mins, {}, {}};
        }
        else {
            return std::nullopt;
        }
    }

    std::pair<int, bool> parse_sleep(std::string_view s) {
        const auto parts = split_by_all_no_empty(s, "[]-: #");
        //From example:
        //[1518-11-01 00:05] falls asleep
        //[1518-11-01 00:25] wakes up
        return {parse<int>(parts[4]), parts[5] == "falls"};
    }

    std::vector<shift> to_shifts(std::vector<std::string> lines) {
        std::sort(lines.begin(), lines.end());//Get in chronological order
        std::vector<shift> shifts;
        for (const auto& l : lines) {
            auto s = is_shift_start(l);
            if (s) {
                shifts.push_back(std::move(*s));
            }
            else {
                const auto [min, sleep] = parse_sleep(l);
                if (sleep) {
                    shifts.back().sleeps.push_back(min);
                }
                else {
                    shifts.back().wakes.push_back(min);
                }
            }
        }
        return shifts;
    }

    std::vector<shift> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2018" / "day_4_input.txt");
        return to_shifts(lines);
    }

    struct guard_sleeps {
        int id = 0;
        int sleep_time = 0;
        std::vector<int> sleeps;
        std::vector<int> wakes;

        bool operator==(const int rhs) const { return id == rhs; }
        auto operator<=>(const guard_sleeps& rhs) const { return sleep_time <=> rhs.sleep_time; }

        void calc_total_sleep_time() {
            sleep_time = 0;
            for (int i = 0; i < sleeps.size(); ++i) {
                sleep_time += wakes[i] - sleeps[i];
            }
        }

        [[nodiscard]] std::pair<int, int> most_likely_sleep_min() const {
            std::array<int, 60> mins{};
            for (int i = 0; i < sleeps.size(); ++i) {
                for (int j = sleeps[i]; j < wakes[i]; ++j) {
                    ++mins[j];
                }
            }
            const auto max_e = std::max_element(mins.begin(), mins.end());
            return {static_cast<int>(std::distance(mins.begin(), max_e)), *max_e};
        }
    };

    std::vector<guard_sleeps> to_sleeps(const std::vector<shift>& shifts) {
        std::vector<guard_sleeps> retval;
        for (const auto& s : shifts) {
            const auto found = std::find(retval.begin(), retval.end(), s.id);
            if (found != retval.end()) {
                found->sleeps.insert(found->sleeps.end(), s.sleeps.begin(), s.sleeps.end());
                found->wakes.insert(found->wakes.end(), s.wakes.begin(), s.wakes.end());
            }
            else {
                retval.emplace_back(s.id, 0, s.sleeps, s.wakes);
            }
        }
        std::for_each(retval.begin(), retval.end(), [](auto& s){ s.calc_total_sleep_time(); });
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto sleeps = to_sleeps(input);
        const auto sleepiest = std::max_element(sleeps.begin(), sleeps.end());
        const auto [sleepiest_min, num] = sleepiest->most_likely_sleep_min();
        return std::to_string(sleepiest->id * sleepiest_min);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto sleeps = to_sleeps(input);
        int id = 0, min = -1, times = 0;
        for (const auto& s : sleeps) {
            const auto [sleepiest_min, num] = s.most_likely_sleep_min();
            if (num > times) {
                times = num;
                id = s.id;
                min = sleepiest_min;
            }
        }
        return std::to_string(id * min);
    }

    aoc::registration r{2018, 4, part_1, part_2};

    TEST_SUITE("2018_day04") {
        TEST_CASE("2018_day04:example") {
            const std::vector<std::string> lines {
                    "[1518-11-01 00:00] Guard #10 begins shift",
                    "[1518-11-01 00:05] falls asleep",
                    "[1518-11-01 00:25] wakes up",
                    "[1518-11-01 00:30] falls asleep",
                    "[1518-11-01 00:55] wakes up",
                    "[1518-11-01 23:58] Guard #99 begins shift",
                    "[1518-11-02 00:40] falls asleep",
                    "[1518-11-02 00:50] wakes up",
                    "[1518-11-03 00:05] Guard #10 begins shift",
                    "[1518-11-03 00:24] falls asleep",
                    "[1518-11-03 00:29] wakes up",
                    "[1518-11-04 00:02] Guard #99 begins shift",
                    "[1518-11-04 00:36] falls asleep",
                    "[1518-11-04 00:46] wakes up",
                    "[1518-11-05 00:03] Guard #99 begins shift",
                    "[1518-11-05 00:45] falls asleep",
                    "[1518-11-05 00:55] wakes up"
            };
            const auto input = to_shifts(lines);
            const auto sleeps = to_sleeps(input);
            const auto sleepiest = std::max_element(sleeps.begin(), sleeps.end());
            const auto [sleepiest_min1, num1] = sleepiest->most_likely_sleep_min();
            CHECK_EQ(sleepiest->id, 10);
            CHECK_EQ(sleepiest_min1, 24);

            int id = 0, min = -1, times = 0;
            for (const auto& s : sleeps) {
                const auto [sleepiest_min2, num2] = s.most_likely_sleep_min();
                if (num2 > times) {
                    times = num2;
                    id = s.id;
                    min = sleepiest_min2;
                }
            }
            CHECK_EQ(id, 99);
            CHECK_EQ(min, 45);
        }
    }

} /* namespace <anon> */