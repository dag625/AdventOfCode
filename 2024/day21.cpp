//
// Created by Dan on 12/21/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <utility>
#include <vector>
#include <iostream>

#include "utilities.h"
#include "parse.h"
#include "point.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/21
     */

    std::vector<std::string> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2024" / "day_21_input.txt");
        return lines;
    }

    struct edge {
        char to = '\0';
        std::vector<std::string> dirs;

        bool operator==(const char rhs) const { return to == rhs; }
    };

    struct mapping {
        char from = '\0';
        std::vector<edge> options;

        bool operator==(const char rhs) const { return from == rhs; }
    };

    std::vector<std::string> diffs_to_dirs(const position from, const position to) {
        const auto del = to - from;
        //Add dirs in order:  Left, Up, Down, Right
        //This will always avoid the gap and using consecutive in same direction gets us shortest.
        std::vector<std::string> retval;
        if (del.x < 0) {
            std::string s;
            for (int i = 0; i > del.x; --i) {
                s.push_back('<');
            }
            retval.push_back(s);
        }
        if (del.x > 0) {
            std::string s;
            for (int i = 0; i < del.x; ++i) {
                s.push_back('>');
            }
            retval.push_back(s);
        }
        if (del.y < 0) {
            std::string s;
            for (int i = 0; i > del.y; --i) {
                s.push_back('v');
            }
            retval.push_back(s);
        }
        if (del.y > 0) {
            std::string s;
            for (int i = 0; i < del.y; ++i) {
                s.push_back('^');
            }
            retval.push_back(s);
        }
        return retval;
    }

    /*
+---+---+---+
| 7 | 8 | 9 |
+---+---+---+
| 4 | 5 | 6 |
+---+---+---+
| 1 | 2 | 3 |
+---+---+---+
    | 0 | A |
    +---+---+
     */
    std::vector<mapping> init_numpad() {
        std::vector<mapping> retval;
        const std::vector<std::pair<char, position>> pad {
                {'0', {1, 0}},
                {'1', {0, 1}},
                {'2', {1, 1}},
                {'3', {2, 1}},
                {'4', {0, 2}},
                {'5', {1, 2}},
                {'6', {2, 2}},
                {'7', {0, 3}},
                {'8', {1, 3}},
                {'9', {2, 3}},
                {'A', {2, 0}}
        };
        for (const auto [from, fpos] : pad) {
            mapping next;
            next.from = from;
            for (const auto [to, tpos] : pad) {
                edge e;
                e.to = to;
                e.dirs = diffs_to_dirs(fpos, tpos);
                next.options.push_back(std::move(e));
            }
            //The auto-gen is fine for most but not all, so we fixup and give the whole direction where only one is valid:
            if (fpos == position{0, 3}) { //7
                const auto found0 = std::find(next.options.begin(), next.options.end(), '0');
                const auto foundA = std::find(next.options.begin(), next.options.end(), 'A');
                found0->dirs = {">vvv"};
                foundA->dirs = {">>vvv"};
            }
            else if (fpos == position{0, 2}) { //4
                const auto found0 = std::find(next.options.begin(), next.options.end(), '0');
                const auto foundA = std::find(next.options.begin(), next.options.end(), 'A');
                found0->dirs = {">vv"};
                foundA->dirs = {">>vv"};
            }
            else if (fpos == position{0, 1}) { //1
                const auto found0 = std::find(next.options.begin(), next.options.end(), '0');
                const auto foundA = std::find(next.options.begin(), next.options.end(), 'A');
                found0->dirs = {">v"};
                foundA->dirs = {">>v"};
            }
            else if (fpos == position{1, 0}) { //0
                const auto found7 = std::find(next.options.begin(), next.options.end(), '7');
                const auto found4 = std::find(next.options.begin(), next.options.end(), '4');
                const auto found1 = std::find(next.options.begin(), next.options.end(), '1');
                found7->dirs = {"^^^<"};
                found4->dirs = {"^^<"};
                found1->dirs = {"^<"};
            }
            else if (fpos == position{2, 0}) { //A
                const auto found7 = std::find(next.options.begin(), next.options.end(), '7');
                const auto found4 = std::find(next.options.begin(), next.options.end(), '4');
                const auto found1 = std::find(next.options.begin(), next.options.end(), '1');
                found7->dirs = {"^^^<<"};
                found4->dirs = {"^^<<"};
                found1->dirs = {"^<<"};
            }
            retval.push_back(std::move(next));
        }
        return retval;
    }

    /*
    +---+---+
    | ^ | A |
+---+---+---+
| < | v | > |
+---+---+---+
     */
    std::vector<mapping> init_dirpad() {
        std::vector<mapping> retval;
        retval.emplace_back('A', std::vector<edge>{{'A', {""}}, {'^', {"<"}}, {'v', {"<", "v"}}, {'<', {"v<<"}}, {'>', {"v"}}});
        retval.emplace_back('^', std::vector<edge>{{'A', {">"}}, {'^', {""}}, {'v', {"v"}}, {'<', {"v<"}}, {'>', {"v", ">"}}});
        retval.emplace_back('v', std::vector<edge>{{'A', {"^", ">"}}, {'^', {"^"}}, {'v', {""}}, {'<', {"<"}}, {'>', {">"}}});
        retval.emplace_back('<', std::vector<edge>{{'A', {">>^"}}, {'^', {">^"}}, {'v', {">"}}, {'<', {""}}, {'>', {">>"}}});
        retval.emplace_back('>', std::vector<edge>{{'A', {"^"}}, {'^', {"<", "^"}}, {'v', {"<"}}, {'<', {"<<"}}, {'>', {""}}});
        return retval;
    }

    const std::vector<mapping>& numpad() { static const auto val = init_numpad(); return val; }
    const std::vector<mapping>& dirpad() { static const auto val = init_dirpad(); return val; }

    struct cache_key {
        std::string in;
        int depth = 0;

        cache_key(std::string_view s, int d) : in{s}, depth{d} {}

        auto operator<=>(const cache_key& rhs) const { return std::tie(depth, in) <=> std::tie(rhs.depth, rhs.in); }
        bool operator==(const cache_key& rhs) const { return std::tie(depth, in) == std::tie(rhs.depth, rhs.in); }
    };

    struct cache_item {
        cache_key key;
        int64_t shortest;

        cache_item(cache_key k, int64_t l) : key{std::move(k)}, shortest{l} {}

        auto operator<=>(const cache_item& rhs) const { return key <=> rhs.key; }
        bool operator==(const cache_item& rhs) const { return key == rhs.key; }

        auto operator<=>(const cache_key& rhs) const { return key <=> rhs; }
        bool operator==(const cache_key& rhs) const { return key == rhs; }
    };

    std::vector<std::string> get_options(std::string_view s, const std::vector<mapping>& map) {
        std::vector<std::string> retval;
        retval.emplace_back();
        char last = 'A';
        for (const char c : s) {
            const auto from_map = std::find(map.begin(), map.end(), last);
            const auto to_edge = std::find(from_map->options.begin(), from_map->options.end(), c);
            last = c;
            if (to_edge->dirs.size() > 2) {
                throw std::runtime_error{fmt::format("Have edge from {} to {} with {} dirs.", last, c, to_edge->dirs.size())};
            }
            else if (to_edge->dirs.size() == 2) {
                std::vector<std::string> split_tries;
                for (const auto& t : retval) {
                    split_tries.push_back(t + to_edge->dirs[0] + to_edge->dirs[1]);
                    split_tries.push_back(t + to_edge->dirs[1] + to_edge->dirs[0]);
                }
                retval.swap(split_tries);
            }
            else if (to_edge->dirs.size() == 1) {
                std::for_each(retval.begin(), retval.end(), [&e = to_edge->dirs.front()](std::string& v){ v.append(e); });
            }
            //if empty, we have no directions to add.
            std::for_each(retval.begin(), retval.end(), [](std::string& v){ v.push_back('A'); });
        }
//        std::for_each(retval.begin(), retval.end(), [s](std::string& v){
//            std::cout << "From " << s << " to " << v << std::endl;
//        });
        return retval;
    }

    int64_t get_min_dir(std::string_view s, const int depth_remaining, std::vector<cache_item>& cache) {
        const cache_key current {s, depth_remaining};
        const auto found = std::lower_bound(cache.begin(), cache.end(), current);
        if (found != cache.end() && *found == current) {
            return found->shortest;
        }
        else if (depth_remaining == 0) {
            return static_cast<int64_t>(s.size());
        }
        else {
            std::vector<std::string> to_try = get_options(s, dirpad());
            int64_t retval = std::numeric_limits<int64_t>::max();
            for (const auto& t : to_try) {
                /*
                 * We can't actually build the string for part 2 as it gets waaaaay too long.  So we need to content
                 * ourselves with just finding the length.  And for that every sequence leading up to an 'A' is going
                 * to independently contribute to the length.  So we split the string by the 'A's (leading to a set of
                 * strings each ending in 'A'), find the length of each right to the end, and then sum the lengths of
                 * the substrings together.  This also results in a much smaller cache space since each string past
                 * the first level of recursion will be at most 6-ish characters, so things complete very quickly,
                 */
                const auto parts = split_with(t, 'A');
                int64_t res = 0;
                for (const auto& p : parts) {
                    const auto part_res = get_min_dir(p, depth_remaining - 1, cache);
                    res += part_res;
                }
                if (res < retval) {
                    retval = res;
                }
            }
            const cache_key ret_key {s, depth_remaining};
            const auto found_ret = std::lower_bound(cache.begin(), cache.end(), ret_key);
            if (found_ret == cache.end() || *found_ret != ret_key) {
                cache.emplace(found_ret, ret_key, retval);
            }
            return retval;
        }
    }

    int64_t get_min(std::string_view s, const int num_dir_pads) {
        const auto num_pads = get_options(s, numpad());
        int64_t retval = std::numeric_limits<int64_t>::max();
        std::vector<cache_item> cache;
        for (const auto& t : num_pads) {
            const auto res = get_min_dir(t, num_dir_pads, cache);
            if (res < retval) {
                retval = res;
            }
        }
        return retval;
    }

    int get_numpad_num(std::string_view s) {
        s.remove_suffix(1);
        return parse<int>(s);
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        std::size_t total = 0;
        for (const auto& nums : input) {
            const auto converted = get_min(nums, 2);
            const auto num = get_numpad_num(nums);
            total += num * converted;
        }
        return std::to_string(total);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        std::size_t total = 0;
        for (const auto& nums : input) {
            const auto converted = get_min(nums, 25);
            const auto num = get_numpad_num(nums);
            total += num * converted;
        }
        return std::to_string(total);
    }

    aoc::registration r{2024, 21, part_1, part_2};

    TEST_SUITE("2024_day21") {
        TEST_CASE("2024_day21:example") {
            const std::vector<std::string> input {
                    "029A",
                    "980A",
                    "179A",
                    "456A",
                    "379A"
            };

            std::size_t total = 0;
            for (const auto& nums : input) {
                const auto converted = get_min(nums, 2);
                const auto num = get_numpad_num(nums);
                std::cout << fmt::format("{} ({}) -> {}", nums, num, converted) << std::endl;
                total += num * converted;
            }
            CHECK_EQ(total, 126384);
        }
    }

} /* namespace <anon> */