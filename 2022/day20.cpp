//
// Created by Dan on 12/20/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    int64_t parse_int(std::string_view s) { return parse<int64_t>(s); }

    std::vector<int64_t> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_20_input.txt");
        return lines | std::views::transform(parse_int) | to<std::vector<int64_t>>();
    }

    struct datum {
        int64_t value;
        int position;
    };

    std::vector<datum> to_data(const std::vector<int64_t>& nums) {
        std::vector<datum> retval;
        retval.reserve(nums.size());
        for (int i = 0; i < nums.size(); ++i) {
            retval.push_back({nums[i], i});
        }
        return retval;
    }

    void mix(std::vector<datum>& input) {
        //fmt::print("{}\n", join(input, ',', [](const datum& d){ return d.value; }));
        const auto size = static_cast<int64_t>(input.size());
        for (int i = 0; i < size; ++i) {
            const auto found = std::find_if(input.begin(), input.end(), [i](const datum& d){ return d.position == i; });
            if (found->value == 0) {
                //fmt::print("{}\n", join(input, ',', [](const datum& d){ return d.value; }));
                continue;
            }
            const auto old_idx = static_cast<int64_t>(std::distance(input.begin(), found));
            auto new_idx = old_idx + found->value;
            //while (new_idx <= 0) { new_idx += size - 1; }
            if (new_idx <= 0) {
                auto num = std::div(new_idx, size - 1);
                new_idx += (-num.quot + (num.rem != 0 ? 1 : 0)) * (size - 1);
            }
            if (new_idx >= size) { new_idx %= size - 1; }

            if (new_idx > old_idx) {
                std::rotate(input.begin() + old_idx, input.begin() + old_idx + 1, input.begin() + new_idx + 1);
            }
            else {
                std::rotate(input.begin() + new_idx, input.begin() + old_idx, input.begin() + old_idx + 1);
            }
            //fmt::print("{}\n", join(input, ',', [](const datum& d){ return d.value; }));
        }
    }

    /*
    --- Day 20: Grove Positioning System ---
    It's finally time to meet back up with the Elves. When you try to contact them, however, you get no reply. Perhaps you're out of range?

    You know they're headed to the grove where the star fruit grows, so if you can figure out where that is, you should be able to meet back up with them.

    Fortunately, your handheld device has a file (your puzzle input) that contains the grove's coordinates! Unfortunately, the file is encrypted - just in case the device were to fall into the wrong hands.

    Maybe you can decrypt it?

    When you were still back at the camp, you overheard some Elves talking about coordinate file encryption. The main operation involved in decrypting the file is called mixing.

    The encrypted file is a list of numbers. To mix the file, move each number forward or backward in the file a number of positions equal to the value of the number being moved. The list is circular, so moving a number off one end of the list wraps back around to the other end as if the ends were connected.

    For example, to move the 1 in a sequence like 4, 5, 6, 1, 7, 8, 9, the 1 moves one position forward: 4, 5, 6, 7, 1, 8, 9. To move the -2 in a sequence like 4, -2, 5, 6, 7, 8, 9, the -2 moves two positions backward, wrapping around: 4, 5, 6, 7, 8, -2, 9.

    The numbers should be moved in the order they originally appear in the encrypted file. Numbers moving around during the mixing process do not change the order in which the numbers are moved.

    Consider this encrypted file:

    1
    2
    -3
    3
    -2
    0
    4
    Mixing this file proceeds as follows:

    Initial arrangement:
    1, 2, -3, 3, -2, 0, 4

    1 moves between 2 and -3:
    2, 1, -3, 3, -2, 0, 4

    2 moves between -3 and 3:
    1, -3, 2, 3, -2, 0, 4

    -3 moves between -2 and 0:
    1, 2, 3, -2, -3, 0, 4

    3 moves between 0 and 4:
    1, 2, -2, -3, 0, 3, 4

    -2 moves between 4 and 1:
    1, 2, -3, 0, 3, 4, -2

    0 does not move:
    1, 2, -3, 0, 3, 4, -2

    4 moves between -3 and 0:
    1, 2, -3, 4, 0, 3, -2
    Then, the grove coordinates can be found by looking at the 1000th, 2000th, and 3000th numbers after the value 0, wrapping around the list as necessary. In the above example, the 1000th number after 0 is 4, the 2000th is -3, and the 3000th is 2; adding these together produces 3.

    Mix your encrypted file exactly once. What is the sum of the three numbers that form the grove coordinates?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = to_data(get_input(input_dir));
        mix(input);
        const auto found = std::find_if(input.begin(), input.end(), [](const datum& d){ return d.value == 0; });
        const auto idx = std::distance(input.begin(), found);
        return std::to_string(
                input[(idx + 1000) % input.size()].value +
                input[(idx + 2000) % input.size()].value +
                input[(idx + 3000) % input.size()].value);
    }

    /*
    --- Part Two ---
    The grove coordinate values seem nonsensical. While you ponder the mysteries of Elf encryption, you suddenly remember the rest of the decryption routine you overheard back at camp.

    First, you need to apply the decryption key, 811589153. Multiply each number by the decryption key before you begin; this will produce the actual list of numbers to mix.

    Second, you need to mix the list of numbers ten times. The order in which the numbers are mixed does not change during mixing; the numbers are still moved in the order they appeared in the original, pre-mixed list. (So, if -3 appears fourth in the original list of numbers to mix, -3 will be the fourth number to move during each round of mixing.)

    Using the same example as above:

    Initial arrangement:
    811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612

    After 1 round of mixing:
    0, -2434767459, 3246356612, -1623178306, 2434767459, 1623178306, 811589153

    After 2 rounds of mixing:
    0, 2434767459, 1623178306, 3246356612, -2434767459, -1623178306, 811589153

    After 3 rounds of mixing:
    0, 811589153, 2434767459, 3246356612, 1623178306, -1623178306, -2434767459

    After 4 rounds of mixing:
    0, 1623178306, -2434767459, 811589153, 2434767459, 3246356612, -1623178306

    After 5 rounds of mixing:
    0, 811589153, -1623178306, 1623178306, -2434767459, 3246356612, 2434767459

    After 6 rounds of mixing:
    0, 811589153, -1623178306, 3246356612, -2434767459, 1623178306, 2434767459

    After 7 rounds of mixing:
    0, -2434767459, 2434767459, 1623178306, -1623178306, 811589153, 3246356612

    After 8 rounds of mixing:
    0, 1623178306, 3246356612, 811589153, -2434767459, 2434767459, -1623178306

    After 9 rounds of mixing:
    0, 811589153, 1623178306, -2434767459, 3246356612, 2434767459, -1623178306

    After 10 rounds of mixing:
    0, -2434767459, 1623178306, 3246356612, -1623178306, 2434767459, 811589153
    The grove coordinates can still be found in the same way. Here, the 1000th number after 0 is 811589153, the 2000th is 2434767459, and the 3000th is -1623178306; adding these together produces 1623178306.

    Apply the decryption key and mix your encrypted file ten times. What is the sum of the three numbers that form the grove coordinates?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        auto raw = get_input(input_dir);
        std::for_each(raw.begin(), raw.end(), [](int64_t& v){ v *= 811589153ll; });
        auto input = to_data(raw);
        for (int i = 0; i < 10; ++i) {
            mix(input);
        }
        const auto found = std::find_if(input.begin(), input.end(), [](const datum& d){ return d.value == 0; });
        const auto idx = std::distance(input.begin(), found);
        return std::to_string(
                input[(idx + 1000) % input.size()].value +
                input[(idx + 2000) % input.size()].value +
                input[(idx + 3000) % input.size()].value);
    }

    aoc::registration r{2022, 20, part_1, part_2};

    TEST_SUITE("2022_day20") {
        TEST_CASE("2022_day20:example") {
            std::vector<int64_t> raw {1, 2, -3, 3, -2, 0, 4};
            auto input = to_data(raw);
            mix(input);
            //fmt::print("{}\n", join(input, ',', [](const datum& d){ return d.value; }));
            CHECK_EQ(input[0].value, 1);
            CHECK_EQ(input[1].value, 2);
            CHECK_EQ(input[2].value, -3);
            CHECK_EQ(input[3].value, 4);
            CHECK_EQ(input[4].value, 0);
            CHECK_EQ(input[5].value, 3);
            CHECK_EQ(input[6].value, -2);

            const auto found = std::find_if(input.begin(), input.end(), [](const datum& d){ return d.value == 0; });
            const auto idx = std::distance(input.begin(), found);
            const int64_t result =
                    input[(idx + 1000) % input.size()].value +
                    input[(idx + 2000) % input.size()].value +
                    input[(idx + 3000) % input.size()].value;
            CHECK_EQ(result, 3);

            std::for_each(raw.begin(), raw.end(), [](int64_t& v){ v *= 811589153ll; });
            input = to_data(raw);
            for (int i = 0; i < 10; ++i) {
                mix(input);
            }
            const auto found2 = std::find_if(input.begin(), input.end(), [](const datum& d){ return d.value == 0; });
            const auto idx2 = std::distance(input.begin(), found2);
            const int64_t result2 =
                    input[(idx2 + 1000) % input.size()].value +
                    input[(idx2 + 2000) % input.size()].value +
                    input[(idx2 + 3000) % input.size()].value;
            CHECK_EQ(result2, 1623178306ll);
        }
    }

}