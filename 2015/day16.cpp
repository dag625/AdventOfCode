//
// Created by Dan on 12/26/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <regex>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct sue {
        int id;
        std::optional<int> children;
        std::optional<int> cats;
        std::optional<int> samoyeds;
        std::optional<int> pomeranians;
        std::optional<int> akitas;
        std::optional<int> vizslas;
        std::optional<int> goldfish;
        std::optional<int> trees;
        std::optional<int> cars;
        std::optional<int> perfumes;
    };

    struct readout {
        int children;
        int cats;
        int samoyeds;
        int pomeranians;
        int akitas;
        int vizslas;
        int goldfish;
        int trees;
        int cars;
        int perfumes;
    };

    bool operator==(const sue& s, const readout& r) noexcept {
        return (!s.children.has_value() || *s.children == r.children) &&
                (!s.cats.has_value() || *s.cats == r.cats) &&
                (!s.samoyeds.has_value() || *s.samoyeds == r.samoyeds) &&
                (!s.pomeranians.has_value() || *s.pomeranians == r.pomeranians) &&
                (!s.akitas.has_value() || *s.akitas == r.akitas) &&
                (!s.vizslas.has_value() || *s.vizslas == r.vizslas) &&
                (!s.goldfish.has_value() || *s.goldfish == r.goldfish) &&
                (!s.trees.has_value() || *s.trees == r.trees) &&
                (!s.cars.has_value() || *s.cars == r.cars) &&
                (!s.perfumes.has_value() || *s.perfumes == r.perfumes);
    }

    bool part_2_find(const sue& s, const readout& r) noexcept {
        return (!s.children.has_value() || *s.children == r.children) &&
               (!s.cats.has_value() || *s.cats > r.cats) &&
               (!s.samoyeds.has_value() || *s.samoyeds == r.samoyeds) &&
               (!s.pomeranians.has_value() || *s.pomeranians < r.pomeranians) &&
               (!s.akitas.has_value() || *s.akitas == r.akitas) &&
               (!s.vizslas.has_value() || *s.vizslas == r.vizslas) &&
               (!s.goldfish.has_value() || *s.goldfish < r.goldfish) &&
               (!s.trees.has_value() || *s.trees > r.trees) &&
               (!s.cars.has_value() || *s.cars == r.cars) &&
               (!s.perfumes.has_value() || *s.perfumes == r.perfumes);
    }

    constexpr readout OUTPUT = {3, 7, 2, 3, 0, 0, 5, 3, 2, 1};

    sue parse_sue(std::string_view s) {
        using namespace std::string_view_literals;
        using svmatch = std::match_results<std::string_view::const_iterator>;
        const std::regex re {R"(Sue (\d+): (.+))"};
        sue retval;
        svmatch match;
        if (!std::regex_match(s.begin(), s.end(), match, re)) {
            throw std::runtime_error{fmt::format("Invalid Sue line:  {}", s)};
        }
        retval.id = parse<int>(match[1].str());
        const auto parts_str = match[2].str();
        const auto parts = split(parts_str, ", "sv);
        for (const auto& p : parts) {
            const auto nv = split(p, ": "sv);
            const auto num = parse<int>(nv[1]);
            if (nv[0] == "children"sv) {
                retval.children = num;
            }
            else if (nv[0] == "cats"sv) {
                retval.cats = num;
            }
            else if (nv[0] == "samoyeds"sv) {
                retval.samoyeds = num;
            }
            else if (nv[0] == "pomeranians"sv) {
                retval.pomeranians = num;
            }
            else if (nv[0] == "akitas"sv) {
                retval.akitas = num;
            }
            else if (nv[0] == "vizslas"sv) {
                retval.vizslas = num;
            }
            else if (nv[0] == "goldfish"sv) {
                retval.goldfish = num;
            }
            else if (nv[0] == "trees"sv) {
                retval.trees = num;
            }
            else if (nv[0] == "cars"sv) {
                retval.cars = num;
            }
            else if (nv[0] == "perfumes"sv) {
                retval.perfumes = num;
            }
            else {
                throw std::runtime_error{fmt::format("Unknown Sue field:  {}", nv[0])};
            }
        }
        return retval;
    }

    std::vector<sue> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2015" / "day_16_input.txt");
        return lines | std::views::transform([](std::string_view s){ return parse_sue(s); }) | to<std::vector<sue>>();
    }

    /*
    --- Day 16: Aunt Sue ---
    Your Aunt Sue has given you a wonderful gift, and you'd like to send her a thank you card. However, there's a small problem: she signed it "From, Aunt Sue".

    You have 500 Aunts named "Sue".

    So, to avoid sending the card to the wrong person, you need to figure out which Aunt Sue (which you conveniently number 1 to 500, for sanity) gave you the gift. You open the present and, as luck would have it, good ol' Aunt Sue got you a My First Crime Scene Analysis Machine! Just what you wanted. Or needed, as the case may be.

    The My First Crime Scene Analysis Machine (MFCSAM for short) can detect a few specific compounds in a given sample, as well as how many distinct kinds of those compounds there are. According to the instructions, these are what the MFCSAM can detect:

    children, by human DNA age analysis.
    cats. It doesn't differentiate individual breeds.
    Several seemingly random breeds of dog: samoyeds, pomeranians, akitas, and vizslas.
    goldfish. No other kinds of fish.
    trees, all in one group.
    cars, presumably by exhaust or gasoline or something.
    perfumes, which is handy, since many of your Aunts Sue wear a few kinds.
    In fact, many of your Aunts Sue have many of these. You put the wrapping from the gift into the MFCSAM. It beeps inquisitively at you a few times and then prints out a message on ticker tape:

    children: 3
    cats: 7
    samoyeds: 2
    pomeranians: 3
    akitas: 0
    vizslas: 0
    goldfish: 5
    trees: 3
    cars: 2
    perfumes: 1
    You make a list of the things you can remember about each Aunt Sue. Things missing from your list aren't zero - you simply don't remember the value.

    What is the number of the Sue that got you the gift?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        auto found = std::find(input.begin(), input.end(), OUTPUT);
        if (found == input.end()) {
            throw std::runtime_error{fmt::format("Failed to find a matching Sue.")};
        }
        return std::to_string(found->id);
    }

    /*
    --- Part Two ---
    As you're about to send the thank you note, something in the MFCSAM's instructions catches your eye. Apparently, it has an outdated retroencabulator, and so the output from the machine isn't exact values - some of them indicate ranges.

    In particular, the cats and trees readings indicates that there are greater than that many (due to the unpredictable nuclear decay of cat dander and tree pollen), while the pomeranians and goldfish readings indicate that there are fewer than that many (due to the modial interaction of magnetoreluctance).

    What is the number of the real Aunt Sue?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        auto found = std::find_if(input.begin(), input.end(), [](const sue& s){ return part_2_find(s, OUTPUT); });
        if (found == input.end()) {
            throw std::runtime_error{fmt::format("Failed to find a matching Sue.")};
        }
        return std::to_string(found->id);
    }

    aoc::registration r {2015, 16, part_1, part_2};

//    TEST_SUITE("2015_day16") {
//        TEST_CASE("2015_day16:example") {
//
//        }
//    }

}