//
// Created by Daniel Garcia on 4/27/21.
//

#include "registration.h"
#include "utilities.h"
#include "grid.h"
#include "parse.h"

#include <doctest/doctest.h>

#include <string_view>
#include <regex>
#include <numeric>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    enum class action {
        turn_on,
        turn_off,
        toggle
    };

    struct instruction {
        position start;
        position end;
        action action;
    };

    std::regex instr_regex {R"(^(turn on|turn off|toggle) (\d+),(\d+) through (\d+),(\d+)$)"};

    instruction parse(std::string_view s) {
        std::match_results<std::string_view::iterator> match;
        if (std::regex_match(s.begin(), s.end(), match, instr_regex) && match.size() == 6) {
            action a{};
            if (match[1].str() == "turn on"sv) {
                a = action::turn_on;
            }
            else if (match[1].str() == "turn off"sv) {
                a = action::turn_off;
            }
            else if (match[1].str() == "toggle"sv) {
                a = action::toggle;
            }
            else {
                throw std::runtime_error{"Invalid instruction action."};
            }
            return {{aoc::parse<int>(match[2].str()), aoc::parse<int>(match[3].str())},
                    {aoc::parse<int>(match[4].str()), aoc::parse<int>(match[5].str())},
                    a};
        }
        else {
            throw std::runtime_error{"Failed to parse instruction entry."};
        }
    }

    std::vector<instruction> get_input(const std::filesystem::path& input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2015" / "day_6_input.txt");
        std::vector<instruction> retval;
        retval.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse);
        return retval;
    }

    using light = uint16_t;

    void act_1(light& l, action a) {
        switch (a) {
            case action::turn_on: l = 1; break;
            case action::turn_off: l = 0; break;
            case action::toggle: l = (l - 1) & 1; break;
            default: break;
        }
    }

    void act_2(light& l, action a) {
        switch (a) {
            case action::turn_on: ++l; break;
            case action::turn_off: l = l - (-(l > 0) & 0x1); break;
            case action::toggle: l += 2; break;
            default: break;
        }
    }

    void apply(grid<light>& lights, const instruction& inst, void (*act)(light&, action)) {
        const int row_delta = inst.end.x > inst.start.x ? 1 : -1;
        const int col_delta = inst.end.y > inst.start.y ? 1 : -1;
        if (row_delta > 0) {
            for (auto row = inst.start.x; row <= inst.end.x; ++row) {
                auto row_view = lights.row_span(row);
                if (col_delta > 0) {
                    for (auto col = inst.start.y; col <= inst.end.y; ++col) {
                        act(row_view[col], inst.action);
                    }
                }
                else {
                    for (auto col = inst.start.y; col >= inst.end.y; --col) {
                        act(row_view[col], inst.action);
                    }
                }
            }
        }
        else {
            for (auto row = inst.start.x; row >= inst.end.x; --row) {
                auto row_view = lights.row_span(row);
                if (col_delta > 0) {
                    for (auto col = inst.start.y; col <= inst.end.y; ++col) {
                        act(row_view[col], inst.action);
                    }
                }
                else {
                    for (auto col = inst.start.y; col >= inst.end.y; --col) {
                        act(row_view[col], inst.action);
                    }
                }
            }
        }
    }

    /*
    --- Day 6: Probably a Fire Hazard ---

    Because your neighbors keep defeating you in the holiday house decorating contest year after year, you've decided to deploy one million lights in a 1000x1000 grid.

    Furthermore, because you've been especially nice this year, Santa has mailed you instructions on how to display the ideal lighting configuration.

    Lights in your grid are numbered from 0 to 999 in each direction; the lights at each corner are at 0,0, 0,999, 999,999, and 999,0. The instructions include whether to turn on, turn off, or toggle various inclusive ranges given as coordinate pairs. Each coordinate pair represents opposite corners of a rectangle, inclusive; a coordinate pair like 0,0 through 2,2 therefore refers to 9 lights in a 3x3 square. The lights all start turned off.

    To defeat your neighbors this year, all you have to do is set up your lights by doing the instructions Santa sent you in order.

    For example:

    turn on 0,0 through 999,999 would turn on (or leave on) every light.
    toggle 0,0 through 999,0 would toggle the first line of 1000 lights, turning off the ones that were on, and turning on the ones that were off.
    turn off 499,499 through 500,500 would turn off (or leave off) the middle four lights.
    After following the instructions, how many lights are lit?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        auto instrustions = get_input(input_dir);
        grid<light> lights {1000,1000};
        for (const auto& ins : instrustions) {
            apply(lights, ins, act_1);
        }
        return std::to_string(std::count_if(lights.begin(), lights.end(),
                                            [](light l){ return l != 0; }));
    }

    /*
    You just finish implementing your winning light pattern when you realize you mistranslated Santa's message from Ancient Nordic Elvish.

    The light grid you bought actually has individual brightness controls; each light can have a brightness of zero or more. The lights all start at zero.

    The phrase turn on actually means that you should increase the brightness of those lights by 1.

    The phrase turn off actually means that you should decrease the brightness of those lights by 1, to a minimum of zero.

    The phrase toggle actually means that you should increase the brightness of those lights by 2.

    What is the total brightness of all lights combined after following Santa's instructions?

    For example:

    turn on 0,0 through 0,0 would increase the total brightness by 1.
    toggle 0,0 through 999,999 would increase the total brightness by 2000000.
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto instrustions = get_input(input_dir);
        grid<light> lights {1000,1000};
        for (const auto& ins : instrustions) {
            apply(lights, ins, act_2);
        }
        return std::to_string(std::accumulate(lights.begin(), lights.end(), uint64_t{}));
    }

    aoc::registration r {2015, 6, part_1, part_2};

    TEST_SUITE("2015_day6") {
        TEST_CASE("2015_day6:example") {
//        REQUIRE_EQ(count_visits(calculate_visits(">")), 2);
//        REQUIRE_EQ(count_visits(calculate_visits("^>v<")), 4);
//        REQUIRE_EQ(count_visits(calculate_visits("^v^v^v^v^v")), 2);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^v")), 3);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^>v<")), 3);
//        REQUIRE_EQ(count_visits(calculate_alt_visits("^v^v^v^v^v")), 11);
    }

}

}

