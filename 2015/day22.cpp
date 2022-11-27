//
// Created by Dan on 7/2/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <utility>
#include <vector>
#include <array>
#include <string_view>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr int BOSS_HP = 58;
    constexpr int BOSS_DMG = 9;
    constexpr int BOSS_ARM = 0;
    constexpr int PLAYER_HP = 50;
    constexpr int PLAYER_MANA = 500;

    struct effect {
        int id = 0;
        int turns = 0;
        int damage = 0;
        int armor = 0;
        int mana = 0;
    };

    struct magic_attack {
        int mana_cost = 0;
        int inst_damage = 0;
        int inst_heal = 0;
        int effect_id = 0;
        int effect_turns = 0;
        int effect_damage = 0;
        int effect_armor = 0;
        int effect_mana = 0;
        std::string_view name;

        [[nodiscard]] bool is_effect() const { return effect_turns > 0; }

        [[nodiscard]] effect get_effect() const {
            return {effect_id, effect_turns, effect_damage, effect_armor, effect_mana};
        }
    };

    constexpr auto MAGIC_ATTACKS = std::array{
            magic_attack{53, 4, 0, 0, 0, 0, 0, 0,    "Magic Missile"sv},
            magic_attack{73, 2, 2, 0, 0, 0, 0, 0,    "Drain"sv},
            magic_attack{113, 0, 0, 1, 6, 0, 7, 0,   "Shield"sv},
            magic_attack{173, 0, 0, 2, 6, 3, 0, 0,   "Poison"sv},
            magic_attack{229, 0, 0, 3, 5, 0, 0, 101, "Recharge"sv}
    };

    struct boss {
    };

    enum class attack_result {
        keep_going,
        win,
        lose,
        not_enough_mana,
        effect_in_progress
    };

}

template<>
struct fmt::formatter<attack_result>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const attack_result& res, FormatContext& ctx) {
        switch (res) {
            case attack_result::win:
                return fmt::format_to(ctx.out(), "Win");
            case attack_result::lose:
                return fmt::format_to(ctx.out(), "Lose");
            case attack_result::keep_going:
                return fmt::format_to(ctx.out(), "Continue");
            case attack_result::effect_in_progress:
                return fmt::format_to(ctx.out(), "Invalid (Effect in Progress)");
            case attack_result::not_enough_mana:
                return fmt::format_to(ctx.out(), "Invalid (Not Enough Mana)");
            default:
                return fmt::format_to(ctx.out(), "Invalid (Unknown {})", static_cast<int>(res));
        }
    }
};

namespace {

    class player {
        int hp = PLAYER_HP;
        int damage = 0;
        int armor = 0;
        int mana = PLAYER_MANA;
        int mana_spent = 0;
        bool has_magic = true;
        std::vector<effect> effects;
        std::vector<magic_attack> attacks;

        [[nodiscard]] int net_armor() const {
            int retval = armor;
            if (has_magic) {
                for (const auto &e: effects) {
                    retval += e.armor;
                }
            }
            return retval;
        }
        void apply_effects(player& other) {
            for (auto& e : effects) {
                other.hp -= e.damage;
                mana += e.mana;
                --e.turns;
            }
            effects.erase(std::remove_if(effects.begin(), effects.end(),
                                         [](const effect& e){ return e.turns <= 0; }),
                          effects.end());
        }
    public:
        player() = default;
        explicit player(boss) :
            hp{BOSS_HP}, damage{BOSS_DMG}, armor{BOSS_ARM}, mana{0}, has_magic{false} {}
        player(int h, int d, int a, bool m, int n) :
            hp{h}, damage{d}, armor{a}, has_magic{m}, mana{n} {}

        [[nodiscard]] attack_result attack(player& other) {
            other.apply_effects(*this);
            if (hp <= 0) {
                return attack_result::lose;
            }
            other.hp -= std::max(damage - other.net_armor(), 1);
            return other.hp <= 0 ? attack_result::win : attack_result::keep_going;
        }

        [[nodiscard]] attack_result attack(player& other, const magic_attack& magic) {
            if (has_magic) {
                if (mana < magic.mana_cost) {
                    return attack_result::not_enough_mana;
                }
                else if (magic.is_effect() && std::any_of(effects.begin(), effects.end(),
                                                          [&magic](const effect& e){ return e.id == magic.effect_id && e.turns > 1; }))
                {
                    return attack_result::effect_in_progress;
                }
                attacks.push_back(magic);
                apply_effects(other);
                mana -= magic.mana_cost;
                mana_spent += magic.mana_cost;
                if (magic.is_effect()) {
                    effects.push_back(magic.get_effect());
                }
                else {
                    other.hp -= magic.inst_damage;
                    hp += magic.inst_heal;
                }
                return other.hp <= 0 ? attack_result::win : attack_result::keep_going;
            }
            else {
                return attack(other);
            }
        }

