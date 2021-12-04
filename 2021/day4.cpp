//
// Created by Dan on 12/4/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <string_view>
#include <ranges>
#include <numeric>
#include <iostream>

#include "utilities.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct bingo_square {
        int16_t num = 0;
        bool called = false;
        bingo_square() = default;
        bingo_square(int val) : num{static_cast<int16_t>(val)} {}

        bingo_square& operator=(int val) { num = val; return *this; }
        bool check(int val) { if (num == val) { called = true; } return called; }
    };

    std::ostream& operator<<(std::ostream& os, const bingo_square& s) {
        std::stringstream tmp;
        if (s.called) {
            tmp << 'x';
        }
        tmp << s.num;
        os << tmp.str();
        return os;
    }

    class bingo_board {
        grid<bingo_square> board;

        static grid<bingo_square> from_numbers(const std::vector<int>& nums) {
            std::vector<bingo_square> squares;
            squares.reserve(nums.size());
            std::ranges::transform(nums, std::back_inserter(squares), [](int v){ return bingo_square{v}; });
            return grid<bingo_square>{std::move(squares), 5};
        }

    public:
        bingo_board() : board{5,5} {}
        bingo_board(const std::vector<int>& nums) : board{from_numbers(nums)} {}

        bool have_bingo(int val) {
            for (int r = 0; r < board.num_rows(); ++r) {
                auto row = board.row_span(r);
                if (std::accumulate(row.begin(), row.end(), true, [val](bool have, bingo_square& s){ return s.check(val) && have; })) {
                    return true;
                }
            }
            for (int c = 0; c < board.num_cols(); ++c) {
                auto col = board.column_span(c);
                if (std::accumulate(col.begin(), col.end(), true, [val](bool have, bingo_square& s){ return s.check(val) && have; })) {
                    return true;
                }
            }
            return false;
        }

        bool have_bingo() {
            for (int r = 0; r < board.num_rows(); ++r) {
                auto row = board.row_span(r);
                if (std::accumulate(row.begin(), row.end(), true, [](bool have, bingo_square& s){ return s.called && have; })) {
                    return true;
                }
            }
            for (int c = 0; c < board.num_cols(); ++c) {
                auto col = board.column_span(c);
                if (std::accumulate(col.begin(), col.end(), true, [](bool have, bingo_square& s){ return s.called && have; })) {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] int sum_unmarked() const {
            return std::accumulate(board.begin(), board.end(), 0,
                                   [](int val, const bingo_square& s){ return val + (!s.called ? s.num : 0); });
        }

        std::ostream& display(std::ostream& os) const {
            board.display(os);
            return os;
        }
    };

    struct info {
        std::vector<int> numbers;
        std::vector<bingo_board> boards;
    };

    info get_input(const fs::path& input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_4_input.txt");
        const auto num_strs = split(lines.front(), ',');

        const auto to_int = [](std::string_view s){
            int val = 0;
            auto res = std::from_chars(s.data(), s.data() + s.size(), val);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            return val;
        };

        info retval;
        retval.numbers.reserve(num_strs.size());
        std::ranges::transform(num_strs, std::back_inserter(retval.numbers), to_int);

        for (auto blank = lines.begin() + 1; blank != lines.end(); blank += 6) {
            std::vector<std::string_view> square_strs;
            square_strs.reserve(25);
            for (auto current = blank + 1; current != blank + 6; ++current) {
                auto nums = split_no_empty(*current, ' ');
                square_strs.insert(square_strs.end(), nums.begin(), nums.end());
            }

            std::vector<int> squares;
            squares.reserve(square_strs.size());
            std::ranges::transform(square_strs, std::back_inserter(squares), to_int);
            retval.boards.emplace_back(squares);
        }
        return retval;
    }

    /*
    --- Day 4: Giant Squid ---
    You're already almost 1.5km (almost a mile) below the surface of the ocean, already so deep that you can't see any sunlight. What you can see, however, is a giant squid that has attached itself to the outside of your submarine.

    Maybe it wants to play bingo?

    Bingo is played on a set of boards each consisting of a 5x5 grid of numbers. Numbers are chosen at random, and the chosen number is marked on all boards on which it appears. (Numbers may not appear on all boards.) If all numbers in any row or any column of a board are marked, that board wins. (Diagonals don't count.)

    The submarine has a bingo subsystem to help passengers (currently, you and the giant squid) pass the time. It automatically generates a random order in which to draw numbers and a random set of boards (your puzzle input). For example:

    7,4,9,5,11,17,23,2,0,14,21,24,10,16,13,6,15,25,12,22,18,20,8,19,3,26,1

    22 13 17 11  0
     8  2 23  4 24
    21  9 14 16  7
     6 10  3 18  5
     1 12 20 15 19

     3 15  0  2 22
     9 18 13 17  5
    19  8  7 25 23
    20 11 10 24  4
    14 21 16 12  6

    14 21 17 24  4
    10 16 15  9 19
    18  8 23 26 20
    22 11 13  6  5
     2  0 12  3  7
    After the first five numbers are drawn (7, 4, 9, 5, and 11), there are no winners, but the boards are marked as follows (shown here adjacent to each other to save space):

    22 13 17 11  0         3 15  0  2 22        14 21 17 24  4
     8  2 23  4 24         9 18 13 17  5        10 16 15  9 19
    21  9 14 16  7        19  8  7 25 23        18  8 23 26 20
     6 10  3 18  5        20 11 10 24  4        22 11 13  6  5
     1 12 20 15 19        14 21 16 12  6         2  0 12  3  7
    After the next six numbers are drawn (17, 23, 2, 0, 14, and 21), there are still no winners:

    22 13 17 11  0         3 15  0  2 22        14 21 17 24  4
     8  2 23  4 24         9 18 13 17  5        10 16 15  9 19
    21  9 14 16  7        19  8  7 25 23        18  8 23 26 20
     6 10  3 18  5        20 11 10 24  4        22 11 13  6  5
     1 12 20 15 19        14 21 16 12  6         2  0 12  3  7
    Finally, 24 is drawn:

    22 13 17 11  0         3 15  0  2 22        14 21 17 24  4
     8  2 23  4 24         9 18 13 17  5        10 16 15  9 19
    21  9 14 16  7        19  8  7 25 23        18  8 23 26 20
     6 10  3 18  5        20 11 10 24  4        22 11 13  6  5
     1 12 20 15 19        14 21 16 12  6         2  0 12  3  7
    At this point, the third board wins because it has at least one complete row or column of marked numbers (in this case, the entire top row is marked: 14 21 17 24 4).

    The score of the winning board can now be calculated. Start by finding the sum of all unmarked numbers on that board; in this case, the sum is 188. Then, multiply that sum by the number that was just called when the board won, 24, to get the final score, 188 * 24 = 4512.

    To guarantee victory against the giant squid, figure out which board will win first. What will your final score be if you choose that board?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        auto input = get_input(input_dir);
        for (const auto val : input.numbers) {
            for (auto& b : input.boards) {
                if (b.have_bingo(val)) {
                    return std::to_string(val * b.sum_unmarked());
                }
            }
        }
        return std::to_string(-1);
    }

    /*
    --- Part Two ---
    On the other hand, it might be wise to try a different strategy: let the giant squid win.

    You aren't sure how many bingo boards a giant squid could play at once, so rather than waste time counting its arms, the safe thing to do is to figure out which board will win last and choose that one. That way, no matter which boards it picks, it will win for sure.

    In the above example, the second board is the last to win, which happens after 13 is eventually called and its middle column is completely marked. If you were to keep playing until this point, the second board would have a sum of unmarked numbers equal to 148 for a final score of 148 * 13 = 1924.

    Figure out which board will win last. Once it wins, what would its final score be?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto input = get_input(input_dir);
        for (const auto val : input.numbers) {
            for (auto b = input.boards.begin(); b != input.boards.end();) {
                if (b->have_bingo(val)) {
                    if (input.boards.size() == 1) {
                        return std::to_string(val * b->sum_unmarked());
                    }
                    else {
                        b = input.boards.erase(b);
                    }
                }
                else {
                    ++b;
                }
            }
        }
        return std::to_string(-1);
    }

    aoc::registration r {2021, 4, part_1, part_2};

    TEST_SUITE("2021_day4") {
        TEST_CASE("2021_day4:example") {
            std::vector<int> nums = {14, 21, 17, 24,  4,
                    10, 16, 15,  9, 19,
                    18,  8, 23, 26, 20,
                    22, 11, 13,  6,  5,
                    2,  0, 12,  3,  7};
            std::vector<int> called = {7,4,9,5,11,17,23,2,0,14,21,24,10,16,13,6,15,25,12,22,18,20,8,19,3,26,1};
            bingo_board board {nums};
            int bingo_num = -1;
            for (const auto val : called) {
                bingo_num = val;
                if (board.have_bingo(val)) {
                    break;
                }
            }
            REQUIRE_EQ(bingo_num, 24);
            REQUIRE_EQ(board.sum_unmarked(), 188);
        }
    }
}