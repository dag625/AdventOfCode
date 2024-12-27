//
// Created by Daniel Garcia on 12/6/20.
//

#include "registration.h"
#include "utilities.h"

#include <numeric>
#include <bitset>
#include <algorithm>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        using answers = std::bitset<26>;

        answers net_of_group(const std::vector<answers>& group) noexcept {
            return std::accumulate(group.begin(), group.end(), answers{},
                                   [](answers acc, answers v){ return acc | v; });
        }

        answers all_of_group(const std::vector<answers>& group) noexcept {
            return std::accumulate(group.begin(), group.end(), answers{}.flip(),
                                   [](answers acc, answers v){ return acc & v; });
        }

        std::vector<std::vector<answers>> get_input(const std::vector<std::string>& lines) {
            std::vector<std::vector<answers>> retval;
            std::vector<answers> group;
            for (const auto& s : lines) {
                if (s.empty()) {
                    if (!group.empty()) {
                        std::vector<answers> tmp;
                        group.swap(tmp);
                        retval.push_back(std::move(tmp));
                    }
                }
                else {
                    answers a {};
                    for (char c : s) {
                        a.set(c - 'a');
                    }
                    group.push_back(a);
                }
            }
            if (!group.empty()) {
                retval.push_back(std::move(group));
            }
            return retval;
        }

    }

    /************************* Part 1 *************************/
    std::string solve_day_6_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        std::vector<answers> group_answers;
        group_answers.reserve(input.size());
        std::transform(input.begin(), input.end(), std::back_inserter(group_answers), net_of_group);
        auto sum = std::accumulate(group_answers.begin(), group_answers.end(), 0ull,
                                   [](std::size_t acc, answers a){ return acc + a.count(); });
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string solve_day_6_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        std::vector<answers> group_answers;
        group_answers.reserve(input.size());
        std::transform(input.begin(), input.end(), std::back_inserter(group_answers), all_of_group);
        auto sum = std::accumulate(group_answers.begin(), group_answers.end(), 0ull,
                                   [](std::size_t acc, answers a){ return acc + a.count(); });
        return std::to_string(sum);
    }

    static aoc::registration r {2020, 6, solve_day_6_1, solve_day_6_2};

} /* namespace aoc2020 */