        [[nodiscard]] bool apply_poison(int dmg) {
            hp -= dmg;
            return hp <= 0;
        }

        [[nodiscard]] int total_mana_spent() const { return mana_spent; }
        [[nodiscard]] bool is_magic() const { return has_magic; }
        [[nodiscard]] std::string str() const { return fmt::format("{{Magic:{:>3}, HP: {:3}, Mana:{:4}, Arm:{:3}}}", (has_magic ? "Yes" : "No"), hp, mana, armor); }
        [[nodiscard]] std::string magic_attacks_list() const { return fmt::format("{}", fmt::join(attacks | std::views::transform([](const magic_attack& a) { return a.name; }), ", ")); }
    };

    int play(player p1, player p2, int magic_user_poison, int& mana_limit, bool return_first = true, const int round = 0) {
        //fmt::print(" {:<4}:  {} attacks {}\n", round, p1.str(), p2.str());
        const auto next_round = return_first ? round : round + 1;
        if (p1.is_magic()) {
            if (p1.apply_poison(magic_user_poison)) {
                return std::numeric_limits<int>::max();
            }
            int best_att = std::numeric_limits<int>::max();
            for (const auto& att : MAGIC_ATTACKS) {
                if (att.mana_cost + p1.total_mana_spent() > mana_limit) {
                    continue;
                }
                auto p1_tmp = p1;
                auto p2_tmp = p2;
                const auto res = p1_tmp.attack(p2_tmp, att);
                //fmt::print("r{:<4}:  Result of {} attacks {} with {:<14} -> {}\n", round, p1_tmp.str(), p2_tmp.str(), att.name, res);
                if (res == attack_result::keep_going) {
                    const int mana = play(p2_tmp, p1_tmp, magic_user_poison, mana_limit, !return_first, next_round);
                    if (mana < best_att) {
                        best_att = mana;
                    }
                }
                else if (res == attack_result::win) {
                    const int mana = return_first ? p1_tmp.total_mana_spent() : std::numeric_limits<int>::max();
                    if (return_first) {
                        //fmt::print("w{:<4}:  Spent {} mana using:  {}\n", round, mana, p1_tmp.magic_attacks_list());
                    }
                    if (mana < best_att) {
                        best_att = mana;
                    }
                }
                if (best_att < mana_limit) {
                    mana_limit = best_att;
                }
            }
            return best_att;
        }
        else {
            const auto res = p1.attack(p2);
            //fmt::print("r{:<4}:  Result of {} attacks {} -> {}\n", round, p1.str(), p2.str(), res);
            if (res == attack_result::keep_going) {
                return play(p2, p1, magic_user_poison, mana_limit, !return_first, next_round);
            }
            else if (res == attack_result::lose) {
                return !return_first ? p2.total_mana_spent() : std::numeric_limits<int>::max();
            }
            else if (res == attack_result::win) {
                return return_first ? p1.total_mana_spent() : std::numeric_limits<int>::max();
            }
            else {
                return std::numeric_limits<int>::max();
            }
        }
    }

    int play(player p1, player p2, int magic_user_poison) {
        int mana_limit = std::numeric_limits<int>::max();
        return play(std::move(p1), std::move(p2), magic_user_poison, mana_limit);
    }

