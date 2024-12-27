//
// Created by Dan on 11/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct data {
        std::string text;
        int id = 0;
        std::string checksum;
    };

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

    data parse_data(std::string_view s) {
        const auto id_start = s.find_last_of('-') + 1;
        const auto chk_start = s.find_last_of('[') + 1;
        const auto id_str = s.substr(id_start);
        return { std::string{s.substr(0, id_start)}, parse<int>(id_str), std::string{s.substr(chk_start, 5)} };
    }

    std::vector<data> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_data) | to<std::vector<data>>();
    }

    std::string calculate_checksum(std::string_view text) {
        auto counts = init_info();
        for (const char c : text) {
            if (c != '-') {
                --counts[c - 'a'].count;
            }
        }
        std::sort(counts.begin(), counts.end());
        return counts | std::views::take(5) | std::views::transform([](const char_info& c){ return c.val; }) | to<std::string>();
    }

    std::string decrypt(const data& d) {
        std::string retval;
        const int rot = d.id % 26;
        for (const char c : d.text) {
            if (c == '-') {
                retval.push_back(' ');
            }
            else {
                const int val = static_cast<int>(c) - 'a';
                retval.push_back(static_cast<char>('a' + (val + rot) % 26));
            }
        }
        return retval;
    }

    /*
    --- Day 4: Security Through Obscurity ---
    Finally, you come across an information kiosk with a list of rooms. Of course, the list is encrypted and full of decoy data, but the instructions to decode the list are barely hidden nearby. Better remove the decoy data first.

    Each room consists of an encrypted name (lowercase letters separated by dashes) followed by a dash, a sector ID, and a checksum in square brackets.

    A room is real (not a decoy) if the checksum is the five most common letters in the encrypted name, in order, with ties broken by alphabetization. For example:

    aaaaa-bbb-z-y-x-123[abxyz] is a real room because the most common letters are a (5), b (3), and then a tie between x, y, and z, which are listed alphabetically.
    a-b-c-d-e-f-g-h-987[abcde] is a real room because although the letters are all tied (1 of each), the first five are listed alphabetically.
    not-a-real-room-404[oarel] is a real room.
    totally-real-room-200[decoy] is not.
    Of the real rooms from the list above, the sum of their sector IDs is 1514.

    What is the sum of the sector IDs of the real rooms?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t sum = 0;
        for (const auto& d : input) {
            const auto cs = calculate_checksum(d.text);
            if (cs == d.checksum) {
                sum += static_cast<int64_t>(d.id);
            }
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    With all the decoy data out of the way, it's time to decrypt this list and get moving.

    The room names are encrypted by a state-of-the-art shift cipher, which is nearly unbreakable without the right software. However, the information kiosk designers at Easter Bunny HQ were not expecting to deal with a master cryptographer like yourself.

    To decrypt a room name, rotate each letter forward through the alphabet a number of times equal to the room's sector ID. A becomes B, B becomes C, Z becomes A, and so on. Dashes become spaces.

    For example, the real name for qzmt-zixmtkozy-ivhz-343 is very encrypted name.

    What is the sector ID of the room where North Pole objects are stored?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int id = -1;
        for (const auto& d : input) {
            const auto cs = calculate_checksum(d.text);
            /*
             * This is a bit cheaty, but the puzzle doesn't specify the name to look for exactly.  So I initially
             * decrypted and printed out all valid data, and there was only one entry with the text 'north' or
             * 'northpole' in it, so I use this as the condition.  The exact full name for my input was something
             * like 'northpole object storage'.
             */
            if (cs == d.checksum && decrypt(d).contains("northpole")) {
                id = d.id;
                break;
            }
        }
        return std::to_string(id);
    }

    aoc::registration r{2016, 4, part_1, part_2};

//    TEST_SUITE("2016_day4") {
//        TEST_CASE("2016_day4:example") {
//
//        }
//    }

}