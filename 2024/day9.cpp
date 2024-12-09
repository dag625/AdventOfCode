//
// Created by Dan on 12/9/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/9
     */

    constexpr int EMPTY_ID = -1;

    std::string get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_9_input.txt");
        return lines.front();
    }

    std::vector<int> expand(const std::string& s) {
        std::vector<int> retval;
        retval.reserve(s.size() * s.size());
        for (int i = 0; i < s.size(); ++i) {
            const int len = static_cast<int>(s[i] - '0');
            int id = EMPTY_ID;
            if (i % 2 == 0) {
                id = i / 2;
            }
            for (int j = 0; j < len; ++j) {
                retval.push_back(id);
            }
        }
        return retval;
    }

    void compact(std::vector<int>& disk) {
        int next_free = 0, next_data = static_cast<int>(disk.size()) - 1;
        while (disk[next_free] > EMPTY_ID) { ++next_free; }
        while (disk[next_data] == EMPTY_ID) { --next_data; }
        while (next_free < next_data) {
            std::swap(disk[next_free], disk[next_data]);
            while (next_free < disk.size() && disk[next_free] > EMPTY_ID) { ++next_free; }
            while (next_data >= 0 && disk[next_data] == EMPTY_ID) { --next_data; }
        }
    }

    std::pair<int, int> next_space(const std::vector<int>& disk, const int begin, const int min_size) {
        for (int start = begin; start < disk.size();) {
            if (disk[start] == EMPTY_ID) {
                int end = start + 1;
                for (; end < disk.size() && disk[end] == EMPTY_ID; ++end) {}
                if (end - start >= min_size) {
                    return {start, end};
                }
                else {
                    start = end;
                }
            }
            else {
                ++start;
            }
        }
        return {-1, -1};
    }

    void defrag(std::vector<int>& disk) {
        int next_free = 0, next_data = static_cast<int>(disk.size()) - 1;
        while (disk[next_free] > EMPTY_ID) { ++next_free; }
        while (disk[next_data] == EMPTY_ID) { --next_data; }
        while (next_free < next_data) {
            const int data = disk[next_data];
            int data_start = next_data;
            while (disk[data_start] == data) { --data_start; }
            ++next_data;
            ++data_start;
            const int data_size = next_data - data_start;
            const auto [empty_start, empty_end] = next_space(disk, next_free, data_size);
            if (empty_start >= 0 && empty_start < data_start) {
                for (int i = 0; i < data_size; ++i) {
                    std::swap(disk[data_start + i], disk[empty_start + i]);
                }
                next_data = data_start - 1;
                while (disk[next_data] == EMPTY_ID) { --next_data; }
                while (disk[next_free] > EMPTY_ID) { ++next_free; }
            }
            else {
                next_data = data_start - 1;
                while (disk[next_data] == EMPTY_ID) { --next_data; }
            }
        }
    }

    int64_t checksum(const std::vector<int>& disk) {
        int64_t retval = 0;
        for (int64_t i = 0; i < disk.size(); ++i) {
            if (disk[i] != EMPTY_ID) {
                retval += i * disk[i];
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto disk = expand(input);
        compact(disk);
        const auto res = checksum(disk);
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto disk = expand(input);
        defrag(disk);
        const auto res = checksum(disk);
        return std::to_string(res);
    }

    aoc::registration r{2024, 9, part_1, part_2};

    TEST_SUITE("2024_day09") {
        TEST_CASE("2024_day09:example") {
            const std::string input = "2333133121414131402";
            auto disk = expand(input);
            compact(disk);
            const auto res1 = checksum(disk);
            CHECK_EQ(res1, 1928);

            disk = expand(input);
            defrag(disk);
            const auto res2 = checksum(disk);
            CHECK_EQ(res2, 2858);
        }
    }

} /* namespace <anon> */