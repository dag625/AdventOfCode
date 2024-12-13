//
// Created by Dan on 12/13/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <iostream>

#include "utilities.h"
#include "grid_combine.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/21
     */

    struct conversion {
        grid<char> from;
        grid<char> to;

        [[nodiscard]] auto from_size() const { return from.num_rows(); }
        [[nodiscard]] auto to_size() const { return to.num_rows(); }
    };

    std::vector<std::string> split_grid_line(std::string_view s) {
        const auto lines = split(s, '/');
        return lines | std::views::transform([](std::string_view v){ return std::string{v}; }) | std::ranges::to<std::vector>();
    }

    conversion parse_conversion(std::string_view s) {
        const auto parts = split(s, ' ');
        return {to_grid(split_grid_line(parts[0])), to_grid(split_grid_line(parts[2]))};
    }

    std::vector<conversion> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2017" / "day_21_input.txt");
        return lines | std::views::transform(&parse_conversion) | std::ranges::to<std::vector>();
    }

    grid<char> get_init() {
        const std::vector<std::string> lines {
                ".#.",
                "..#",
                "###"
        };
        return to_grid(lines);
    }

    grid<char> convert(const grid<char>& start, const std::vector<conversion>& conv) {
        for (const auto& c : conv) {
            auto to_find = c.from;
            for (int i = 0; i < 4; ++i) {
                if (start == to_find) {
                    return c.to;
                }
                if (const auto fh = to_find.flip_horizontal(); start == fh) {
                    return c.to;
                }
                else if (const auto fv = to_find.flip_vertical(); start == fv) {
                    return c.to;
                }
                to_find = to_find.rotate_cw();
            }

        }
        return start;//Shouldn't ever do this I think.
    }

    template <std::size_t N>
    grid<char> stepN(const grid<char>& start, const std::vector<conversion>& conv) {
        std::vector<std::vector<grid<char>>> subs;
        for (int r = 0; r < start.num_rows(); r += N) {
            subs.emplace_back();
            for (int c = 0; c < start.num_cols(); c += N) {
                const auto sub = start.subgrid({r, c}, {N, N});
                subs[r / N].push_back(convert(sub, conv));
            }
        }
        if (subs.size() == 1) {
            return subs[0][0];
        }

        const auto rows = subs | std::views::transform([](const std::vector<grid<char>>& gs){ return combine_horizontal(gs).value(); }) | std::ranges::to<std::vector>();
        return combine_vertical(rows).value();
    }

    grid<char> step(const grid<char>& start, const std::vector<conversion>& conv2, const std::vector<conversion>& conv3) {
        if (start.num_rows() % 2 == 0) {
            return stepN<2>(start, conv2);
        }
        else { //Divisible by 3
            return stepN<3>(start, conv3);
        }
    }

    std::pair<std::vector<conversion>, std::vector<conversion>> split_conversions(const std::vector<conversion>& all) {
        return {
            all | std::views::filter([](const conversion& c){ return c.from_size() == 2; }) | std::ranges::to<std::vector>(),
            all | std::views::filter([](const conversion& c){ return c.from_size() == 3; }) | std::ranges::to<std::vector>()
        };
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto [conv2, conv3] = split_conversions(input);
        auto g = get_init();
        for (int i = 0; i < 5; ++i) {
            g = step(g, conv2, conv3);
        }
        const auto num_on = std::count_if(g.begin(), g.end(), [](const char c){ return c == '#'; });
        return std::to_string(num_on);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);        const auto [conv2, conv3] = split_conversions(input);
        auto g = get_init();
        for (int i = 0; i < 18; ++i) {
            g = step(g, conv2, conv3);
        }
        const auto num_on = std::count_if(g.begin(), g.end(), [](const char c){ return c == '#'; });
        return std::to_string(num_on);
        //~>8sec on release.  There's probably a pattern we can look for, but I'll stick with brute force for now.
    }

    aoc::registration r{2017, 21, part_1, part_2};

    TEST_SUITE("2017_day21") {
        TEST_CASE("2017_day21:example") {
            const std::vector<std::string> lines {
                    "../.# => ##./#../...",
                    ".#./..#/### => #..#/..../..../#..#"
            };
            const auto input = lines | std::views::transform(&parse_conversion) | std::ranges::to<std::vector>();
            const auto [conv2, conv3] = split_conversions(input);
            auto g = get_init();
            std::cout << "\nInitial grid:" << std::endl;
            g.display(std::cout);
            g = step(g, conv2, conv3);
            std::cout << "\nAfter 1 iteration:" << std::endl;
            g.display(std::cout);
            g = step(g, conv2, conv3);
            std::cout << "\nAfter 2 iterations:" << std::endl;
            g.display(std::cout);
            CHECK(true);
        }
    }

} /* namespace <anon> */