//
// Created by Dan on 12/21/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <string>
#include <string_view>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct monkey {
        std::string name{};
        std::string op1{};
        std::string op2{};
        std::optional<int64_t> value{};
        char op = '\0';

        monkey() = default;
        monkey(std::string_view n, int64_t val) : name{n}, value{val} {}
        monkey(std::string_view n, std::string_view o1, std::string_view o2, char o) : name{n}, op1{o1}, op2{o2}, op{o} {}

        std::weak_ordering operator<=>(const monkey& rhs) const noexcept {
            return name <=> rhs.name;
        }
        std::weak_ordering operator<=>(const std::string& rhs) const noexcept {
            return name <=> rhs;
        }
    };

    monkey parse_monkey(std::string_view s) {
        const auto parts = split(s, ' ');
        const auto name = parts[0].substr(0, 4);
        if (parts.size() == 2) {
            return {name, parse<int>(parts[1])};
        }
        else {
            return {name, parts[1], parts[3], parts[2].front()};
        }
    }

    std::vector<monkey> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(parse_monkey) | to<std::vector<monkey>>();
    }

    constexpr std::string_view ROOT_NAME{"root"};
    constexpr std::string_view HUMAN_NAME{"humn"};

    std::optional<int64_t> calculate(monkey& m, const std::vector<monkey>& all, std::vector<std::string>& needed) {
        if (m.value.has_value()) {
            return m.value.value();
        }
        else {
            const auto found1 = std::lower_bound(all.begin(), all.end(), m.op1);
            const auto found2 = std::lower_bound(all.begin(), all.end(), m.op2);
            bool good = true;
            if (!found1->value.has_value()) {
                good = false;
                needed.push_back(m.op1);
            }
            if (!found2->value.has_value()) {
                good = false;
                needed.push_back(m.op2);
            }
            if (good) {
                switch (m.op) {
                    case '+': m.value = *found1->value + *found2->value; break;
                    case '-': m.value = *found1->value - *found2->value; break;
                    case '*': m.value = *found1->value * *found2->value; break;
                    case '/': m.value = *found1->value / *found2->value; break;
                    default: break;
                }
                return m.value;
            }
            else {
                return std::nullopt;
            }
        }
    }

    int64_t find_from(std::string_view start_name, std::vector<monkey>& all) {
        std::vector<std::string> stack;
        stack.emplace_back(start_name);
        int64_t last_val = 0;
        while (!stack.empty()) {
            const auto m = std::lower_bound(all.begin(), all.end(), stack.back());
            const auto res = calculate(*m, all, stack);
            if (res) {
                last_val = *res;
                stack.pop_back();
            }
        }
        return last_val;
    }

    int64_t find_root(std::vector<monkey>& all) {
        return find_from(ROOT_NAME, all);
    }

    bool humn_on_op1_side(const monkey& root, const std::vector<monkey>& all) {
        std::vector<std::string> stack;
        stack.emplace_back(root.op1);
        while (!stack.empty()) {
            const auto m = std::lower_bound(all.begin(), all.end(), stack.back());
            stack.pop_back();
            if (m->value.has_value()) {
                if (m->name == HUMAN_NAME) {
                    return true;
                }
            }
            else {
                stack.push_back(m->op1);
                stack.push_back(m->op2);
            }
        }
        return false;
    }

    int64_t calc_non_humn_side(const monkey& root, std::vector<monkey>& all) {
        const bool on_op1 = humn_on_op1_side(root, all);
        const auto match_val = find_from(on_op1 ? root.op2 : root.op1, all);
        return match_val;
    }

    char invert_op(char op) {
        switch (op) {
            case '+': return '-';
            case '-': return '+';
            case '*': return '/';
            case '/': return '*';
            default: return op;
        }
    }

    std::vector<monkey> create_inverse(std::vector<monkey>& all, const int64_t root_val) {
        //Create a new list of monkeys that inverts the tree so that 'humn' is the root.  This tree
        //should be narrow because we can calculate most monkey values.
        std::vector<monkey> retval;
        std::string next_op {HUMAN_NAME};
        while (true) {
            auto found = std::find_if(all.begin(), all.end(),
                                      [&next_op](const monkey& m){ return m.op1 == next_op || m.op2 == next_op; });
            if (found->name == ROOT_NAME) {
                retval.emplace_back(next_op, root_val);
                break;
            }
            bool is_next_1 = found->op1 == next_op;
            next_op = found->name;
            std::string other_name;
            const char new_op = invert_op(found->op);
            if (is_next_1) {
                other_name = found->op2;
                find_from(found->op2, all);//Pre-calculate the other so we can just copy that node and not the whole sub-tree
                retval.emplace_back(found->op1, found->name, found->op2, new_op);
            }
            else {
                other_name = found->op1;
                find_from(found->op1, all);//Pre-calculate the other so we can just copy that node and not the whole sub-tree
                if (new_op == '+' || new_op == '*') {
                    retval.emplace_back(found->op2, found->op1, found->name, found->op);
                }
                else {
                    retval.emplace_back(found->op2, found->name, found->op1, new_op);
                }
            }
//            fmt::print("From '{} = {} {} {}' to '{} = {} {} {}'\n",
//                       found->name, found->op1, found->op, found->op2,
//                       retval.back().name, retval.back().op1, retval.back().op, retval.back().op2);
            auto found_other = std::lower_bound(all.begin(), all.end(), other_name);
            retval.push_back(*found_other);
        }
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    /*
    --- Day 21: Monkey Math ---
    The monkeys are back! You're worried they're going to try to steal your stuff again, but it seems like they're just holding their ground and making various monkey noises at you.

    Eventually, one of the elephants realizes you don't speak monkey and comes over to interpret. As it turns out, they overheard you talking about trying to find the grove; they can show you a shortcut if you answer their riddle.

    Each monkey is given a job: either to yell a specific number or to yell the result of a math operation. All of the number-yelling monkeys know their number from the start; however, the math operation monkeys need to wait for two other monkeys to yell a number, and those two other monkeys might also be waiting on other monkeys.

    Your job is to work out the number the monkey named root will yell before the monkeys figure it out themselves.

    For example:

    root: pppw + sjmn
    dbpl: 5
    cczh: sllz + lgvd
    zczc: 2
    ptdq: humn - dvpt
    dvpt: 3
    lfqf: 4
    humn: 5
    ljgn: 2
    sjmn: drzm * dbpl
    sllz: 4
    pppw: cczh / lfqf
    lgvd: ljgn * ptdq
    drzm: hmdt - zczc
    hmdt: 32
    Each line contains the name of a monkey, a colon, and then the job of that monkey:

    A lone number means the monkey's job is simply to yell that number.
    A job like aaaa + bbbb means the monkey waits for monkeys aaaa and bbbb to yell each of their numbers; the monkey then yells the sum of those two numbers.
    aaaa - bbbb means the monkey yells aaaa's number minus bbbb's number.
    Job aaaa * bbbb will yell aaaa's number multiplied by bbbb's number.
    Job aaaa / bbbb will yell aaaa's number divided by bbbb's number.
    So, in the above example, monkey drzm has to wait for monkeys hmdt and zczc to yell their numbers. Fortunately, both hmdt and zczc have jobs that involve simply yelling a single number, so they do this immediately: 32 and 2. Monkey drzm can then yell its number by finding 32 minus 2: 30.

    Then, monkey sjmn has one of its numbers (30, from monkey drzm), and already has its other number, 5, from dbpl. This allows it to yell its own number by finding 30 multiplied by 5: 150.

    This process continues until root yells a number: 152.

    However, your actual situation involves considerably more monkeys. What number will the monkey named root yell?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        std::sort(input.begin(), input.end());
        const auto root_val = find_root(input);
        return std::to_string(root_val);
    }

    /*
    --- Part Two ---
    Due to some kind of monkey-elephant-human mistranslation, you seem to have misunderstood a few key details about the riddle.

    First, you got the wrong job for the monkey named root; specifically, you got the wrong math operation. The correct operation for monkey root should be =, which means that it still listens for two numbers (from the same two monkeys as before), but now checks that the two numbers match.

    Second, you got the wrong monkey for the job starting with humn:. It isn't a monkey - it's you. Actually, you got the job wrong, too: you need to figure out what number you need to yell so that root's equality check passes. (The number that appears after humn: in your input is now irrelevant.)

    In the above example, the number you need to yell to pass root's equality test is 301. (This causes root to get the same number, 150, from both of its monkeys.)

    What number do you yell to pass root's equality test?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        std::sort(input.begin(), input.end());
        const auto root = std::lower_bound(input.begin(), input.end(), std::string{ROOT_NAME});
        const auto match_val = calc_non_humn_side(*root, input);
        auto inv = create_inverse(input, match_val);
        const auto humn_val = find_from(HUMAN_NAME, inv);
        return std::to_string(humn_val);
    }

    aoc::registration r{2022, 21, part_1, part_2};

    TEST_SUITE("2022_day21") {
        TEST_CASE("2022_day21:example") {
            std::vector<std::string> lines {
                    "root: pppw + sjmn",
                    "dbpl: 5",
                    "cczh: sllz + lgvd",
                    "zczc: 2",
                    "ptdq: humn - dvpt",
                    "dvpt: 3",
                    "lfqf: 4",
                    "humn: 5",
                    "ljgn: 2",
                    "sjmn: drzm * dbpl",
                    "sllz: 4",
                    "pppw: cczh / lfqf",
                    "lgvd: ljgn * ptdq",
                    "drzm: hmdt - zczc",
                    "hmdt: 32"
            };
            auto input = lines | std::views::transform(parse_monkey) | to<std::vector<monkey>>();
            std::sort(input.begin(), input.end());
            const auto root_val = find_root(input);
            CHECK_EQ(root_val, 152);

            input = lines | std::views::transform(parse_monkey) | to<std::vector<monkey>>();
            std::sort(input.begin(), input.end());
            const auto root = std::lower_bound(input.begin(), input.end(), std::string{ROOT_NAME});
            const auto match_val = calc_non_humn_side(*root, input);
            auto inv = create_inverse(input, match_val);
            const auto humn_val = find_from(HUMAN_NAME, inv);
            CHECK_EQ(humn_val, 301);
        }
    }

}