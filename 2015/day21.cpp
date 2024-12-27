//
// Created by Dan on 7/2/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int BOSS_HP = 100;
    constexpr int BOSS_DMG = 8;
    constexpr int BOSS_ARM = 2;
    constexpr int MAX_RINGS = 2;

    enum class item_class {
        weapon,
        armor,
        ring
    };

    struct item {
        item_class type;
        int cost = 0;
        int damage = 0;
        int armor = 0;
    };

    constexpr auto WEAPONS = std::array{
        item{item_class::weapon, 8,  4, 0},
        item{item_class::weapon, 10, 5, 0},
        item{item_class::weapon, 25, 6, 0},
        item{item_class::weapon, 40, 7, 0},
        item{item_class::weapon, 74, 8, 0}
    };

    constexpr auto ARMOR = std::array{
        item{item_class::armor,  0,  0, 0},
        item{item_class::armor, 13,  0, 1},
        item{item_class::armor, 31,  0, 2},
        item{item_class::armor, 53,  0, 3},
        item{item_class::armor, 75,  0, 4},
        item{item_class::armor, 102, 0, 5}
    };

    constexpr auto RINGS = std::array{
        item{item_class::ring, 0,   0, 0},
        item{item_class::ring, 0,   0, 0},
        item{item_class::ring, 25,  1, 0},
        item{item_class::ring, 50,  2, 0},
        item{item_class::ring, 100, 3, 0},
        item{item_class::ring, 20,  0, 1},
        item{item_class::ring, 40,  0, 2},
        item{item_class::ring, 80,  0, 3}
    };

    struct boss{};

    class player {
        int hp = 100;
        int damage = 0;
        int armor = 0;
        bool have_weapon = false;
        bool have_armor = false;
        int num_rings = 0;
    public:
        player() = default;
        explicit player(boss) :
            hp{BOSS_HP}, damage{BOSS_DMG}, armor{BOSS_ARM},
            have_weapon{true}, have_armor{true}, num_rings{MAX_RINGS} {}
        player(int h, int d, int a) :
            hp{h}, damage{d}, armor{a},
            have_weapon{true}, have_armor{true}, num_rings{MAX_RINGS} {}

        bool add_item(const item& it) {
            if (it.type == item_class::weapon && !have_weapon) {
                damage += it.damage;
                armor += it.armor;
                have_weapon = true;
                return true;
            }
            else if (it.type == item_class::armor && !have_armor) {
                damage += it.damage;
                armor += it.armor;
                have_armor = true;
                return true;
            }
            else if (it.type == item_class::ring && num_rings < MAX_RINGS) {
                damage += it.damage;
                armor += it.armor;
                ++num_rings;
                return true;
            }
            return false;
        }

        bool defend(const player& other) {
            hp -= std::max(other.damage - armor, 1);
            return hp <= 0;
        }

        [[nodiscard]] int rounds_until_defeat(const player& other) const {
            int att = std::max(other.damage - armor, 1);
            return hp % att == 0 ? hp / att : (hp / att + 1);
        }

        [[nodiscard]] std::pair<int, int> both_rounds_until_defeat(const player& other) const {
            return {rounds_until_defeat(other), other.rounds_until_defeat(*this)};
        }

        [[nodiscard]] bool does_player_win_against(const player& other, bool this_goes_first = true) const {
            const auto [other_win_t, my_win_t] = both_rounds_until_defeat(other);
            return this_goes_first ? my_win_t <= other_win_t : my_win_t < other_win_t;
        }

        [[nodiscard]] std::string str() const { return fmt::format("{{Dmg:{:3}, Arm:{:3}}}", damage, armor); }
    };

    /*
    --- Day 21: RPG Simulator 20XX ---
    Little Henry Case got a new video game for Christmas. It's an RPG, and he's stuck on a boss. He needs to know what equipment to buy at the shop. He hands you the controller.

    In this game, the player (you) and the enemy (the boss) take turns attacking. The player always goes first. Each attack reduces the opponent's hit points by at least 1. The first character at or below 0 hit points loses.

    Damage dealt by an attacker each turn is equal to the attacker's damage score minus the defender's armor score. An attacker always does at least 1 damage. So, if the attacker has a damage score of 8, and the defender has an armor score of 3, the defender loses 5 hit points. If the defender had an armor score of 300, the defender would still lose 1 hit point.

    Your damage score and armor score both start at zero. They can be increased by buying items in exchange for gold. You start with no items and have as much gold as you need. Your total damage or armor is equal to the sum of those stats from all of your items. You have 100 hit points.

    Here is what the item shop is selling:

    Weapons:    Cost  Damage  Armor
    Dagger        8     4       0
    Shortsword   10     5       0
    Warhammer    25     6       0
    Longsword    40     7       0
    Greataxe     74     8       0

    Armor:      Cost  Damage  Armor
    Leather      13     0       1
    Chainmail    31     0       2
    Splintmail   53     0       3
    Bandedmail   75     0       4
    Platemail   102     0       5

    Rings:      Cost  Damage  Armor
    Damage +1    25     1       0
    Damage +2    50     2       0
    Damage +3   100     3       0
    Defense +1   20     0       1
    Defense +2   40     0       2
    Defense +3   80     0       3
    You must buy exactly one weapon; no dual-wielding. Armor is optional, but you can't use more than one. You can buy 0-2 rings (at most one for each hand). You must use any items you buy. The shop only has one of each item, so you can't buy, for example, two rings of Damage +3.

    For example, suppose you have 8 hit points, 5 damage, and 5 armor, and that the boss has 12 hit points, 7 damage, and 2 armor:

    The player deals 5-2 = 3 damage; the boss goes down to 9 hit points.
    The boss deals 7-5 = 2 damage; the player goes down to 6 hit points.
    The player deals 5-2 = 3 damage; the boss goes down to 6 hit points.
    The boss deals 7-5 = 2 damage; the player goes down to 4 hit points.
    The player deals 5-2 = 3 damage; the boss goes down to 3 hit points.
    The boss deals 7-5 = 2 damage; the player goes down to 2 hit points.
    The player deals 5-2 = 3 damage; the boss goes down to 0 hit points.
    In this scenario, the player wins! (Barely.)

    You have 100 hit points. The boss's actual stats are in your puzzle input. What is the least amount of gold you can spend and still win the fight?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        player b{boss{}};
        int min_cost = std::numeric_limits<int>::max();
        for (const auto& w : WEAPONS) {
            for (const auto& a : ARMOR) {
                for (int i = 0; i < RINGS.size(); ++i) {
                    for (int j = i + 1; j < RINGS.size(); ++j) {
                        player p{};
                        p.add_item(w);
                        p.add_item(a);
                        p.add_item(RINGS[i]);
                        p.add_item(RINGS[j]);
                        const auto cost = w.cost + a.cost + RINGS[i].cost + RINGS[j].cost;
                        if (p.does_player_win_against(b) && cost < min_cost) {
                            min_cost = cost;
                        }
                    }
                }
            }
        }
        return std::to_string(min_cost);
    }

    /*
    --- Part Two ---
    Turns out the shopkeeper is working with the boss, and can persuade you to buy whatever items he wants. The other rules still apply, and he still only has one of each item.

    What is the most amount of gold you can spend and still lose the fight?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        player b{boss{}};
        int max_cost = -1;
        for (const auto& w : WEAPONS) {
            for (const auto& a : ARMOR) {
                for (int i = 0; i < RINGS.size(); ++i) {
                    for (int j = i + 1; j < RINGS.size(); ++j) {
                        player p{};
                        p.add_item(w);
                        p.add_item(a);
                        p.add_item(RINGS[i]);
                        p.add_item(RINGS[j]);
                        const auto cost = w.cost + a.cost + RINGS[i].cost + RINGS[j].cost;
                        if (b.does_player_win_against(p, false) && cost > max_cost) {
                            max_cost = cost;
                        }
                    }
                }
            }
        }
        return std::to_string(max_cost);
    }

    aoc::registration r{2015, 21, part_1, part_2};

    TEST_SUITE("2015_day21") {
        TEST_CASE("2015_day21:example") {
            player p {8, 5, 5}, boss {12, 7, 2};
            CHECK_FALSE(boss.defend(p));
            CHECK_FALSE(p.defend(boss));
            CHECK_FALSE(boss.defend(p));
            CHECK_FALSE(p.defend(boss));
            CHECK_FALSE(boss.defend(p));
            CHECK_FALSE(p.defend(boss));
            CHECK(boss.defend(p));
        }
        TEST_CASE("2015_day21:case1") {
            player p {100, 8, 0}, b {boss{}};
            CHECK_FALSE(p.does_player_win_against(b));
            CHECK(b.does_player_win_against(p, false));
        }
        TEST_CASE("2015_day21:case2") {
            player p {100, 9, 0}, b {boss{}};
            CHECK_FALSE(p.does_player_win_against(b));
            CHECK(b.does_player_win_against(p, false));
        }
        TEST_CASE("2015_day21:case3") {
            player p {100, 10, 0}, b {boss{}};
            CHECK(p.does_player_win_against(b));
            CHECK_FALSE(b.does_player_win_against(p, false));
        }
        TEST_CASE("2015_day21:case4") {
            player p {100, 11, 0}, b {boss{}};
            CHECK(p.does_player_win_against(b));
            CHECK_FALSE(b.does_player_win_against(p, false));
        }
    }

}