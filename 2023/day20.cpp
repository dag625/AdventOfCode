//
// Created by Dan on 12/19/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <deque>
#include <numeric>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    enum class mod_type {
        broadcast,
        flipflop,
        conjunction
    };

    struct input {
        std::string name;
        bool last_was_high = false;
    };

    struct mod {
        mod_type type = mod_type::flipflop;
        std::string name;
        std::vector<std::string> outputs;
        std::vector<input> inputs;
        bool is_on = false;
        std::vector<int> num_to_high;

        mod() = default;
        mod(mod_type t, std::string n) : type{t}, name{std::move(n)} {}
        mod(mod_type t, std::string n, std::vector<std::string> o) : type{t}, name{std::move(n)}, outputs{std::move(o)} {}
    };

    mod parse_mod(std::string_view s) {
        using namespace std::string_view_literals;
        auto parts = split(s, ' ');
        mod_type type = mod_type::broadcast;
        if (parts[0].front() == '%') {
            type = mod_type::flipflop;
            parts[0].remove_prefix(1);
        }
        else if (parts[0].front() == '&') {
            type = mod_type::conjunction;
            parts[0].remove_prefix(1);
        }
        auto outputs = parts |
                       std::views::drop(2) |
                       std::views::transform([](std::string_view p){ if (p.back() == ',') { p.remove_suffix(1); } return std::string{p}; }) |
                       to<std::vector<std::string>>();
        return {type, std::string{parts[0]}, std::move(outputs)};
    }

    void set_inputs(std::vector<mod>& retval) {
        for (auto& m : retval) {
            for (auto& out : m.outputs) {
                auto found = std::find_if(retval.begin(), retval.end(), [&out](const mod& v){ return v.name == out; });
                if (found != retval.end()) {
                    found->inputs.emplace_back(m.name);
                }
            }
        }
    }

    std::vector<mod> get_input(const std::vector<std::string>& lines) {
        auto retval = lines | std::views::transform(&parse_mod) | to<std::vector<mod>>();
        set_inputs(retval);
        return retval;
    }

    struct pulse {
        std::string src;
        std::string dest;
        bool is_high = false;
    };

    std::pair<int, int> push_button(std::vector<mod>& modules, int press_num) {
        std::deque<pulse> pulses;
        pulses.emplace_back("button", "broadcaster");
        int low_sent = 0, high_sent = 0;
        while (!pulses.empty()) {
            const auto pulse = pulses.front();
            pulses.pop_front();

            low_sent += !pulse.is_high;
            high_sent += pulse.is_high;

            auto found = std::find_if(modules.begin(), modules.end(), [&n = pulse.dest](const mod& m){ return m.name == n; });
            if (found != modules.end()) {
                if (found->type == mod_type::flipflop) {
                    if (!pulse.is_high) {
                        found->is_on = !found->is_on;
                        for (auto& out : found->outputs) {
                            pulses.emplace_back(found->name, out, found->is_on);
                        }
                    }
                }
                else if (found->type == mod_type::conjunction) {
                    auto in = std::find_if(found->inputs.begin(), found->inputs.end(), [n = pulse.src](const input& i){ return i.name == n; });
                    if (in != found->inputs.end()) {
                        in->last_was_high = pulse.is_high;
                        if (std::all_of(found->inputs.begin(), found->inputs.end(),
                                        [](const input &i) { return i.last_was_high; }))
                        {
                            for (auto &out: found->outputs) {
                                pulses.emplace_back(found->name, out, false);
                            }
                        }
                        else {
                            found->num_to_high.push_back(press_num);
                            for (auto &out: found->outputs) {
                                pulses.emplace_back(found->name, out, true);
                            }
                        }
                    }
                }
                else {
                    for (auto& out : found->outputs) {
                        pulses.emplace_back(found->name, out, pulse.is_high);
                    }
                }
            }
        }
        return {low_sent, high_sent};
    }

    /*
    --- Day 20: Pulse Propagation ---
    With your help, the Elves manage to find the right parts and fix all of the machines. Now, they just need to send the command to boot up the machines and get the sand flowing again.

    The machines are far apart and wired together with long cables. The cables don't connect to the machines directly, but rather to communication modules attached to the machines that perform various initialization tasks and also act as communication relays.

    Modules communicate using pulses. Each pulse is either a high pulse or a low pulse. When a module sends a pulse, it sends that type of pulse to each module in its list of destination modules.

    There are several different types of modules:

    Flip-flop modules (prefix %) are either on or off; they are initially off. If a flip-flop module receives a high pulse, it is ignored and nothing happens. However, if a flip-flop module receives a low pulse, it flips between on and off. If it was off, it turns on and sends a high pulse. If it was on, it turns off and sends a low pulse.

    Conjunction modules (prefix &) remember the type of the most recent pulse received from each of their connected input modules; they initially default to remembering a low pulse for each input. When a pulse is received, the conjunction module first updates its memory for that input. Then, if it remembers high pulses for all inputs, it sends a low pulse; otherwise, it sends a high pulse.

    There is a single broadcast module (named broadcaster). When it receives a pulse, it sends the same pulse to all of its destination modules.

    Here at Desert Machine Headquarters, there is a module with a single button on it called, aptly, the button module. When you push the button, a single low pulse is sent directly to the broadcaster module.

    After pushing the button, you must wait until all pulses have been delivered and fully handled before pushing it again. Never push the button if modules are still processing pulses.

    Pulses are always processed in the order they are sent. So, if a pulse is sent to modules a, b, and c, and then module a processes its pulse and sends more pulses, the pulses sent to modules b and c would have to be handled first.

    The module configuration (your puzzle input) lists each module. The name of the module is preceded by a symbol identifying its type, if any. The name is then followed by an arrow and a list of its destination modules. For example:

    broadcaster -> a, b, c
    %a -> b
    %b -> c
    %c -> inv
    &inv -> a
    In this module configuration, the broadcaster has three destination modules named a, b, and c. Each of these modules is a flip-flop module (as indicated by the % prefix). a outputs to b which outputs to c which outputs to another module named inv. inv is a conjunction module (as indicated by the & prefix) which, because it has only one input, acts like an inverter (it sends the opposite of the pulse type it receives); it outputs to a.

    By pushing the button once, the following pulses are sent:

    button -low-> broadcaster
    broadcaster -low-> a
    broadcaster -low-> b
    broadcaster -low-> c
    a -high-> b
    b -high-> c
    c -high-> inv
    inv -low-> a
    a -low-> b
    b -low-> c
    c -low-> inv
    inv -high-> a
    After this sequence, the flip-flop modules all end up off, so pushing the button again repeats the same sequence.

    Here's a more interesting example:

    broadcaster -> a
    %a -> inv, con
    &inv -> b
    %b -> con
    &con -> output
    This module configuration includes the broadcaster, two flip-flops (named a and b), a single-input conjunction module (inv), a multi-input conjunction module (con), and an untyped module named output (for testing purposes). The multi-input conjunction module con watches the two flip-flop modules and, if they're both on, sends a low pulse to the output module.

    Here's what happens if you push the button once:

    button -low-> broadcaster
    broadcaster -low-> a
    a -high-> inv
    a -high-> con
    inv -low-> b
    con -high-> output
    b -high-> con
    con -low-> output
    Both flip-flops turn on and a low pulse is sent to output! However, now that both flip-flops are on and con remembers a high pulse from each of its two inputs, pushing the button a second time does something different:

    button -low-> broadcaster
    broadcaster -low-> a
    a -low-> inv
    a -low-> con
    inv -high-> b
    con -high-> output
    Flip-flop a turns off! Now, con remembers a low pulse from module a, and so it sends only a high pulse to output.

    Push the button a third time:

    button -low-> broadcaster
    broadcaster -low-> a
    a -high-> inv
    a -high-> con
    inv -low-> b
    con -low-> output
    b -low-> con
    con -high-> output
    This time, flip-flop a turns on, then flip-flop b turns off. However, before b can turn off, the pulse sent to con is handled first, so it briefly remembers all high pulses for its inputs and sends a low pulse to output. After that, flip-flop b turns off, which causes con to update its state and send a high pulse to output.

    Finally, with a on and b off, push the button a fourth time:

    button -low-> broadcaster
    broadcaster -low-> a
    a -low-> inv
    a -low-> con
    inv -high-> b
    con -high-> output
    This completes the cycle: a turns off, causing con to remember only low pulses and restoring all modules to their original states.

    To get the cables warmed up, the Elves have pushed the button 1000 times. How many pulses got sent as a result (including the pulses sent by the button itself)?

    In the first example, the same thing happens every time the button is pushed: 8 low pulses and 4 high pulses are sent. So, after pushing the button 1000 times, 8000 low pulses and 4000 high pulses are sent. Multiplying these together gives 32000000.

    In the second example, after pushing the button 1000 times, 4250 low pulses and 2750 high pulses are sent. Multiplying these together gives 11687500.

    Consult your module configuration; determine the number of low pulses and high pulses that would be sent after pushing the button 1000 times, waiting for all pulses to be fully handled after each push of the button. What do you get if you multiply the total number of low pulses sent by the total number of high pulses sent?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        int64_t low = 0, high = 0;
        //Do stupidly
        for (int i = 0; i < 1000; ++i) {
            const auto [ls, hs] = push_button(input, i+1);
            low += ls;
            high += hs;
        }
        const auto res = low * high;
        return std::to_string(res);
    }

    /*
    --- Part Two ---
    The final machine responsible for moving the sand down to Island Island has a module attached named rx. The machine turns on when a single low pulse is sent to rx.

    Reset all modules to their default states. Waiting for all pulses to be fully handled after each button press, what is the fewest number of button presses required to deliver a single low pulse to the module named rx?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        using namespace std::string_literals;
        auto input = get_input(lines);
        const auto pre_end = std::find_if(input.begin(), input.end(), [](const mod& m){ return std::find(m.outputs.begin(), m.outputs.end(), "rx"s) != m.outputs.end(); });
        const auto pre_pre_ends = input |
                std::views::filter([n = pre_end->name](const mod& m){ return std::find(m.outputs.begin(), m.outputs.end(), n) != m.outputs.end(); }) |
                std::views::transform([](mod& m){ return std::ref(m); }) |
                to<std::vector<std::reference_wrapper<mod>>>();
        int count = 0;
        while (std::any_of(pre_pre_ends.begin(), pre_pre_ends.end(), [](const std::reference_wrapper<mod>& m){ return m.get().num_to_high.empty(); })) {
            push_button(input, ++count);
        }
        const auto cycle_lens = pre_pre_ends |
                std::views::transform([](const std::reference_wrapper<mod>& m){ return m.get().num_to_high.front(); }) |
                to<std::vector<int>>();
        int64_t res = 1;
        for (const int v : cycle_lens) {
            res = std::lcm(res, v);
        }
        return std::to_string(res);
    }

    aoc::registration r{2023, 20, part_1, part_2};

    TEST_SUITE("2023_day20") {
        TEST_CASE("2023_day20:example1") {
            using namespace std::string_literals;
            std::vector<std::string> lines {
                    "broadcaster -> a, b, c"s,
                    "%a -> b"s,
                    "%b -> c"s,
                    "%c -> inv"s,
                    "&inv -> a"s
            };
            auto input = lines | std::views::transform(&parse_mod) | to<std::vector<mod>>();
            set_inputs(input);

            int64_t low = 0, high = 0;
            //Do stupidly
            for (int i = 0; i < 1000; ++i) {
                const auto [ls, hs] = push_button(input, i+1);
                low += ls;
                high += hs;
            }
            const auto res = low * high;
            CHECK_EQ(res, 32000000);
        }
        TEST_CASE("2023_day20:example2") {
            using namespace std::string_literals;
            std::vector<std::string> lines {
                    "broadcaster -> a"s,
                    "%a -> inv, con"s,
                    "&inv -> b"s,
                    "%b -> con"s,
                    "&con -> output"s
            };
            auto input = lines | std::views::transform(&parse_mod) | to<std::vector<mod>>();
            set_inputs(input);

            int64_t low = 0, high = 0;
            //Do stupidly
            for (int i = 0; i < 1000; ++i) {
                const auto [ls, hs] = push_button(input, i+1);
                low += ls;
                high += hs;
            }
            const auto res = low * high;
            CHECK_EQ(res, 11687500);
        }
    }

}