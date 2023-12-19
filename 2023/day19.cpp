//
// Created by Dan on 12/18/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <functional>
#include <deque>

#include "utilities.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    enum class workflow_result {
        next,
        workflow,
        accept,
        reject
    };

    struct part {
        int xcellent = 0;//x
        int musical = 0;//m
        int aero = 0;//a
        int shiny = 0;//s

        [[nodiscard]] int rating() const { return xcellent + musical + aero + shiny; }
    };

    using part_field = int (part::*);

    struct filter_result {
        part min;
        part max;
        workflow_result result;
        std::string next_name;
        int next_index = 0;
    };

    using workflow_action = std::function<workflow_result(const part&, std::string&)>;
    using workflow_filter = std::function<std::vector<filter_result>(const part&, const part&)>;

    struct workflow {
        std::string name;
        std::vector<workflow_action> actions;
        std::vector<workflow_filter> filters;
    };

    std::pair<workflow_action, workflow_filter> parse_action(std::string_view s, const std::string& wf_name, int wf_idx) {
        using namespace std::string_view_literals;
        const auto p3 = split(s, ':');
        if (p3.size() == 1) {
            if (p3[0] == "A"sv) {
                return {[](const part& p, std::string& n){ return workflow_result::accept; },
                        [](const part& min, const part& max){ return std::vector<filter_result>{{min, max, workflow_result::accept}}; }};
            }
            else if (p3[0] == "R"sv) {
                return {[](const part& p, std::string& n){ return workflow_result::reject; },
                        [](const part& min, const part& max){ return std::vector<filter_result>{{min, max, workflow_result::reject}}; }};
            }
            else {
                std::string name {p3[0]};
                return {[name](const part& p, std::string& n){ n = name; return workflow_result::workflow; },
                        [name](const part& min, const part& max){ return std::vector<filter_result>{{min, max, workflow_result::workflow, name, 0}}; }};
            }
        }
        else {
            part_field field = nullptr;
            switch (p3[0][0]) {
                case 'x': field = &part::xcellent; break;
                case 'm': field = &part::musical; break;
                case 'a': field = &part::aero; break;
                case 's': field = &part::shiny; break;
                default: break;
            }

            auto num_str = p3[0];
            num_str.remove_prefix(2);
            const int val = parse<int>(num_str);

            std::function<bool(int)> act_op;
            std::function<filter_result(const part&, const part&, workflow_result, const std::string&, int)> filt_t_op;
            std::function<filter_result(const part&, const part&, workflow_result, const std::string&, int)> filt_f_op;
            switch (p3[0][1]) {
                case '<':
                    act_op = [b = val](int a){ return a < b; };
                    filt_t_op = [val, field](const part& min, const part& max, workflow_result r, const std::string& n, int i){
                        filter_result retval{min, max, r, n, i};
                        retval.min.*field = min.*field;
                        retval.max.*field = std::min(val - 1, max.*field);
                        return retval;
                    };
                    filt_f_op = [val, field](const part& min, const part& max, workflow_result r, const std::string& n, int i){
                        filter_result retval{min, max, r, n, i};
                        retval.min.*field = std::max(val, min.*field);
                        retval.max.*field = max.*field;
                        return retval;
                    };
                    break;
                case '>':
                    act_op = [b = val](int a){ return a > b; };
                    filt_t_op = [val, field](const part& min, const part& max, workflow_result r, const std::string& n, int i){
                        filter_result retval{min, max, r, n, i};
                        retval.min.*field = std::max(val + 1, min.*field);
                        retval.max.*field = max.*field;
                        return retval;
                    };
                    filt_f_op = [val, field](const part& min, const part& max, workflow_result r, const std::string& n, int i){
                        filter_result retval{min, max, r, n, i};
                        retval.min.*field = min.*field;
                        retval.max.*field = std::min(val, max.*field);
                        return retval;
                    };
                    break;
                default: break;
            }

            if (p3[1] == "A"sv) {
                return {[field, act_op](const part& p, std::string& n){
                    if (act_op(p.*field)) {
                        return workflow_result::accept;
                    }
                    else {
                        return workflow_result::next;
                    }
                },
                [filt_t_op, filt_f_op, wf_name, wf_idx](const part& min, const part& max){
                    return std::vector<filter_result>{
                        filt_t_op(min, max, workflow_result::accept, wf_name, wf_idx),
                        filt_f_op(min, max, workflow_result::next, wf_name, wf_idx + 1)
                    };
                }};
            }
            else if (p3[1] == "R"sv) {
                return {[field, act_op](const part& p, std::string& n){
                    if (act_op(p.*field)) {
                        return workflow_result::reject;
                    }
                    else {
                        return workflow_result::next;
                    }
                },
                [filt_t_op, filt_f_op, wf_name, wf_idx](const part& min, const part& max){
                    return std::vector<filter_result>{
                        filt_t_op(min, max, workflow_result::reject, wf_name, wf_idx),
                        filt_f_op(min, max, workflow_result::next, wf_name, wf_idx + 1)
                    };
                }};
            }
            else {
                std::string name {p3[1]};
                return {[field, act_op, name](const part& p, std::string& n){
                    if (act_op(p.*field)) {
                        n = name;
                        return workflow_result::workflow;
                    }
                    else {
                        return workflow_result::next;
                    }
                },
                [filt_t_op, filt_f_op, name, wf_name, wf_idx](const part& min, const part& max){
                    return std::vector<filter_result>{
                            filt_t_op(min, max, workflow_result::workflow, name, 0),
                            filt_f_op(min, max, workflow_result::next, wf_name, wf_idx + 1)
                    };
                }};
            }
        }
    }

    workflow parse_workflow(std::string_view s) {
        using namespace std::string_view_literals;
        auto p1 = split(s, '{');
        std::string name {p1[0]};
        p1[1].remove_suffix(1);
        const auto p2 = split(p1[1], ',');
        std::vector<workflow_action> actions;
        std::vector<workflow_filter> filters;
        int idx = 0;
        for (const auto item : p2) {
            auto [act, filt] = parse_action(item, name, idx);
            actions.push_back(std::move(act));
            filters.push_back(std::move(filt));
            ++idx;
        }
        return {name, std::move(actions), std::move(filters)};
    }

    int get_val_for_part(std::string_view s) {
        const auto parts = split(s, '=');
        return parse<int>(parts[1]);
    }

    part parse_part(std::string_view s) {
        s.remove_prefix(1);
        s.remove_suffix(1);
        const auto parts = split(s, ',');
        return {get_val_for_part(parts[0]), get_val_for_part(parts[1]), get_val_for_part(parts[2]), get_val_for_part(parts[3])};
    }

    std::pair<std::vector<workflow>, std::vector<part>> get_input(const fs::path &input_dir) {
        using namespace std::string_literals;
        const auto lines = read_file_lines(input_dir / "2023" / "day_19_input.txt");
        const auto empty = std::find(lines.begin(), lines.end(), ""s);
        std::pair<std::vector<workflow>, std::vector<part>> retval{};
        std::transform(lines.begin(), empty, std::back_inserter(retval.first), &parse_workflow);
        std::transform(empty + 1, lines.end(), std::back_inserter(retval.second), &parse_part);
        return retval;
    }

    const workflow& find_workflow(const std::string& n, const std::vector<workflow>& workflows) {
        const auto found = std::find_if(workflows.begin(), workflows.end(), [&n](const workflow& w){ return w.name == n; });
        return *found;
    }

    workflow_result run_workflow(const part& p, const workflow& w, std::string& n) {
        for (const auto& act : w.actions) {
            std::string name;
            const auto res = act(p, name);
            if (res == workflow_result::workflow) {
                n = name;
            }
            if (res != workflow_result::next) {
                return res;
            }
        }
        return workflow_result::reject;//Shouldn't get here.
    }

    bool run_workflows(const part& p, const std::vector<workflow>& workflows) {
        workflow_result result = workflow_result::workflow;
        std::string next_wf = "in";
        while (result == workflow_result::workflow) {
            result = run_workflow(p, find_workflow(next_wf, workflows), next_wf);
        }
        return result == workflow_result::accept;
    }

    int64_t count_rejected(const std::vector<std::pair<part, part>>& rr) {
        int64_t retval = 0;
        for (const auto& r : rr) {
            retval += static_cast<int64_t>(r.second.xcellent - r.first.xcellent + 1) *
                    static_cast<int64_t>(r.second.musical - r.first.musical + 1) *
                    static_cast<int64_t>(r.second.aero - r.first.aero + 1) *
                    static_cast<int64_t>(r.second.shiny - r.first.shiny + 1);
        }
        return retval;
    }

    int64_t filter_workflows(const std::vector<workflow>& workflows) {
        std::deque<filter_result> queue;
        part min{1, 1, 1, 1};
        part max{4000, 4000, 4000, 4000};
        queue.emplace_back(min, max, workflow_result::next, "in", 0);
        std::vector<std::pair<part, part>> rejected_ranges;
        while (!queue.empty()) {
            auto next = queue.front();
            queue.erase(queue.begin());
            if (next.result == workflow_result::next || next.result == workflow_result::workflow) {
                const auto &wf = find_workflow(next.next_name, workflows);
                auto filters = wf.filters[next.next_index](next.min, next.max);
                queue.insert(queue.end(), filters.begin(), filters.end());
            }
            else if (next.result == workflow_result::reject) {
                rejected_ranges.emplace_back(next.min, next.max);
            }
        }
        return count_rejected(rejected_ranges);
    }

    /*
    --- Day 19: Aplenty ---
    The Elves of Gear Island are thankful for your help and send you on your way. They even have a hang glider that someone stole from Desert Island; since you're already going that direction, it would help them a lot if you would use it to get down there and return it to them.

    As you reach the bottom of the relentless avalanche of machine parts, you discover that they're already forming a formidable heap. Don't worry, though - a group of Elves is already here organizing the parts, and they have a system.

    To start, each part is rated in each of four categories:

    x: Extremely cool looking
    m: Musical (it makes a noise when you hit it)
    a: Aerodynamic
    s: Shiny
    Then, each part is sent through a series of workflows that will ultimately accept or reject the part. Each workflow has a name and contains a list of rules; each rule specifies a condition and where to send the part if the condition is true. The first rule that matches the part being considered is applied immediately, and the part moves on to the destination described by the rule. (The last rule in each workflow has no condition and always applies if reached.)

    Consider the workflow ex{x>10:one,m<20:two,a>30:R,A}. This workflow is named ex and contains four rules. If workflow ex were considering a specific part, it would perform the following steps in order:

    Rule "x>10:one": If the part's x is more than 10, send the part to the workflow named one.
    Rule "m<20:two": Otherwise, if the part's m is less than 20, send the part to the workflow named two.
    Rule "a>30:R": Otherwise, if the part's a is more than 30, the part is immediately rejected (R).
    Rule "A": Otherwise, because no other rules matched the part, the part is immediately accepted (A).
    If a part is sent to another workflow, it immediately switches to the start of that workflow instead and never returns. If a part is accepted (sent to A) or rejected (sent to R), the part immediately stops any further processing.

    The system works, but it's not keeping up with the torrent of weird metal shapes. The Elves ask if you can help sort a few parts and give you the list of workflows and some part ratings (your puzzle input). For example:

    px{a<2006:qkq,m>2090:A,rfg}
    pv{a>1716:R,A}
    lnx{m>1548:A,A}
    rfg{s<537:gd,x>2440:R,A}
    qs{s>3448:A,lnx}
    qkq{x<1416:A,crn}
    crn{x>2662:A,R}
    in{s<1351:px,qqz}
    qqz{s>2770:qs,m<1801:hdj,R}
    gd{a>3333:R,R}
    hdj{m>838:A,pv}

    {x=787,m=2655,a=1222,s=2876}
    {x=1679,m=44,a=2067,s=496}
    {x=2036,m=264,a=79,s=2244}
    {x=2461,m=1339,a=466,s=291}
    {x=2127,m=1623,a=2188,s=1013}
    The workflows are listed first, followed by a blank line, then the ratings of the parts the Elves would like you to sort. All parts begin in the workflow named in. In this example, the five listed parts go through the following workflows:

    {x=787,m=2655,a=1222,s=2876}: in -> qqz -> qs -> lnx -> A
    {x=1679,m=44,a=2067,s=496}: in -> px -> rfg -> gd -> R
    {x=2036,m=264,a=79,s=2244}: in -> qqz -> hdj -> pv -> A
    {x=2461,m=1339,a=466,s=291}: in -> px -> qkq -> crn -> R
    {x=2127,m=1623,a=2188,s=1013}: in -> px -> rfg -> A
    Ultimately, three parts are accepted. Adding up the x, m, a, and s rating for each of the accepted parts gives 7540 for the part with x=787, 4623 for the part with x=2036, and 6951 for the part with x=2127. Adding all of the ratings for all of the accepted parts gives the sum total of 19114.

    Sort through all of the parts you've been given; what do you get if you add together all of the rating numbers for all of the parts that ultimately get accepted?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto [workflows, parts] = get_input(input_dir);
        int64_t sum = 0;
        for (const auto& p : parts) {
            if (run_workflows(p, workflows)) {
                sum += p.rating();
            }
        }
        return std::to_string(sum);
    }

    /*
    --- Part Two ---
    Even with your help, the sorting process still isn't fast enough.

    One of the Elves comes up with a new plan: rather than sort parts individually through all of these workflows, maybe you can figure out in advance which combinations of ratings will be accepted or rejected.

    Each of the four ratings (x, m, a, s) can have an integer value ranging from a minimum of 1 to a maximum of 4000. Of all possible distinct combinations of ratings, your job is to figure out which ones will be accepted.

    In the above example, there are 167409079868000 distinct combinations of ratings that will be accepted.

    Consider only your list of workflows; the list of part ratings that the Elves wanted you to sort is no longer relevant. How many distinct combinations of ratings will be accepted by the Elves' workflows?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto [workflows, parts] = get_input(input_dir);
        const auto rejected = filter_workflows(workflows);
        const auto accepted = (4000ll * 4000ll * 4000ll * 4000ll) - rejected;
        return std::to_string(accepted);
    }

    aoc::registration r{2023, 19, part_1, part_2};

    TEST_SUITE("2023_day19") {
        TEST_CASE("2023_day19:example") {
            using namespace std::string_literals;
            std::vector<std::string> wfstrs {
                    "px{a<2006:qkq,m>2090:A,rfg}"s,
                    "pv{a>1716:R,A}"s,
                    "lnx{m>1548:A,A}"s,
                    "rfg{s<537:gd,x>2440:R,A}"s,
                    "qs{s>3448:A,lnx}"s,
                    "qkq{x<1416:A,crn}"s,
                    "crn{x>2662:A,R}"s,
                    "in{s<1351:px,qqz}"s,
                    "qqz{s>2770:qs,m<1801:hdj,R}"s,
                    "gd{a>3333:R,R}"s,
                    "hdj{m>838:A,pv}"s
            };
            std::vector<std::string> pstrs {
                    "{x=787,m=2655,a=1222,s=2876}"s,
                    "{x=1679,m=44,a=2067,s=496}"s,
                    "{x=2036,m=264,a=79,s=2244}"s,
                    "{x=2461,m=1339,a=466,s=291}"s,
                    "{x=2127,m=1623,a=2188,s=1013}"s
            };
            std::vector<workflow> workflows;
            std::vector<part> parts;
            std::transform(wfstrs.begin(), wfstrs.end(), std::back_inserter(workflows), &parse_workflow);
            std::transform(pstrs.begin(), pstrs.end(), std::back_inserter(parts), &parse_part);


            const auto rejected = filter_workflows(workflows);
            const auto accepted = (4000ll * 4000ll * 4000ll * 4000ll) - rejected;
            CHECK_EQ(accepted, 167409079868000ll);
        }
    }

}