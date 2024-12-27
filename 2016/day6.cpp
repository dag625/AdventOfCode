//
// Created by Dan on 11/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct char_info {
        char val = '\0';
        int count = 0;

        char_info(char c) : val{c} {}

        bool operator<(const char_info& rhs) const noexcept {
            //We subtract from the count, so the smallest is the most numerous, and then we sort by the character.
            return std::tie(count, val) < std::tie(rhs.count, rhs.val);
        }
    };

    std::vector<char_info> init_info() {
        return std::views::iota('a', '{') | std::views::transform([](char c){ return char_info{c}; }) | to<std::vector<char_info>>();
    }

    std::vector<std::string> get_input(const std::vector<std::string>& lines) {
        return lines;
    }

    /*
    --- Day 6: Signals and Noise ---
    Something is jamming your communications with Santa. Fortunately, your signal is only partially jammed, and protocol in situations like this is to switch to a simple repetition code to get the message through.

    In this model, the same message is sent repeatedly. You've recorded the repeating message signal (your puzzle input), but the data seems quite corrupted - almost too badly to recover. Almost.

    All you need to do is figure out which character is most frequent for each position. For example, suppose you had recorded the following messages:

    eedadn
    drvtee
    eandsr
    raavrd
    atevrs
    tsrnev
    sdttsa
    rasrtv
    nssdts
    ntnada
    svetve
    tesnvt
    vntsnd
    vrdear
    dvrsen
    enarar
    The most common character in the first column is e; in the second, a; in the third, s, and so on. Combining these characters returns the error-corrected message, easter.

    Given the recording in your puzzle input, what is the error-corrected version of the message being sent?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::array<std::vector<char_info>, 8> data{};
        data[0] = init_info();
        data[1] = init_info();
        data[2] = init_info();
        data[3] = init_info();
        data[4] = init_info();
        data[5] = init_info();
        data[6] = init_info();
        data[7] = init_info();
        for (const auto& l : input) {
            for (int i = 0; i < 8; ++i) {
                --data[i][static_cast<int>(l[i] - 'a')].count;
            }
        }
        std::string result;
        for (auto& d : data) {
            std::sort(d.begin(), d.end());
            result.push_back(d.front().val);
        }
        return result;
    }

    /*
    --- Part Two ---
    Of course, that would be the message - if you hadn't agreed to use a modified repetition code instead.

    In this modified code, the sender instead transmits what looks like random data, but for each character, the character they actually want to send is slightly less likely than the others. Even after signal-jamming noise, you can look at the letter distributions in each column and choose the least common letter to reconstruct the original message.

    In the above example, the least common character in the first column is a; in the second, d, and so on. Repeating this process for the remaining characters produces the original message, advent.

    Given the recording in your puzzle input and this new decoding methodology, what is the original message that Santa is trying to send?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::array<std::vector<char_info>, 8> data{};
        data[0] = init_info();
        data[1] = init_info();
        data[2] = init_info();
        data[3] = init_info();
        data[4] = init_info();
        data[5] = init_info();
        data[6] = init_info();
        data[7] = init_info();
        for (const auto& l : input) {
            for (int i = 0; i < 8; ++i) {
                ++data[i][static_cast<int>(l[i] - 'a')].count;
            }
        }
        std::string result;
        for (auto& d : data) {
            std::sort(d.begin(), d.end());
            result.push_back(d.front().val);
        }
        return result;
    }

    aoc::registration r{2016, 6, part_1, part_2};

//    TEST_SUITE("2016_day6") {
//        TEST_CASE("2016_day6:example") {
//
//        }
//    }

}