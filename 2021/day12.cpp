//
// Created by Dan on 12/11/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <regex>
#include <unordered_map>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int START_ID = 0;
    constexpr int END_ID = 1000;

    struct line {
        int start;
        int end;
    };

    bool connect(int id, const line& b) noexcept {
        return id == b.start ||
                id == b.end;
    }

    int other(int id, const line& b) noexcept {
        return id == b.start ? b.end : b.start;
    }

    bool connect(const line& a, const line& b) noexcept {
        return a.start == b.start ||
            a.start == b.end ||
            a.end == b.start ||
            a.end == b.end;
    }

    int get_id(std::unordered_map<std::string_view, int>& map, int& next, std::string_view name) {
        using namespace std::string_view_literals;
        if (name == "start"sv) {
            return START_ID;
        }
        else if (name == "end"sv) {
            return END_ID;
        }
        else if (map.contains(name)) {
            return map[name];
        }
        else {
            auto id = next++;
            if (name[0] > 'Z') {
                id = -id;
            }
            map[name] = id;
            return id;
        }
    }

    std::vector<line> get_input(const std::vector<std::string>& lines) {
        std::vector<line> retval;
        retval.reserve(lines.size());
        std::unordered_map<std::string_view, int> id_map;
        int next_id = 1;
        for (const auto& s : lines) {
            const auto parts = split(s, '-');
            retval.push_back({get_id(id_map, next_id, parts[0]), get_id(id_map, next_id, parts[1])});
        }
        return retval;
    }

    int count_paths(const std::vector<line>& edges, std::vector<int>& path) {
        int retval = 0, prev = path.back();
        for (const auto& next : edges | std::views::filter([prev](const line& e){ return connect(prev, e); })) {
            const auto next_id = other(prev, next);
            if (next_id == END_ID) {
                retval += 1;
            }
            else if (next_id == START_ID || (next_id < 0 && std::find(path.begin(), path.end(), next_id) != path.end())) {
                continue;
            }
            else {
                path.push_back(next_id);
                retval += count_paths(edges, path);
                path.pop_back();
            }
        }
        return retval;
    }

    int count_paths_allow_one_small_twice(const std::vector<line>& edges, std::vector<int>& path, std::optional<int>& repeated) {
        int retval = 0, prev = path.back();
        for (const auto& next : edges | std::views::filter([prev](const line& e){ return connect(prev, e); })) {
            const auto next_id = other(prev, next);
            if (next_id == END_ID) {
                retval += 1;
            }
            else if (next_id == START_ID) {
                continue;
            }
            else if (next_id < 0 && std::find(path.begin(), path.end(), next_id) != path.end()) {
                if (repeated) {
                    continue;
                }
                else {
                    repeated = next_id;
                    path.push_back(next_id);
                    retval += count_paths_allow_one_small_twice(edges, path, repeated);
                    path.pop_back();
                    repeated = std::nullopt;
                }
            }
            else {
                path.push_back(next_id);
                retval += count_paths_allow_one_small_twice(edges, path, repeated);
                path.pop_back();
            }
        }
        return retval;
    }

    /*
    --- Day 12: Passage Pathing ---
    With your submarine's subterranean subsystems subsisting suboptimally, the only way you're getting out of this cave anytime soon is by finding a path yourself. Not just a path - the only way to know if you've found the best path is to find all of them.

    Fortunately, the sensors are still mostly working, and so you build a rough map of the remaining caves (your puzzle input). For example:

    start-A
    start-b
    A-c
    A-b
    b-d
    A-end
    b-end
    This is a list of how all of the caves are connected. You start in the cave named start, and your destination is the cave named end. An entry like b-d means that cave b is connected to cave d - that is, you can move between them.

    So, the above cave system looks roughly like this:

        start
        /   \
    c--A-----b--d
        \   /
         end
    Your goal is to find the number of distinct paths that start at start, end at end, and don't visit small caves more than once. There are two types of caves: big caves (written in uppercase, like A) and small caves (written in lowercase, like b). It would be a waste of time to visit any small cave more than once, but big caves are large enough that it might be worth visiting them multiple times. So, all paths you find should visit small caves at most once, and can visit big caves any number of times.

    Given these rules, there are 10 paths through this example cave system:

    start,A,b,A,c,A,end
    start,A,b,A,end
    start,A,b,end
    start,A,c,A,b,A,end
    start,A,c,A,b,end
    start,A,c,A,end
    start,A,end
    start,b,A,c,A,end
    start,b,A,end
    start,b,end
    (Each line in the above list corresponds to a single path; the caves visited by that path are listed in the order they are visited and separated by commas.)

    Note that in this cave system, cave d is never visited by any path: to do so, cave b would need to be visited twice (once on the way to cave d and a second time when returning from cave d), and since cave b is small, this is not allowed.

    Here is a slightly larger example:

    dc-end
    HN-start
    start-kj
    dc-start
    dc-HN
    LN-dc
    HN-end
    kj-sa
    kj-HN
    kj-dc
    The 19 paths through it are as follows:

    start,HN,dc,HN,end
    start,HN,dc,HN,kj,HN,end
    start,HN,dc,end
    start,HN,dc,kj,HN,end
    start,HN,end
    start,HN,kj,HN,dc,HN,end
    start,HN,kj,HN,dc,end
    start,HN,kj,HN,end
    start,HN,kj,dc,HN,end
    start,HN,kj,dc,end
    start,dc,HN,end
    start,dc,HN,kj,HN,end
    start,dc,end
    start,dc,kj,HN,end
    start,kj,HN,dc,HN,end
    start,kj,HN,dc,end
    start,kj,HN,end
    start,kj,dc,HN,end
    start,kj,dc,end
    Finally, this even larger example has 226 paths through it:

    fs-end
    he-DX
    fs-he
    start-DX
    pj-DX
    end-zg
    zg-sl
    zg-pj
    pj-he
    RW-he
    fs-DX
    pj-RW
    zg-RW
    start-pj
    he-WI
    zg-he
    pj-fs
    start-RW
    How many paths through this cave system are there that visit small caves at most once?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<int> path = {START_ID};
        return std::to_string(count_paths(input, path));
    }

    /*
    --- Part Two ---
    After reviewing the available paths, you realize you might have time to visit a single small cave twice. Specifically, big caves can be visited any number of times, a single small cave can be visited at most twice, and the remaining small caves can be visited at most once. However, the caves named start and end can only be visited exactly once each: once you leave the start cave, you may not return to it, and once you reach the end cave, the path must end immediately.

    Now, the 36 possible paths through the first example above are:

    start,A,b,A,b,A,c,A,end
    start,A,b,A,b,A,end
    start,A,b,A,b,end
    start,A,b,A,c,A,b,A,end
    start,A,b,A,c,A,b,end
    start,A,b,A,c,A,c,A,end
    start,A,b,A,c,A,end
    start,A,b,A,end
    start,A,b,d,b,A,c,A,end
    start,A,b,d,b,A,end
    start,A,b,d,b,end
    start,A,b,end
    start,A,c,A,b,A,b,A,end
    start,A,c,A,b,A,b,end
    start,A,c,A,b,A,c,A,end
    start,A,c,A,b,A,end
    start,A,c,A,b,d,b,A,end
    start,A,c,A,b,d,b,end
    start,A,c,A,b,end
    start,A,c,A,c,A,b,A,end
    start,A,c,A,c,A,b,end
    start,A,c,A,c,A,end
    start,A,c,A,end
    start,A,end
    start,b,A,b,A,c,A,end
    start,b,A,b,A,end
    start,b,A,b,end
    start,b,A,c,A,b,A,end
    start,b,A,c,A,b,end
    start,b,A,c,A,c,A,end
    start,b,A,c,A,end
    start,b,A,end
    start,b,d,b,A,c,A,end
    start,b,d,b,A,end
    start,b,d,b,end
    start,b,end
    The slightly larger example above now has 103 paths through it, and the even larger example now has 3509 paths through it.

    Given these new rules, how many paths through this cave system are there?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<int> path = {START_ID};
        std::optional<int> repeated{};
        return std::to_string(count_paths_allow_one_small_twice(input, path, repeated));
    }

    aoc::registration r {2021, 12, part_1, part_2};

//    TEST_SUITE("2021_day12") {
//        TEST_CASE("2021_day12:example") {

//        }
//    }

}