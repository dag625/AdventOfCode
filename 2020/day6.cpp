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

        std::vector<std::vector<answers>> get_input(const fs::path& input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_6_input.txt");
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

    /*
    As your flight approaches the regional airport where you'll switch to a much larger plane, customs declaration forms are distributed to the passengers.

    The form asks a series of 26 yes-or-no questions marked a through z. All you need to do is identify the questions for which anyone in your group answers "yes". Since your group is just you, this doesn't take very long.

    However, the person sitting next to you seems to be experiencing a language barrier and asks if you can help. For each of the people in their group, you write down the questions for which they answer "yes", one per line. For example:

    abcx
    abcy
    abcz
    In this group, there are 6 questions to which anyone answered "yes": a, b, c, x, y, and z. (Duplicate answers to the same question don't count extra; each question counts at most once.)

    Another group asks for your help, then another, and eventually you've collected answers from every group on the plane (your puzzle input). Each group's answers are separated by a blank line, and within each group, each person's answers are on a single line. For example:

    abc

    a
    b
    c

    ab
    ac

    a
    a
    a
    a

    b
    This list represents answers from five groups:

    The first group contains one person who answered "yes" to 3 questions: a, b, and c.
    The second group contains three people; combined, they answered "yes" to 3 questions: a, b, and c.
    The third group contains two people; combined, they answered "yes" to 3 questions: a, b, and c.
    The fourth group contains four people; combined, they answered "yes" to only 1 question, a.
    The last group contains one person who answered "yes" to only 1 question, b.
    In this example, the sum of these counts is 3 + 3 + 3 + 1 + 1 = 11.

    For each group, count the number of questions to which anyone answered "yes". What is the sum of those counts?
    */
    std::string solve_day_6_1(const fs::path& input_dir) {
        auto input = get_input(input_dir);
        std::vector<answers> group_answers;
        group_answers.reserve(input.size());
        std::transform(input.begin(), input.end(), std::back_inserter(group_answers), net_of_group);
        auto sum = std::accumulate(group_answers.begin(), group_answers.end(), 0ull,
                                   [](std::size_t acc, answers a){ return acc + a.count(); });
        return std::to_string(sum);
    }

    /*
    As you finish the last group's customs declaration, you notice that you misread one word in the instructions:

    You don't need to identify the questions to which anyone answered "yes"; you need to identify the questions to which everyone answered "yes"!

    Using the same example as above:

    abc

    a
    b
    c

    ab
    ac

    a
    a
    a
    a

    b
    This list represents answers from five groups:

    In the first group, everyone (all 1 person) answered "yes" to 3 questions: a, b, and c.
    In the second group, there is no question to which everyone answered "yes".
    In the third group, everyone answered yes to only 1 question, a. Since some people did not answer "yes" to b or c, they don't count.
    In the fourth group, everyone answered yes to only 1 question, a.
    In the fifth group, everyone (all 1 person) answered "yes" to 1 question, b.
    In this example, the sum of these counts is 3 + 0 + 1 + 1 + 1 = 6.

    For each group, count the number of questions to which everyone answered "yes". What is the sum of those counts?
    */
    std::string solve_day_6_2(const fs::path& input_dir) {
        auto input = get_input(input_dir);
        std::vector<answers> group_answers;
        group_answers.reserve(input.size());
        std::transform(input.begin(), input.end(), std::back_inserter(group_answers), all_of_group);
        auto sum = std::accumulate(group_answers.begin(), group_answers.end(), 0ull,
                                   [](std::size_t acc, answers a){ return acc + a.count(); });
        return std::to_string(sum);
    }

    static aoc::registration r {2020, 6, solve_day_6_1, solve_day_6_2};

} /* namespace aoc2020 */