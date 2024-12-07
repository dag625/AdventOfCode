//
// Created by Dan on 12/6/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <array>

#include "utilities.h"
#include "openssl_hash.h"
#include "grid.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/17
     */

    std::string get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_17_input.txt");
        return lines.front();
    }

    std::string calc_hash(openssl_hash& h, const std::string& pass, const std::string& path) {
        const auto secret = fmt::format("{}{}", pass, path);
        h.reinit();
        h.add(secret);
        return h.finalizeHex();
    }

    bool char_to_door(const char c) {
        return c > 'a';
    }

    std::array<bool, 4> hash_to_doors(std::string_view h) {
        return {char_to_door(h[0]), char_to_door(h[1]), char_to_door(h[2]),char_to_door(h[3])};
    }

    velocity to_vel(char c) {
        switch (c) {
            case 'U': return {-1, 0};
            case 'D': return {1, 0};
            case 'L': return {0, -1};
            case 'R': return {0, 1};
            default: return {};
        }
    }

    position path_to_pos(std::string_view p) {
        position pos{}, z;
        for (const char c : p) {
            pos += to_vel(c);
        }
        return pos;
    }

    bool is_in(position p) {
        return p.x >= 0 && p.x < 4 && p.y >= 0 && p.y < 4;
    }

    struct cell {
        std::string path;
        position pos;
    };

    std::vector<cell> next_paths(openssl_hash& h, const std::string& pass, const std::string& current_path) {
        const auto doors = hash_to_doors(calc_hash(h, pass, current_path));
        std::vector<cell> retval;
        retval.reserve(doors.size());
        if (doors[0]) {
            auto np = current_path + 'U';
            const auto pos = path_to_pos(np);
            if (is_in(pos)) {
                retval.emplace_back(std::move(np), pos);
            }
        }
        if (doors[1]) {
            auto np = current_path + 'D';
            const auto pos = path_to_pos(np);
            if (is_in(pos)) {
                retval.emplace_back(std::move(np), pos);
            }
        }
        if (doors[2]) {
            auto np = current_path + 'L';
            const auto pos = path_to_pos(np);
            if (is_in(pos)) {
                retval.emplace_back(std::move(np), pos);
            }
        }
        if (doors[3]) {
            auto np = current_path + 'R';
            const auto pos = path_to_pos(np);
            if (is_in(pos)) {
                retval.emplace_back(std::move(np), pos);
            }
        }
        return retval;
    }

    std::string shortest_path(const std::string& passcode, const position dest) {
        openssl_hash h {EVP_md5()};

        //Using less will create a max heap; we want a min heap.
        const auto heap_cmp = [](std::string_view a, std::string_view b){ return a.size() > b.size(); };

        std::vector<std::string> queue;
        queue.emplace_back();
        while (!queue.empty()) {
            const auto current = queue.front();
            std::pop_heap(queue.begin(), queue.end(), heap_cmp);
            queue.erase(queue.end() - 1);

            const auto next = next_paths(h, passcode, current);
            for (const auto& n : next) {
                if (n.pos == dest) {
                    return n.path;
                }
                queue.push_back(n.path);
                std::push_heap(queue.begin(), queue.end(), heap_cmp);
            }
        }
        return {};
    }

    std::string longest_path(const std::string& passcode, const position dest) {
        openssl_hash h {EVP_md5()};

        //Using less will create a max heap; we want a min heap.
        const auto heap_cmp = [](std::string_view a, std::string_view b){ return a.size() < b.size(); };

        std::vector<std::string> queue;
        queue.emplace_back();
        std::string retval;
        while (!queue.empty()) {
            const auto current = queue.front();
            std::pop_heap(queue.begin(), queue.end(), heap_cmp);
            queue.erase(queue.end() - 1);

            const auto next = next_paths(h, passcode, current);
            for (const auto& n : next) {
                if (n.pos == dest) {
                    if (n.path.size() > retval.size()) {
                        retval = n.path;
                    }
                    continue;
                }
                queue.push_back(n.path);
                std::push_heap(queue.begin(), queue.end(), heap_cmp);
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto p = shortest_path(input, {3, 3});
        return p;
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto p = longest_path(input, {3, 3});
        return std::to_string(p.size());
    }

    aoc::registration r{2016, 17, part_1, part_2};

//    TEST_SUITE("2016_day17") {
//        TEST_CASE("2016_day17:example") {
//
//        }
//    }

} /* namespace <anon> */