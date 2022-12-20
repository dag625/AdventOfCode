//
// Created by Dan on 12/19/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <optional>
#include <map>

#include "utilities.h"
#include "parse.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr std::size_t ORE_R_IDX = 0;
    constexpr std::size_t CLAY_R_IDX = 1;
    constexpr std::size_t OBSDN_R_IDX = 2;
    constexpr std::size_t GEO_R_IDX = 3;
    constexpr std::size_t NONE_R_IDX = 4;

    constexpr std::size_t ORE_M_IDX = 0;
    constexpr std::size_t CLAY_M_IDX = 1;
    constexpr std::size_t OBSDN_M_IDX = 2;

    using blueprint = std::array<std::array<int, 3>, 4>;

    //Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 22 ore. Each obsidian robot costs 33 ore and 14 clay. Each geode robot costs 22 ore and 77 obsidian.
    //0         1  2    3   4     5     6 7    8    9    10    11    12 13   14   15       16    17    18 19  20  21 22    23   24    25    26    27 28  29  30 31
    blueprint parse_blueprint(std::string_view s) {
        const auto parts = split(s, ' ');
        blueprint retval{};
        retval[ORE_R_IDX][ORE_M_IDX] = parse<int>(parts[6]);
        retval[CLAY_R_IDX][ORE_M_IDX] = parse<int>(parts[12]);
        retval[OBSDN_R_IDX][ORE_M_IDX] = parse<int>(parts[18]);
        retval[OBSDN_R_IDX][CLAY_M_IDX] = parse<int>(parts[21]);
        retval[GEO_R_IDX][ORE_M_IDX] = parse<int>(parts[27]);
        retval[GEO_R_IDX][OBSDN_M_IDX] = parse<int>(parts[30]);
        return retval;
    }

    std::vector<blueprint> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2022" / "day_19_input.txt");
        return lines | std::views::transform(parse_blueprint) | to<std::vector<blueprint>>();
    }

    struct resource {
        int rate = 0;
        int total = 0;
    };

    struct status {
        std::array<resource, 4> resources{};
        int minute = 24;

        struct init{};
        status() = default;
        explicit status(init) { resources[ORE_R_IDX].rate = 1; }

        std::strong_ordering operator<=>(const status& rhs) const noexcept {
            const auto rval = std::lexicographical_compare_three_way(resources.begin(), resources.end(),
                                                           rhs.resources.begin(), rhs.resources.end(),
                                                           [](const resource& a, const resource& b){ return a.rate <=> b.rate; });
            if (rval == std::strong_ordering::equal) {
                return minute <=> rhs.minute;
            }
            else {
                return rval;
            }
        }
    };

    void mine(status& state) {
        for (auto& r : state.resources) {
            r.total += r.rate;
        }
    }

    std::optional<status> check_build(const blueprint& bp, const status& current, const int idx) {
        status next = current;
        if (idx == NONE_R_IDX) {
            --next.minute;
            if (next.minute <= 0) {
                return std::nullopt;
            }
            mine(next);
            return next;
        }
        int time_to_build = -1;
        bool missing_resource = false;
        for (int i = 0; i < bp[idx].size(); ++i) {
            if (bp[idx][i] != 0) {
                int time_for_res = -1;
                if (current.resources[i].rate > 0) {
                    if (current.resources[i].total >= bp[idx][i]) {
                        time_for_res = 1;
                    }
                    else {
                        const int needed = bp[idx][i] - current.resources[i].total;
                        const auto res = std::div(needed, current.resources[i].rate);
                        time_for_res = res.quot + (res.rem > 0 ? 1 : 0);
                    }
                }
                else {
                    missing_resource = true;
                    break;
                }
                if (time_for_res >= 0 && time_for_res > time_to_build) {
                    time_to_build = time_for_res;
                }
            }
        }
        if (!missing_resource && (next.minute - time_to_build) > 0) {
            next.minute -= time_to_build;
            for (int i = 0; i < time_to_build; ++i) {
                mine(next);
            }
            ++next.resources[idx].rate;
            for (int i = 0; i < bp[idx].size(); ++i) {
                next.resources[i].total -= bp[idx][i];
            }
            return next;
        }
        else if (!missing_resource) {
            while (next.minute > 0) {
                mine(next);
                --next.minute;
            }
            return next;
        }
        else {
            return std::nullopt;
        }
    }

    int check_recursive_impl(const blueprint& bp, const status& current, std::map<status, int>& cache, const std::array<int, 3>& max_r, const int other_best, const int num_minutes) {
        const auto found = cache.find(current);
        if (found != cache.end()) {
            return found->second;
        }
        if (other_best > current.resources[GEO_R_IDX].total + (current.minute * current.minute + current.minute) / 2) {
            return 0;
        }
        int best = 0;
        for (int i = NONE_R_IDX; i >= 0; --i) {
            if (i < GEO_R_IDX && current.resources[i].rate >= max_r[i]) {
                continue;
            }
            auto r = check_build(bp, current, i);
            if (r) {
                if (r->minute == 0) {
                    const int geo = r->resources[GEO_R_IDX].total;
                    if (geo > best) {
                        best = geo;
                    }
                }
                else {
                    const int res = check_recursive_impl(bp, *r, cache, max_r, std::max(best, other_best), num_minutes);
                    if (res > best) {
                        best = res;
                    }
                }
            }
        }
        cache[current] = best;
        return best;
    }

    int check_recursive(const blueprint& bp, const int num_minutes) {
        status start {status::init{}};
        std::array<int, 3> max_r{};
        max_r[ORE_M_IDX] = std::max_element(bp.begin(), bp.end(), [](const std::array<int, 3>& a, const std::array<int, 3>& b){ return a[ORE_R_IDX] < b[ORE_R_IDX]; })->at(0);
        max_r[CLAY_M_IDX] = bp[OBSDN_R_IDX][CLAY_M_IDX];
        max_r[OBSDN_M_IDX] = bp[GEO_R_IDX][OBSDN_M_IDX];
        std::map<status, int> cache;
        const auto retval = check_recursive_impl(bp, start, cache, max_r, 0, num_minutes);
        return retval;
    }

    /*

    */
    std::string part_1(const std::filesystem::path &input_dir) {
        auto input = get_input(input_dir);
        int result = 0;
        for (int id = 1; id <= input.size(); ++id) {
            result += check_recursive(input[id - 1], 24) * id;
        }
        return std::to_string(result);
    }

    /*

    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    aoc::registration r{2022, 19, part_1, part_2};

    TEST_SUITE("2022_day19") {
        TEST_CASE("2022_day19:example") {
            std::vector<std::string> lines {
                "Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.",
                "Blueprint 2: Each ore robot costs 2 ore. Each clay robot costs 3 ore. Each obsidian robot costs 3 ore and 8 clay. Each geode robot costs 3 ore and 12 obsidian."
            };
            auto input = lines | std::views::transform(parse_blueprint) | to<std::vector<blueprint>>();;
            int result = 0;
            for (int id = 1; id <= input.size(); ++id) {
                const auto res = check_recursive(input[id - 1], 24);
                result += res * id;
            }
            CHECK_EQ(result, 33);
        }
    }

}