    /*
    --- Day 22: Wizard Simulator 20XX ---
    Little Henry Case decides that defeating bosses with swords and stuff is boring. Now he's playing the game with a wizard. Of course, he gets stuck on another boss and needs your help again.

    In this version, combat still proceeds with the player and the boss taking alternating turns. The player still goes first. Now, however, you don't get any equipment; instead, you must choose one of your spells to cast. The first character at or below 0 hit points loses.

    Since you're a wizard, you don't get to wear armor, and you can't attack normally. However, since you do magic damage, your opponent's armor is ignored, and so the boss effectively has zero armor as well. As before, if armor (from a spell, in this case) would reduce damage below 1, it becomes 1 instead - that is, the boss' attacks always deal at least 1 damage.

    On each of your turns, you must select one of your spells to cast. If you cannot afford to cast any spell, you lose. Spells cost mana; you start with 500 mana, but have no maximum limit. You must have enough mana to cast a spell, and its cost is immediately deducted when you cast it. Your spells are Magic Missile, Drain, Shield, Poison, and Recharge.

    Magic Missile costs 53 mana. It instantly does 4 damage.
    Drain costs 73 mana. It instantly does 2 damage and heals you for 2 hit points.
    Shield costs 113 mana. It starts an effect that lasts for 6 turns. While it is active, your armor is increased by 7.
    Poison costs 173 mana. It starts an effect that lasts for 6 turns. At the start of each turn while it is active, it deals the boss 3 damage.
    Recharge costs 229 mana. It starts an effect that lasts for 5 turns. At the start of each turn while it is active, it gives you 101 new mana.
    Effects all work the same way. Effects apply at the start of both the player's turns and the boss' turns. Effects are created with a timer (the number of turns they last); at the start of each turn, after they apply any effect they have, their timer is decreased by one. If this decreases the timer to zero, the effect ends. You cannot cast a spell that would start an effect which is already active. However, effects can be started on the same turn they end.

    For example, suppose the player has 10 hit points and 250 mana, and that the boss has 13 hit points and 8 damage:

    -- Player turn --
    - Player has 10 hit points, 0 armor, 250 mana
    - Boss has 13 hit points
    Player casts Poison.

    -- Boss turn --
    - Player has 10 hit points, 0 armor, 77 mana
    - Boss has 13 hit points
    Poison deals 3 damage; its timer is now 5.
    Boss attacks for 8 damage.

    -- Player turn --
    - Player has 2 hit points, 0 armor, 77 mana
    - Boss has 10 hit points
    Poison deals 3 damage; its timer is now 4.
    Player casts Magic Missile, dealing 4 damage.

    -- Boss turn --
    - Player has 2 hit points, 0 armor, 24 mana
    - Boss has 3 hit points
    Poison deals 3 damage. This kills the boss, and the player wins.
    Now, suppose the same initial conditions, except that the boss has 14 hit points instead:

    -- Player turn --
    - Player has 10 hit points, 0 armor, 250 mana
    - Boss has 14 hit points
    Player casts Recharge.

    -- Boss turn --
    - Player has 10 hit points, 0 armor, 21 mana
    - Boss has 14 hit points
    Recharge provides 101 mana; its timer is now 4.
    Boss attacks for 8 damage!

    -- Player turn --
    - Player has 2 hit points, 0 armor, 122 mana
    - Boss has 14 hit points
    Recharge provides 101 mana; its timer is now 3.
    Player casts Shield, increasing armor by 7.

    -- Boss turn --
    - Player has 2 hit points, 7 armor, 110 mana
    - Boss has 14 hit points
    Shield's timer is now 5.
    Recharge provides 101 mana; its timer is now 2.
    Boss attacks for 8 - 7 = 1 damage!

    -- Player turn --
    - Player has 1 hit point, 7 armor, 211 mana
    - Boss has 14 hit points
    Shield's timer is now 4.
    Recharge provides 101 mana; its timer is now 1.
    Player casts Drain, dealing 2 damage, and healing 2 hit points.

    -- Boss turn --
    - Player has 3 hit points, 7 armor, 239 mana
    - Boss has 12 hit points
    Shield's timer is now 3.
    Recharge provides 101 mana; its timer is now 0.
    Recharge wears off.
    Boss attacks for 8 - 7 = 1 damage!

    -- Player turn --
    - Player has 2 hit points, 7 armor, 340 mana
    - Boss has 12 hit points
    Shield's timer is now 2.
    Player casts Poison.

    -- Boss turn --
    - Player has 2 hit points, 7 armor, 167 mana
    - Boss has 12 hit points
    Shield's timer is now 1.
    Poison deals 3 damage; its timer is now 5.
    Boss attacks for 8 - 7 = 1 damage!

    -- Player turn --
    - Player has 1 hit point, 7 armor, 167 mana
    - Boss has 9 hit points
    Shield's timer is now 0.
    Shield wears off, decreasing armor by 7.
    Poison deals 3 damage; its timer is now 4.
    Player casts Magic Missile, dealing 4 damage.

    -- Boss turn --
    - Player has 1 hit point, 0 armor, 114 mana
    - Boss has 2 hit points
    Poison deals 3 damage. This kills the boss, and the player wins.
    You start with 50 hit points and 500 mana points. The boss's actual stats are in your puzzle input. What is the least amount of mana you can spend and still win the fight? (Do not include mana recharge effects as "spending" negative mana.)
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        player p {}, b{boss{}};
        return std::to_string(play(p, b, 0));
    }

    /*
    --- Part Two ---
    On the next run through the game, you increase the difficulty to hard.

    At the start of each player turn (before any other effects apply), you lose 1 hit point. If this brings you to or below 0 hit points, you lose.

    With the same starting stats for you and the boss, what is the least amount of mana you can spend and still win the fight?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        player p {}, b{boss{}};
        return std::to_string(play(p, b, 1));
    }

    aoc::registration r{2015, 22, part_1, part_2};

    TEST_SUITE("2015_day22") {
        TEST_CASE("2015_day22:example1") {
            player p {10, 0, 0, true, 250}, b{13, 8, 0, false, 0};
            const auto res = play(p, b, 0);
            CHECK_EQ(res, 226);
        }
    }

}