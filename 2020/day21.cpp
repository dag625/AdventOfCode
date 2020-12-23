//
// Created by Daniel Garcia on 12/22/20.
//

#include "day21.h"
#include "utilities.h"

#include <doctest/doctest.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <ostream>
#include <array>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace std::string_view_literals;

    namespace {

        struct food {
            std::vector<std::string> ingredients;
            std::vector<std::string> allergens;
        };

        struct allergen_info {
            std::string_view name;
            std::vector<std::string_view> ingredients;

            explicit allergen_info(std::string_view n) : name{n}, ingredients{} {}
            allergen_info(std::string_view n, std::vector<std::string_view> pi) : name{n}, ingredients{std::move(pi)} {}
        };

        bool operator==(const allergen_info& ai, std::string_view name) {
            return ai.name == name;
        }

        bool operator< (const allergen_info& a, const allergen_info& b) {
            return a.name < b.name;
        }

        std::vector<allergen_info> get_allergen_info(const std::vector<food>& foods) {
            std::vector<allergen_info> allergens;
            for (const auto& f : foods) {
                for (const auto& a : f.allergens) {
                    auto found = std::find(allergens.begin(), allergens.end(), a);
                    if (found != allergens.end()) {
                        std::vector<std::string_view> ingred_intersect;
                        std::set_intersection(f.ingredients.begin(), f.ingredients.end(),
                                              found->ingredients.begin(), found->ingredients.end(),
                                              std::back_inserter(ingred_intersect));
                        found->ingredients.swap(ingred_intersect);
                    }
                    else {
                        std::vector<std::string_view> pi;
                        pi.reserve(f.ingredients.size());
                        std::transform(f.ingredients.begin(), f.ingredients.end(), std::back_inserter(pi), [](std::string_view s){ return s; });
                        allergens.emplace_back(a, std::move(pi));
                    }
                }
            }
            return allergens;
        }

        std::ostream& operator<<(std::ostream& os, const allergen_info& ai) {
            os << ai.name << ":  ";
            bool first = true;
            for (const auto& pi : ai.ingredients) {
                if (!first) {
                    os << ", ";
                }
                first = false;
                os << pi;
            }
            return os;
        }

        food parse_food(std::string_view line) {
            constexpr auto CONTAINS = "contains"sv;
            auto in_end = line.find('(');
            if (in_end == std::string_view::npos) {
                throw std::runtime_error{"No allergens list for food."};
            }
            auto in_list = aoc::split_no_empty(line.substr(0, in_end), ' ');
            auto al_list_str = line.substr(in_end + CONTAINS.size() + 1);
            al_list_str.remove_suffix(1);
            auto al_list = aoc::split_no_empty(al_list_str, ',');
            food retval;
            retval.ingredients.reserve(in_list.size());
            retval.allergens.reserve(al_list.size());
            std::transform(in_list.begin(), in_list.end(), std::back_inserter(retval.ingredients), [](std::string_view s){ return std::string{aoc::trim(s)}; });
            std::transform(al_list.begin(), al_list.end(), std::back_inserter(retval.allergens), [](std::string_view s){ return std::string{aoc::trim(s)}; });
            std::sort(retval.ingredients.begin(), retval.ingredients.end());
            return retval;
        }

        std::vector<food> get_input(const fs::path &input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_21_input.txt");
            std::vector<food> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_food);
            return retval;
        }

    }

    /*
    You reach the train's last stop and the closest you can get to your vacation island without getting wet. There aren't even any boats here, but nothing can stop you now: you build a raft. You just need a few days' worth of food for your journey.

    You don't speak the local language, so you can't read any ingredients lists. However, sometimes, allergens are listed in a language you do understand. You should be able to use this information to determine which ingredient contains which allergen and work out which foods are safe to take with you on your trip.

    You start by compiling a list of foods (your puzzle input), one food per line. Each line includes that food's ingredients list followed by some or all of the allergens the food contains.

    Each allergen is found in exactly one ingredient. Each ingredient contains zero or one allergen. Allergens aren't always marked; when they're listed (as in (contains nuts, shellfish) after an ingredients list), the ingredient that contains each listed allergen will be somewhere in the corresponding ingredients list. However, even if an allergen isn't listed, the ingredient that contains that allergen could still be present: maybe they forgot to label it, or maybe it was labeled in a language you don't know.

    For example, consider the following list of foods:

    mxmxvkd kfcds sqjhc nhms (contains dairy, fish)
    trh fvjkl sbzzf mxmxvkd (contains dairy)
    sqjhc fvjkl (contains soy)
    sqjhc mxmxvkd sbzzf (contains fish)
    The first food in the list has four ingredients (written in a language you don't understand): mxmxvkd, kfcds, sqjhc, and nhms. While the food might contain other allergens, a few allergens the food definitely contains are listed afterward: dairy and fish.

    The first step is to determine which ingredients can't possibly contain any of the allergens in any food in your list. In the above example, none of the ingredients kfcds, nhms, sbzzf, or trh can contain an allergen. Counting the number of times any of these ingredients appear in any ingredients list produces 5: they all appear once each except sbzzf, which appears twice.

    Determine which ingredients cannot possibly contain any of the allergens in your list. How many times do any of those ingredients appear?
    */
    void solve_day_21_1(const std::filesystem::path& input_dir) {
        const auto foods = get_input(input_dir);
        std::vector<allergen_info> allergens = get_allergen_info(foods);
        std::vector<std::string_view> unsafe_ingredients;
        for (const auto& a : allergens) {
            const auto mid = unsafe_ingredients.size();
            unsafe_ingredients.insert(unsafe_ingredients.end(), a.ingredients.begin(), a.ingredients.end());
            std::inplace_merge(unsafe_ingredients.begin(), unsafe_ingredients.begin() + mid, unsafe_ingredients.end());
        }
        unsafe_ingredients.erase(std::unique(unsafe_ingredients.begin(), unsafe_ingredients.end()), unsafe_ingredients.end());
        int safe_count = 0;
        for (const auto& f : foods) {
            safe_count += std::count_if(f.ingredients.begin(), f.ingredients.end(), [&unsafe_ingredients](std::string_view in){
                return std::find(unsafe_ingredients.begin(), unsafe_ingredients.end(), in) == unsafe_ingredients.end();
            });
        }
        std::cout << '\t' << safe_count << '\n';
    }

    /*
    Now that you've isolated the inert ingredients, you should have enough information to figure out which ingredient contains which allergen.

    In the above example:

    mxmxvkd contains dairy.
    sqjhc contains fish.
    fvjkl contains soy.
    Arrange the ingredients alphabetically by their allergen and separate them by commas to produce your canonical dangerous ingredient list. (There should not be any spaces in your canonical dangerous ingredient list.) In the above example, this would be mxmxvkd,sqjhc,fvjkl.

    Time to stock your raft with supplies. What is your canonical dangerous ingredient list?
    */
    void solve_day_21_2(const std::filesystem::path& input_dir) {
        const auto foods = get_input(input_dir);
        std::vector<allergen_info> allergens = get_allergen_info(foods);

        auto order = [](const allergen_info& a, const allergen_info& b){ return a.ingredients.size() < b.ingredients.size(); };
        std::sort(allergens.begin(), allergens.end(), order);
        for (auto current = allergens.begin(); current != allergens.end(); ++current) {
            for (auto other = current + 1; other != allergens.end(); ++other) {
                other->ingredients.erase(
                        std::remove(other->ingredients.begin(), other->ingredients.end(), current->ingredients.front()),
                        other->ingredients.end());
            }
            std::sort(current + 1, allergens.end(), order);
        }

        std::sort(allergens.begin(), allergens.end());
        std::cout << '\t' << aoc::join(allergens.begin(), allergens.end(), ',', [](const allergen_info& ai){ return ai.ingredients.front(); }) << '\n';
    }

    TEST_SUITE("day21" * doctest::description("Tests for day 20 challenges.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day21:example" * doctest::description("Testing with input from challenge example.")) {
            constexpr auto input = std::array{
                "mxmxvkd kfcds sqjhc nhms (contains dairy, fish)"sv,
                "trh fvjkl sbzzf mxmxvkd (contains dairy)"sv,
                "sqjhc fvjkl (contains soy)"sv,
                "sqjhc mxmxvkd sbzzf (contains fish)"sv
            };
            std::vector<food> foods;
            foods.reserve(input.size());
            std::transform(input.begin(), input.end(), std::back_inserter(foods), parse_food);
            auto allergens = get_allergen_info(foods);
            for (const auto& a : allergens) {
                std::cout << a << '\n';
            }
            CHECK(true);
        }
    }

} /* namespace aoc2020 */