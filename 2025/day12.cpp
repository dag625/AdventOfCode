//
// Created by Dan on 12/12/2025.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "grid.h"
#include "parse.h"

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2025/day/12
     */

    struct present {
        int id = 0;
        grid<char> shape;
        int area = 0;
    };

    struct tree {
        int width = 0;
        int height = 0;
        std::vector<int> ids;
    };

    tree parse_tree(std::string_view str) {
        const auto parts = split(str, ':');
        const auto dims = split(parts[0], 'x');
        const auto ids = split_no_empty(parts[1], ' ');
        return {parse32(dims[0]), parse32(dims[1]), ids | std::views::transform(&parse32) | std::ranges::to<std::vector>()};
    }

    std::pair<std::vector<present>, std::vector<tree>> get_input(const std::vector<std::string>& lines) {
        std::vector<present> presents;
        auto current = lines.begin();
        while (current->size() < 10) {
            const int id = parse32(*current);
            ++current;
            auto stop = current;
            while (!stop->empty()) { ++stop; }
            presents.emplace_back(id, to_grid(current, stop), 0);
            presents.back().area = static_cast<int>(std::count(presents.back().shape.data().begin(), presents.back().shape.data().end(), '#'));
            current = stop + 1;
        }
        return {
            std::move(presents),
            lines |
                std::views::drop(std::distance(lines.begin(), current)) |
                std::views::transform(&parse_tree) |
                std::ranges::to<std::vector>()};
    }

    bool can_fit(const std::vector<present>& gifts, const tree& t) {
        int total = 0;
        const auto tree_area = t.width * t.height;
        for (int i = 0; i < gifts.size(); ++i) {
            total += gifts[i].area * t.ids[i];
        }
        return total <= tree_area;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto [gifts, trees] = get_input(lines);
        const auto res = std::count_if(trees.begin(), trees.end(), [&gifts](const tree& t){ return can_fit(gifts, t); });
        return std::to_string(res);
    }

    aoc::registration r{2025, 12, part_1};

    //    TEST_SUITE("2025_day12") {
    //        TEST_CASE("2025_day12:example") {
    //            const std::vector<std::string> lines {
    //
    //            };
    //            const auto input = get_input(lines);
    //
    //        }
    //    }

} /* namespace <anon> */
