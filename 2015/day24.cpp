//
// Created by Dan on 11/27/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    int do_parse(const std::string& s) { return parse<int>(s); }

    std::vector<int> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(do_parse) | to<std::vector<int>>();
    }

    enum class size_res {
        correct,
        too_small,
        too_large
    };

    using iter = std::vector<int>::const_iterator;
    using grp_size = std::vector<int>::size_type;

    size_res right_size(const std::vector<int>& group, const int goal) noexcept {
        const auto sz = std::accumulate(group.begin(), group.end(), 0);
        if (sz < goal) {
            return size_res::too_small;
        }
        else if (sz > goal) {
            return size_res::too_large;
        }
        else {
            return size_res::correct;
        }
    }

    uint64_t quantum_entanglement(const std::vector<int>& group) noexcept {
        return std::accumulate(group.begin(), group.end(), 1ull, [](uint64_t prod, int v){ return prod * v; });
    }

    std::pair<grp_size, uint64_t> find_best(std::vector<int>& group, const iter& begin, const iter& end, const int goal, grp_size best_size, uint64_t best_qe, const grp_size max_size) {
        if (group.size() >= best_size || group.size() >= max_size) {
            return {best_size, best_qe};
        }
        group.push_back(0);
        for (auto current = begin; current != end; ++current) {
            group.back() = *current;
            const auto sz = right_size(group, goal);
            if (sz == size_res::correct) {
                const auto qe = quantum_entanglement(group);
                if (group.size() < best_size) {
                    best_size = group.size();
                    best_qe = qe;
                }
                else if (best_size == group.size() && qe < best_qe) {
                    best_qe = qe;
                }
            }
            else if (sz == size_res::too_small) {
                const auto [next_size, next_qe] = find_best(group, current + 1, end, goal, best_size, best_qe, max_size);
                if (next_size < best_size) {
                    best_size = next_size;
                    best_qe = next_qe;
                }
                else if (next_size == best_size && next_qe < best_qe) {
                    best_qe = next_qe;
                }
            }
        }
        group.pop_back();
        return {best_size, best_qe};
    }

    std::pair<grp_size, uint64_t> find_best(const std::vector<int>& values, const int goal, const int num_groups) {
        std::vector<int> group;
        return find_best(group, values.begin(), values.end(), goal, std::numeric_limits<grp_size>::max(), std::numeric_limits<uint64_t>::max(), values.size() / num_groups);
    }

    /*
    --- Day 24: It Hangs in the Balance ---
    It's Christmas Eve, and Santa is loading up the sleigh for this year's deliveries. However, there's one small problem: he can't get the sleigh to balance. If it isn't balanced, he can't defy physics, and nobody gets presents this year.

    No pressure.

    Santa has provided you a list of the weights of every package he needs to fit on the sleigh. The packages need to be split into three groups of exactly the same weight, and every package has to fit. The first group goes in the passenger compartment of the sleigh, and the second and third go in containers on either side. Only when all three groups weigh exactly the same amount will the sleigh be able to fly. Defying physics has rules, you know!

    Of course, that's not the only problem. The first group - the one going in the passenger compartment - needs as few packages as possible so that Santa has some legroom left over. It doesn't matter how many packages are in either of the other two groups, so long as all of the groups weigh the same.

    Furthermore, Santa tells you, if there are multiple ways to arrange the packages such that the fewest possible are in the first group, you need to choose the way where the first group has the smallest quantum entanglement to reduce the chance of any "complications". The quantum entanglement of a group of packages is the product of their weights, that is, the value you get when you multiply their weights together. Only consider quantum entanglement if the first group has the fewest possible number of packages in it and all groups weigh the same amount.

    For example, suppose you have ten packages with weights 1 through 5 and 7 through 11. For this situation, some of the unique first groups, their quantum entanglements, and a way to divide the remaining packages are as follows:

    Group 1;             Group 2; Group 3
    11 9       (QE= 99); 10 8 2;  7 5 4 3 1
    10 9 1     (QE= 90); 11 7 2;  8 5 4 3
    10 8 2     (QE=160); 11 9;    7 5 4 3 1
    10 7 3     (QE=210); 11 9;    8 5 4 2 1
    10 5 4 1   (QE=200); 11 9;    8 7 3 2
    10 5 3 2   (QE=300); 11 9;    8 7 4 1
    10 4 3 2 1 (QE=240); 11 9;    8 7 5
    9 8 3      (QE=216); 11 7 2;  10 5 4 1
    9 7 4      (QE=252); 11 8 1;  10 5 3 2
    9 5 4 2    (QE=360); 11 8 1;  10 7 3
    8 7 5      (QE=280); 11 9;    10 4 3 2 1
    8 5 4 3    (QE=480); 11 9;    10 7 2 1
    7 5 4 3 1  (QE=420); 11 9;    10 8 2
    Of these, although 10 9 1 has the smallest quantum entanglement (90), the configuration with only two packages, 11 9, in the passenger compartment gives Santa the most legroom and wins. In this situation, the quantum entanglement for the ideal configuration is therefore 99. Had there been two configurations with only two packages in the first group, the one with the smaller quantum entanglement would be chosen.

    What is the quantum entanglement of the first group of packages in the ideal configuration?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const int num_groups = 3;
        int sum = 0;
        for (auto v : input) { sum += v; }
        const int goal = sum / num_groups;
        const auto [best_size, best_qe] = find_best(input, goal, num_groups);
        return std::to_string(best_qe);
    }

    /*
    --- Part Two ---
    That's weird... the sleigh still isn't balancing.

    "Ho ho ho", Santa muses to himself. "I forgot the trunk".

    Balance the sleigh again, but this time, separate the packages into four groups instead of three. The other constraints still apply.

    Given the example packages above, this would be some of the new unique first groups, their quantum entanglements, and one way to divide the remaining packages:


    11 4    (QE=44); 10 5;   9 3 2 1; 8 7
    10 5    (QE=50); 11 4;   9 3 2 1; 8 7
    9 5 1   (QE=45); 11 4;   10 3 2;  8 7
    9 4 2   (QE=72); 11 3 1; 10 5;    8 7
    9 3 2 1 (QE=54); 11 4;   10 5;    8 7
    8 7     (QE=56); 11 4;   10 5;    9 3 2 1
    Of these, there are three arrangements that put the minimum (two) number of packages in the first group: 11 4, 10 5, and 8 7. Of these, 11 4 has the lowest quantum entanglement, and so it is selected.

    Now, what is the quantum entanglement of the first group of packages in the ideal configuration?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const int num_groups = 4;
        int sum = 0;
        for (auto v : input) { sum += v; }
        const int goal = sum / num_groups;
        const auto [best_size, best_qe] = find_best(input, goal, num_groups);
        return std::to_string(best_qe);
    }

    aoc::registration r{2015, 24, part_1, part_2};

//    TEST_SUITE("2015_day24") {
//        TEST_CASE("2015_day24:example") {
//
//        }
//    }

}