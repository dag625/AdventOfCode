//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <algorithm>
#include <array>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/21
     */

    struct swap_idx_ins {
        int idx1 = 0;
        int idx2 = 0;

        void run(std::string& s) const {
            std::swap(s[idx1], s[idx2]);
        }

        void reverse(std::string& s) const {
            std::swap(s[idx1], s[idx2]);
        }
    };

    swap_idx_ins parse_swap_idx_ins(std::string_view s) {
        const auto p = split(s, ' ');
        return {parse<int>(p[2]), parse<int>(p[5])};
    }

    struct swap_char_ins {
        char letter1 = '\0';
        char letter2 = '\0';

        void run(std::string& s) const {
            const auto idx1 = s.find(letter1);
            const auto idx2 = s.find(letter2);
            std::swap(s[idx1], s[idx2]);
        }

        void reverse(std::string& s) const {
            const auto idx1 = s.find(letter1);
            const auto idx2 = s.find(letter2);
            std::swap(s[idx1], s[idx2]);
        }
    };

    swap_char_ins parse_swap_char_ins(std::string_view s) {
        const auto p = split(s, ' ');
        return {p[2][0], p[5][0]};
    }

    struct rot_steps_ins {
        int steps = 0;
        bool right = false;

        void run(std::string& s) const {
            if (right) {
                std::rotate(s.begin(), s.end() - steps, s.end());
            }
            else {
                std::rotate(s.begin(), s.begin() + steps, s.end());
            }
        }

        void reverse(std::string& s) const {
            if (right) {
                std::rotate(s.begin(), s.begin() + steps, s.end());
            }
            else {
                std::rotate(s.begin(), s.end() - steps, s.end());
            }
        }
    };

    rot_steps_ins parse_rot_steps_ins(std::string_view s) {
        const auto p = split(s, ' ');
        return {parse<int>(p[2]), p[1] == "right"};
    }

    struct rot_char_ins {
        char letter = '\0';

        /*
         * Position change map:
         * Original     Final
         * 0            1
         * 1            3
         * 2            5
         * 3            7
         * 4            2
         * 5            4
         * 6            6
         * 7            0
         */

        void run(std::string& s) const {
            const auto idx = s.find(letter);
            const auto times = static_cast<std::string::difference_type>((idx + 1 + (idx >= 4 ? 1 : 0)) % s.size());
            std::rotate(s.begin(), s.end() - times, s.end());
        }

        void reverse(std::string& s) const {
            static constexpr std::array<int, 8> to_orig {7, 0, 4, 1, 5, 2, 6, 3};
            const auto idx = s.find(letter);
            const auto orig_idx = to_orig[idx];
            const auto times = static_cast<std::string::difference_type>((orig_idx + 1 + (orig_idx >= 4 ? 1 : 0)) % s.size());
            std::rotate(s.begin(), s.begin() + times, s.end());
        }
    };

    rot_char_ins parse_rot_char_ins(std::string_view s) {
        const auto p = split(s, ' ');
        return {p[6][0]};
    }

    struct reverse_ins {
        int idx1 = 0;
        int idx2 = 0;

        void run(std::string& s) const {
            std::reverse(s.begin() + idx1, s.begin() + idx2 + 1);
        }

        void reverse(std::string& s) const {
            std::reverse(s.begin() + idx1, s.begin() + idx2 + 1);
        }
    };

    reverse_ins parse_reverse_ins(std::string_view s) {
        const auto p = split(s, ' ');
        return {parse<int>(p[2]), parse<int>(p[4])};
    }

    struct move_ins {
        int idx1 = 0;
        int idx2 = 0;

        void run(std::string& s) const {
            const char c = s[idx1];
            s.erase(idx1, 1);
            s.insert(s.begin() + idx2, c);
        }

        void reverse(std::string& s) const {
            const char c = s[idx2];
            s.erase(idx2, 1);
            s.insert(s.begin() + idx1, c);
        }
    };

    move_ins parse_move_ins(std::string_view s) {
        const auto p = split(s, ' ');
        return {parse<int>(p[2]), parse<int>(p[5])};
    }

    using instruction = std::variant<swap_idx_ins, swap_char_ins, rot_steps_ins, rot_char_ins, reverse_ins, move_ins>;

    instruction parse_ins(std::string_view s) {
        if (s.starts_with("swap position")) {
            return parse_swap_idx_ins(s);
        }
        else if (s.starts_with("swap letter")) {
            return parse_swap_char_ins(s);
        }
        //Order of next two is important
        else if (s.starts_with("rotate based")) {
            return parse_rot_char_ins(s);
        }
        else if (s.starts_with("rotate")) {
            return parse_rot_steps_ins(s);
        }
        else if (s.starts_with("reverse")) {
            return parse_reverse_ins(s);
        }
        else if (s.starts_with("move")) {
            return parse_move_ins(s);
        }
        else {
            throw std::runtime_error{fmt::format("Unrecognized instruction:  {}", s)};
        }
    }

    std::vector<instruction> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_ins) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::string pass = "abcdefgh";
        for (const auto& ins : input) {
            std::visit([&pass](const auto& i){ i.run(pass); }, ins);
        }
        return pass;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::string pass = "fbgdceah";
        for (const auto& ins : input | std::views::reverse) {
            std::visit([&pass](const auto& i){ i.reverse(pass); }, ins);
        }
        return pass;
    }

    aoc::registration r{2016, 21, part_1, part_2};

//    TEST_SUITE("2016_day21") {
//        TEST_CASE("2016_day21:example") {
//
//        }
//    }

} /* namespace <anon> */