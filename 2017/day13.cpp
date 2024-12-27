//
// Created by Dan on 12/10/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <numeric>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2017/day/13
     */

    struct layer {
        int id = 0;
        int range = 0;

        [[nodiscard]] int loop_time() const { return 2 * range - 2; }
        [[nodiscard]] int scanner_pos(int time) const {
            const auto loop_t = loop_time();
            int in_loop = time % loop_t;
            if (in_loop < range) {
                //going down
                return in_loop;
            }
            else {
                //coming up
                return loop_t - in_loop;
            }
        }
    };

    layer parse_layer(std::string_view s) {
        const auto parts = split(s, ':');
        return {parse<int>(parts[0]), parse<int>(trim(parts[1]))};
    }

    std::vector<layer> get_input(const std::vector<std::string>& lines) {
        return lines | std::views::transform(&parse_layer) | std::ranges::to<std::vector>();
    }

    int severity(int start, const std::vector<layer>& layers) {
        int sev = 0;
        for (const auto& lay : layers) {
            const auto pos = lay.scanner_pos(start + lay.id);
            if (pos == 0) {
                sev += lay.id * lay.range;
            }
        }
        return sev;
    }

    bool check_if_caught(int start, const std::vector<layer>& layers) {
        for (const auto& lay : layers) {
            const auto pos = lay.scanner_pos(start + lay.id);
            if (pos == 0) {
                return true;
            }
        }
        return false;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto sev = severity(0, input);
        return std::to_string(sev);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        //Do it the dumb way
        int delay = 1;
        for (; delay < std::numeric_limits<int>::max(); ++delay) {
            if (!check_if_caught(delay, input)) {
                break;
            }
        }
        return std::to_string(delay);
    }

    aoc::registration r{2017, 13, part_1, part_2};

    TEST_SUITE("2017_day3") {
        TEST_CASE("2017_day13:example") {
            const std::vector<std::string> lines {
                    "0: 3",
                    "1: 2",
                    "4: 4",
                    "6: 4"
            };
            const auto input = lines | std::views::transform(&parse_layer) | std::ranges::to<std::vector>();
            const auto sev = severity(0, input);
            CHECK_EQ(sev, 24);
        }
    }

} /* namespace <anon> */