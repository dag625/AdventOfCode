//
// Created by Daniel Garcia on 12/7/20.
//

#include "registration.h"
#include "utilities.h"

#include <string>
#include <string_view>
#include <vector>
#include <regex>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        struct bag_requirement {
            std::string name;
            int num;
        };

        struct bag_type {
            std::string name;
            std::vector<bag_requirement> reqs;
        };

        bag_type parse_bag_type(const std::string& s) {
            static std::regex re1 {R"#(^([a-z]+ [a-z]+) bags contain )#"};
            static std::regex re2 {R"#((\d+) ([a-z]+ [a-z]+) bag[s]?[,.])#"};
            std::smatch m1;
            if (!std::regex_search(s, m1, re1)) {
                throw std::runtime_error{"Failed to parse bag type from line which didn't match."};
            }
            else if (m1.size() < 2) {
                throw std::runtime_error{"Failed to parse bag type from line which had too few groups."};
            }
            std::string name = m1[1].str();

            std::vector<bag_requirement> reqs;
            const auto mend = std::regex_iterator<std::string::const_iterator>{};
            for (auto it = std::regex_iterator{s.begin(), s.end(), re2}; it != mend; ++it) {
                if (it->size() < 3) {
                    throw std::runtime_error{"Failed to parse bag type from line which had too few groups for requirements."};
                }
                reqs.push_back({(*it)[2].str(), std::stoi((*it)[1])});
            }
            return { name, reqs };
        }

        std::vector<bag_type> get_input(const fs::path& input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_7_input.txt");
            std::vector<bag_type> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_bag_type);
            return retval;
        }

        std::vector<std::string> get_ancestors(const std::vector<bag_type>& bags, std::string_view name) {
            std::vector<std::string> retval;
            for (const auto& b : bags) {
                if (auto found = std::find_if(b.reqs.begin(), b.reqs.end(), [name](const bag_requirement& r){ return r.name == name; });
                    found != b.reqs.end())
                {
                    retval.push_back(b.name);
                    auto children = get_ancestors(bags, b.name);
                    std::move(children.begin(), children.end(), std::back_inserter(retval));
                }
            }
            std::sort(retval.begin(), retval.end());
            retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
            return retval;
        }

        int64_t count_descendants(const std::vector<bag_type>& bags, std::string_view name) {
            int64_t retval = 0;
            auto found = std::find_if(bags.begin(), bags.end(), [name](const bag_type& b){ return b.name == name; });
            if (found == bags.end()) {
                throw std::runtime_error{"Failed to find bag for descendants."};
            }
            for (const auto& r : found->reqs) {
                retval += r.num * (count_descendants(bags, r.name) + 1);
            }
            return retval;
        }

    }

    /*
    You land at the regional airport in time for your next flight. In fact, it looks like you'll even have time to grab some food: all flights are currently delayed due to issues in luggage processing.

    Due to recent aviation regulations, many rules (your puzzle input) are being enforced about bags and their contents; bags must be color-coded and must contain specific quantities of other color-coded bags. Apparently, nobody responsible for these regulations considered how long they would take to enforce!

    For example, consider the following rules:

    light red bags contain 1 bright white bag, 2 muted yellow bags.
    dark orange bags contain 3 bright white bags, 4 muted yellow bags.
    bright white bags contain 1 shiny gold bag.
    muted yellow bags contain 2 shiny gold bags, 9 faded blue bags.
    shiny gold bags contain 1 dark olive bag, 2 vibrant plum bags.
    dark olive bags contain 3 faded blue bags, 4 dotted black bags.
    vibrant plum bags contain 5 faded blue bags, 6 dotted black bags.
    faded blue bags contain no other bags.
    dotted black bags contain no other bags.
    These rules specify the required contents for 9 bag types. In this example, every faded blue bag is empty, every vibrant plum bag contains 11 bags (5 faded blue and 6 dotted black), and so on.

    You have a shiny gold bag. If you wanted to carry it in at least one other bag, how many different bag colors would be valid for the outermost bag? (In other words: how many colors can, eventually, contain at least one shiny gold bag?)

    In the above rules, the following options would be available to you:

    A bright white bag, which can hold your shiny gold bag directly.
    A muted yellow bag, which can hold your shiny gold bag directly, plus some other bags.
    A dark orange bag, which can hold bright white and muted yellow bags, either of which could then hold your shiny gold bag.
    A light red bag, which can hold bright white and muted yellow bags, either of which could then hold your shiny gold bag.
    So, in this example, the number of bag colors that can eventually contain at least one shiny gold bag is 4.

    How many bag colors can eventually contain at least one shiny gold bag? (The list of rules is quite long; make sure you get all of it.)
    */
    std::string solve_day_7_1(const std::filesystem::path& input_dir) {
        return std::to_string(get_ancestors(get_input(input_dir), "shiny gold").size());
    }

    /*
    It's getting pretty expensive to fly these days - not because of ticket prices, but because of the ridiculous number of bags you need to buy!

    Consider again your shiny gold bag and the rules from the above example:

    faded blue bags contain 0 other bags.
    dotted black bags contain 0 other bags.
    vibrant plum bags contain 11 other bags: 5 faded blue bags and 6 dotted black bags.
    dark olive bags contain 7 other bags: 3 faded blue bags and 4 dotted black bags.
    So, a single shiny gold bag must contain 1 dark olive bag (and the 7 bags within it) plus 2 vibrant plum bags (and the 11 bags within each of those): 1 + 1*7 + 2 + 2*11 = 32 bags!

    Of course, the actual rules have a small chance of going several levels deeper than this example; be sure to count all of the bags, even if the nesting becomes topologically impractical!

    Here's another example:

    shiny gold bags contain 2 dark red bags.
    dark red bags contain 2 dark orange bags.
    dark orange bags contain 2 dark yellow bags.
    dark yellow bags contain 2 dark green bags.
    dark green bags contain 2 dark blue bags.
    dark blue bags contain 2 dark violet bags.
    dark violet bags contain no other bags.
    In this example, a single shiny gold bag must contain 126 other bags.

    How many individual bags are required inside your single shiny gold bag?
    */
    std::string solve_day_7_2(const std::filesystem::path& input_dir) {
        return std::to_string(count_descendants(get_input(input_dir), "shiny gold"));
    }

    static aoc::registration r {2020, 7, solve_day_7_1, solve_day_7_2};

} /* namespace aoc2020 */