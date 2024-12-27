//
// Created by Dan on 11/12/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct input_node {
        int bot = 0;
        int value = 0;
    };

    struct transfer_node {
        int bot = 0;
        bool dest_low_bot = true;
        int dest_low = 0;
        bool dest_hi_bot = true;
        int dest_hi = 0;
    };

    struct config {
        std::vector<input_node> inputs;
        std::vector<transfer_node> transfers;
    };

    input_node parse_input(std::string_view s) {
        const auto parts = split(s, ' ');
        return {parse<int>(parts[5]), parse<int>(parts[1])};
    }

    transfer_node parse_transfer(std::string_view s) {
        using namespace std::string_view_literals;
        const auto parts = split(s, ' ');
        return {parse<int>(parts[1]), parts[5] == "bot"sv, parse<int>(parts[6]), parts[10] == "bot"sv, parse<int>(parts[11])};
    }

    void parse_line(std::string_view s, config& cfg) {
        if (s.front() == 'v') {
            cfg.inputs.push_back(parse_input(s));
        }
        else {
            cfg.transfers.push_back(parse_transfer(s));
        }
    }

    config get_input(const std::vector<std::string>& lines) {
        config retval;
        for (const auto& l : lines) {
            parse_line(l, retval);
        }
        return retval;
    }

    struct bot {
        int id = 0;
        std::optional<int> lo;
        std::optional<int> hi;

        bot(int i) : id{i} {}

        void set(int val) {
            if (lo && *lo != val && !hi && *lo > val) {
                hi = *lo;
                lo = val;
            }
            else if (lo && *lo != val && !hi) {
                hi = val;
            }
            else if (!lo) {
                lo = val;
            }
        }
    };

    std::vector<bot> init_from_config(const config& cfg) {
        std::vector<bot> retval;
        for (const auto& tr : cfg.transfers) {
            retval.emplace_back(tr.bot);
        }
        for (const auto& in : cfg.inputs) {
            auto found = std::find_if(retval.begin(), retval.end(), [id = in.bot](const bot& b){ return b.id == id; });
            found->set(in.value);
        }
        return retval;
    }

    bool iterate(const config& cfg, std::vector<bot>& bots) {
        for (auto& b : bots) {
            if (b.lo && b.hi) {
                const auto found = std::find_if(cfg.transfers.begin(), cfg.transfers.end(),
                                                [id = b.id](const transfer_node& n){ return n.bot == id; });
                if (found->dest_low_bot) {
                    auto bfound = std::find_if(bots.begin(), bots.end(),
                                               [id = found->dest_low](const bot& b){ return b.id == id; });
                    bfound->set(*b.lo);
                }
                if (found->dest_hi_bot) {
                    auto bfound = std::find_if(bots.begin(), bots.end(),
                                               [id = found->dest_hi](const bot& b){ return b.id == id; });
                    bfound->set(*b.hi);
                }
            }
        }
        return std::all_of(bots.begin(), bots.end(), [](const bot& b){ return b.lo && b.hi; });
    }

    struct output {
        int id;
        bool hi;


    };

    std::optional<output> has_output(const transfer_node& t, int val) {
        if (!t.dest_hi_bot && t.dest_hi == val) {
            return output{t.bot, true};
        }
        else if (!t.dest_low_bot && t.dest_low == val) {
            return output{t.bot, false};
        }
        else {
            return std::nullopt;
        }
    }

    int get_output(const std::vector<bot>& bots, const output& out) {
        const auto found = std::find_if(bots.begin(), bots.end(), [id = out.id](const bot& b){ return b.id == id; });
        return out.hi ? found->hi.value() : found->lo.value();
    }

    /*
    --- Day 10: Balance Bots ---
    You come upon a factory in which many robots are zooming around handing small microchips to each other.

    Upon closer examination, you notice that each bot only proceeds when it has two microchips, and once it does, it gives each one to a different bot or puts it in a marked "output" bin. Sometimes, bots take microchips from "input" bins, too.

    Inspecting one of the microchips, it seems like they each contain a single number; the bots must use some logic to decide what to do with each chip. You access the local control computer and download the bots' instructions (your puzzle input).

    Some of the instructions specify that a specific-valued microchip should be given to a specific bot; the rest of the instructions indicate what a given bot should do with its lower-value or higher-value chip.

    For example, consider the following instructions:

    value 5 goes to bot 2
    bot 2 gives low to bot 1 and high to bot 0
    value 3 goes to bot 1
    bot 1 gives low to output 1 and high to bot 0
    bot 0 gives low to output 2 and high to output 0
    value 2 goes to bot 2
    Initially, bot 1 starts with a value-3 chip, and bot 2 starts with a value-2 chip and a value-5 chip.
    Because bot 2 has two microchips, it gives its lower one (2) to bot 1 and its higher one (5) to bot 0.
    Then, bot 1 has two microchips; it puts the value-2 chip in output 1 and gives the value-3 chip to bot 0.
    Finally, bot 0 has two microchips; it puts the 3 in output 2 and the 5 in output 0.
    In the end, output bin 0 contains a value-5 microchip, output bin 1 contains a value-2 microchip, and output bin 2 contains a value-3 microchip. In this configuration, bot number 2 is responsible for comparing value-5 microchips with value-2 microchips.

    Based on your instructions, what is the number of the bot that is responsible for comparing value-61 microchips with value-17 microchips?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto bots = init_from_config(input);
        while (!iterate(input, bots)) {}
        const auto found = std::find_if(bots.begin(), bots.end(), [](const bot& b){ return *b.lo == 17 && *b.hi == 61; });
        return std::to_string(found->id);
    }

    /*
    --- Part Two ---
    What do you get if you multiply together the values of one chip in each of outputs 0, 1, and 2?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        auto bots = init_from_config(input);
        while (!iterate(input, bots)) {}
        const auto found0 = std::find_if(input.transfers.begin(), input.transfers.end(),
                                         [](const transfer_node& t){ return has_output(t, 0).has_value(); });
        const auto found1 = std::find_if(input.transfers.begin(), input.transfers.end(),
                                         [](const transfer_node& t){ return has_output(t, 1).has_value(); });
        const auto found2 = std::find_if(input.transfers.begin(), input.transfers.end(),
                                         [](const transfer_node& t){ return has_output(t, 2).has_value(); });
        const auto output0 = get_output(bots, *has_output(*found0, 0));
        const auto output1 = get_output(bots, *has_output(*found1, 1));
        const auto output2 = get_output(bots, *has_output(*found2, 2));
        return std::to_string(output0 * output1 * output2);
    }

    aoc::registration r{2016, 10, part_1, part_2};

//    TEST_SUITE("2016_day10") {
//        TEST_CASE("2016_day10:example") {
//
//        }
//    }

}