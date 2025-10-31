//
// Created by Dan on 10/7/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"
#include "ranges.h"

namespace {

    using namespace aoc;
    
    /*
     See https://adventofcode.com/2018/day/7
     */

    struct constraint {
        char before = '\0';
        char target = '\0';
    };

    constraint parse_constraint(std::string_view line) {
        //Like:  Step U must be finished before step F can begin.
        line.remove_prefix(5);
        const char before = line[0];
        line.remove_prefix(31);
        const char target = line[0];
        return {before, target};
    }

    std::vector<constraint> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_constraint) | std::ranges::to<std::vector>();
    }

    struct target_info {
        char target = '\0';
        std::vector<char> prereqs;

        target_info(char target) : target(target) {}
        target_info(char target, char before) : target(target) { prereqs.push_back(before); }

        auto operator<=>(const target_info& other) const { return target <=> other.target; }
        bool operator==(const target_info& other) const { return target == other.target; }
    };

    std::vector<target_info> compile_targets(const std::vector<constraint>& constraints) {
        std::vector<target_info> retval;
        retval.reserve(26);

        for (const auto& c : constraints) {
            auto found = std::find_if(retval.begin(), retval.end(), [&c](const auto& t){ return t.target == c.target; });
            if (found != retval.end()) {
                found->prereqs.push_back(c.before);
            }
            else {
                retval.emplace_back(c.target, c.before);
            }
        }

        return retval;
    }

    std::string build_path(std::vector<target_info> targets) {
        std::string retval;
        retval.reserve(targets.size());

        std::array<bool, 26> are_targets{};
        for (const auto& t : targets) {
            are_targets[t.target - 'A'] = true;
        }
        for (const auto& t : targets) {
            for (const auto& p : t.prereqs) {
                if (!are_targets[p - 'A']) {
                    retval.push_back(p);
                }
            }
        }
        std::sort(retval.begin(), retval.end());
        retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        for (const char c : retval | std::views::drop(1)) {
            targets.emplace_back(c);
        }
        retval.resize(1);
        std::sort(targets.begin(), targets.end());

        while (!targets.empty()) {
            auto can_add = [&retval](const auto& t){ return std::all_of(t.prereqs.begin(), t.prereqs.end(), [&retval](const char c){ return retval.contains(c); }); };
            auto next = targets | std::views::filter(can_add) | std::views::transform([](const auto& t){ return t.target; }) | std::views::take(1) | std::ranges::to<std::vector>();
            targets.erase(std::remove_if(targets.begin(), targets.end(), [&next](const auto& t){ return t.target == next.front(); }), targets.end());
            retval.push_back(next.front());
        }

        return retval;
    }

    constexpr int TIME_PADDING = 60;

    struct worker {
        std::optional<char> current;
        int finish = -1;

        bool work_on(const char t, const int current_step, const int padding) {
            if (current) { return false; }
            current = t;
            finish = current_step + (t - 'A') + padding + 1;//When step == finish, then we add it to the result and free up the worker.
            return true;
        }
        char check_finished(const int current_step) {
            if (current && finish == current_step) {
                finish = -1;
                const char c = *current;
                current = std::nullopt;
                return c;
            }
            else {
                return '\0';
            }
        }
    };

    template <int NUM_WORKERS, int PADDING, char MAX_LETTER = 'Z'>
    int build_path_with_help(std::vector<target_info> targets) {
        int step = 0;

        std::string finished;
        finished.reserve(targets.size());

        std::array<worker, NUM_WORKERS> workers{};

        std::string starters;
        std::array<bool, 26> are_targets{};
        for (char c = MAX_LETTER + 1; c <= 'Z'; ++c) {
            are_targets[c - 'A'] = true;
        }
        for (const auto& t : targets) {
            are_targets[t.target - 'A'] = true;
        }
        for (const auto& t : targets) {
            for (const auto& p : t.prereqs) {
                if (!are_targets[p - 'A']) {
                    starters.push_back(p);
                }
            }
        }
        std::sort(starters.begin(), starters.end());
        starters.erase(std::unique(starters.begin(), starters.end()), starters.end());
        for (const auto [idx, c] : starters | std::views::take(workers.size()) | std::views::enumerate) {
            workers[idx].work_on(c, step, PADDING);
        }
        for (const char c : starters | std::views::drop(workers.size())) {
            targets.emplace_back(c);
        }
        std::sort(targets.begin(), targets.end());

        for (; !targets.empty() ||
                std::any_of(workers.begin(), workers.end(), [](const auto& w){ return w.current.has_value(); });
                ++step)
        {
            bool any_finished = false;
            for (auto& w : workers) {
                const char c = w.check_finished(step);
                if (c) {
                    finished.push_back(c);
                    any_finished = true;
                }
            }
            if (!any_finished) {
                continue;;
            }

            auto ready = targets
                | std::views::filter([&finished](const auto& t){ return std::all_of(t.prereqs.begin(), t.prereqs.end(),
                    [&finished](const char c){ return finished.contains(c); }); });
            if (ready.empty()) {
                continue;
            }

            int w = 0;
            auto current = ready.begin();
            std::string to_remove;
            to_remove.reserve(targets.size());
            while (current != ready.end() && w < workers.size()) {
                for (; w < workers.size() && !workers[w].work_on(current->target, step, PADDING); ++w) {}
                if (w < workers.size()) {
                    to_remove.push_back(current->target);
                    ++current;
                    ++w;
                }
            }
            targets.erase(std::remove_if(targets.begin(), targets.end(),
                [&to_remove](const auto& t){ return to_remove.contains(t.target); }), targets.end());
        }

        return step - 1;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto targets = compile_targets(input);
        const auto path = build_path(targets);
        return path;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto targets = compile_targets(input);
        const auto num_steps = build_path_with_help<5, TIME_PADDING>( targets);
        return std::to_string(num_steps);
        //916 too low
        //917 too low
        //988 too high
    }

    aoc::registration r {2018, 7, part_1, part_2};

    TEST_SUITE("2018_day7") {
        TEST_CASE("2018_day7:example") {
            const std::vector<std::string> lines {
                "Step C must be finished before step A can begin.",
                "Step C must be finished before step F can begin.",
                "Step A must be finished before step B can begin.",
                "Step A must be finished before step D can begin.",
                "Step B must be finished before step E can begin.",
                "Step D must be finished before step E can begin.",
                "Step F must be finished before step E can begin."
            };
            const auto input = get_input(lines);
            const auto targets = compile_targets(input);
            const auto num_steps = build_path_with_help<2, 0, 'F'>( targets);
            CHECK_EQ(num_steps, 15);
        }
    }

} /* namespace <anon> */