//
// Created by Daniel Garcia on 12/5/20.
//

#include "day5.h"
#include "utilities.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <iostream>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        struct seat {
            uint8_t row;
            uint8_t col;

            [[nodiscard]] int seat_id() const { return static_cast<int>(row) * 8 + col; }
        };

        seat parse_seat(const std::string& s) {
            if (s.size() != 10) {
                throw std::runtime_error{"Boarding pass has incorrect length."};
            }
            std::string_view row_str {s.data(), 7}, col_str {s.data() + 7, 3};
            if (row_str.find_first_not_of("FB") != std::string_view::npos) {
                throw std::runtime_error{"Boarding pass has invalid row character."};
            }
            if (col_str.find_first_not_of("RL") != std::string_view::npos) {
                throw std::runtime_error{"Boarding pass has invalid column character."};
            }
            uint8_t row = 0, col = 0;
            constexpr char row_denom = 'B' - 'F';
            for (auto c : row_str) {
                row <<= 1;
                row |= (c - 'F') / row_denom;
            }
            constexpr char col_denom = 'R' - 'L';
            for (auto c : col_str) {
                col <<= 1;
                col |= (c - 'L') / col_denom;
            }
            return {row, col};
        }

        std::vector<seat> get_input(const fs::path& input_dir) {
            auto lines = read_file_lines(input_dir / "2020" / "day_5_input.txt");
            std::vector<seat> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_seat);
            std::sort(retval.begin(), retval.end(), [](seat a, seat b){ return a.seat_id() < b.seat_id(); });
            return retval;
        }

    }

    /*
    You board your plane only to discover a new problem: you dropped your boarding pass! You aren't sure which seat is yours, and all of the flight attendants are busy with the flood of people that suddenly made it through passport control.

    You write a quick program to use your phone's camera to scan all of the nearby boarding passes (your puzzle input); perhaps you can find your seat through process of elimination.

    Instead of zones or groups, this airline uses binary space partitioning to seat people. A seat might be specified like FBFBBFFRLR, where F means "front", B means "back", L means "left", and R means "right".

    The first 7 characters will either be F or B; these specify exactly one of the 128 rows on the plane (numbered 0 through 127). Each letter tells you which half of a region the given seat is in. Start with the whole list of rows; the first letter indicates whether the seat is in the front (0 through 63) or the back (64 through 127). The next letter indicates which half of that region the seat is in, and so on until you're left with exactly one row.

    For example, consider just the first seven characters of FBFBBFFRLR:

    Start by considering the whole range, rows 0 through 127.
    F means to take the lower half, keeping rows 0 through 63.
    B means to take the upper half, keeping rows 32 through 63.
    F means to take the lower half, keeping rows 32 through 47.
    B means to take the upper half, keeping rows 40 through 47.
    B keeps rows 44 through 47.
    F keeps rows 44 through 45.
    The final F keeps the lower of the two, row 44.
    The last three characters will be either L or R; these specify exactly one of the 8 columns of seats on the plane (numbered 0 through 7). The same process as above proceeds again, this time with only three steps. L means to keep the lower half, while R means to keep the upper half.

    For example, consider just the last 3 characters of FBFBBFFRLR:

    Start by considering the whole range, columns 0 through 7.
    R means to take the upper half, keeping columns 4 through 7.
    L means to take the lower half, keeping columns 4 through 5.
    The final R keeps the upper of the two, column 5.
    So, decoding FBFBBFFRLR reveals that it is the seat at row 44, column 5.

    Every seat also has a unique seat ID: multiply the row by 8, then add the column. In this example, the seat has ID 44 * 8 + 5 = 357.

    Here are some other boarding passes:

    BFFFBBFRRR: row 70, column 7, seat ID 567.
    FFFBBBFRRR: row 14, column 7, seat ID 119.
    BBFFBBFRLL: row 102, column 4, seat ID 820.
    As a sanity check, look through your list of boarding passes. What is the highest seat ID on a boarding pass?
    */
    void solve_day_5_1(const fs::path& input_dir) {
        auto seats = get_input(input_dir);
        std::cout << '\t' << seats.back().seat_id() << '\n';
    }

    /*
    Ding! The "fasten seat belt" signs have turned on. Time to find your seat.

    It's a completely full flight, so your seat should be the only missing boarding pass in your list. However, there's a catch: some of the seats at the very front and back of the plane don't exist on this aircraft, so they'll be missing from your list as well.

    Your seat wasn't at the very front or back, though; the seats with IDs +1 and -1 from yours will be in your list.

    What is the ID of your seat?
    */
    void solve_day_5_2(const fs::path& input_dir) {
        auto seats = get_input(input_dir);
        auto next_id = seats.front().seat_id();
        for (const auto s : seats) {
            if (s.seat_id() != next_id) {
                break;
            }
            ++next_id;
        }
        std::cout << '\t' << next_id << '\n';
    }

} /* namespace aoc2020 */