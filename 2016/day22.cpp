//
// Created by Dan on 12/7/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "grid.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/22
     */

    struct node_data {
        int size = 0;
        int used = 0;
        int avail = 0;
        int free_pct = 0;
    };

    struct node {
        position pos;
        node_data data;
    };

    node parse_node(std::string_view s) {
        const auto parts = split_no_empty(s, ' ');
        const auto path = split(parts[0], '-');
        return {{parse<int>(path[1].substr(1)), parse<int>(path[2].substr(1))},
                {parse<int>(parts[1]), parse<int>(parts[2]), parse<int>(parts[3]), parse<int>(parts[4])}};
    }

    bool sort_min_used(const node& lhs, const node& rhs) noexcept {
        return lhs.data.used < rhs.data.used;
    }

    bool sort_max_avail(const node& lhs, const node& rhs) noexcept {
        return lhs.data.avail > rhs.data.avail;
    }

    std::pair<grid<node_data>, position> to_grid(const std::vector<node>& nodes) {
        const auto farthest_corner = nodes.back().pos;
        grid<node_data> g {static_cast<size_t>(farthest_corner.x + 1), static_cast<size_t>(farthest_corner.y + 1)};
        position bubble{};
        for (const auto& n : nodes) {
            g[n.pos] = n.data;
            if (n.data.used == 0) {
                bubble = n.pos;
            }
        }
        return {std::move(g), bubble};
    }

    grid<char> to_displayable(const grid<node_data>& g, const position bubble, const position data) {
        grid<char> retval {g.num_rows(), g.num_cols()};
        for (const auto& p : g.list_positions()) {
            if (g[p].used > 100) {
                retval[p] = '#';
            }
            else {
                retval[p] = '.';
            }
        }
        retval[bubble] = 'O';
        retval[data] = '*';
        return retval;
    }

    std::vector<node> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_22_input.txt");
        return lines | std::views::drop(2) | std::views::transform(&parse_node) | std::ranges::to<std::vector>();
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto min_used = input;
        auto max_avail = input;
        std::sort(min_used.begin(), min_used.end(), &sort_min_used);
        std::sort(max_avail.begin(), max_avail.end(), &sort_max_avail);
        int64_t viable_pairs = 0;
        auto smallest_avail = max_avail.end() - 1;
        for (const auto& used : min_used) {
            if (used.data.used == 0) {
                continue;
            }
            while (smallest_avail != max_avail.begin() && smallest_avail->data.avail < used.data.used) {
                --smallest_avail;
            }
            if (smallest_avail == max_avail.begin() && smallest_avail->data.avail < used.data.used) {
                break;
            }
            auto num_fit = std::distance(max_avail.begin(), smallest_avail + 1);
            if (used.data.avail >= smallest_avail->data.avail) {
                //num_fit includes used, exclude it.
                --num_fit;
            }
            viable_pairs += num_fit;
        }
        return std::to_string(viable_pairs);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        /*
         * From the results of the below operations, excluding the very large nodes, any node can hold any other
         * node's data, so we can freely swap among the non-large nodes.
         * Basically smallest_size->data.size > largest_used->data.used.
         */
//        const auto smallest_size = std::min_element(input.begin(), input.end(), [](const node& a, const node& b){ return a.data.size < b.data.size; });
//        const auto exclude_big = input | std::views::filter([](const node& n){ return n.data.used < 100; }) | std::ranges::to<std::vector>();
//        const auto largest_used = std::ranges::max_element(exclude_big, [](const node& a, const node& b){ return a.data.used < b.data.used; });
        /*
         * So from printing things out, this is actually much simpler than I was worried.  The impassible nodes
         * form a line in the y=2 column starting from the bottom and with impassible node with the minimum x
         * having a lesser x than the bubble node.  So the answer is how many steps does it take to move the
         * bubble up and over the "wall" and down to the data, and then to move the data up the y=0 column.  This
         * can be determined without simulation.
         */
        const auto [data_grid, bubble] = to_grid(input);
        const position data_start {static_cast<int>(data_grid.num_rows() - 1), 0};
//        const auto display_grid = to_displayable(data_grid, bubble, data_start);
//        std::cout << "\nNodes:\n";
//        display_grid.display(std::cout);
//        std::cout << std::endl;
        int clear_x = static_cast<int>(data_grid.num_rows() - 1);
        while (clear_x >= 0) {
            if (data_grid[clear_x][2].used < 100) {
                break;
            }
            --clear_x;
        }
        const auto steps_to_top_of_wall = bubble.x - clear_x;
        const auto steps_to_edge = bubble.y;
        const auto steps_to_data = data_start.x - clear_x;
        const auto steps_to_shift_data_by_1 = 5;
        const auto steps_to_move_data_to_top = (data_start.x - 1) * steps_to_shift_data_by_1;
        const auto total_steps = steps_to_top_of_wall + steps_to_edge + steps_to_data + steps_to_move_data_to_top;
        return std::to_string(total_steps);
    }

    aoc::registration r{2016, 22, part_1, part_2};

//    TEST_SUITE("2016_day22") {
//        TEST_CASE("2016_day22:example") {
//
//        }
//    }

} /* namespace <anon> */