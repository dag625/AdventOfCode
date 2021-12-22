//
// Created by Dan on 12/22/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <regex>

#include "utilities.h"
#include "point_nd.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    using point_3d = point<3>;

    using cell = point_3d;

    bool operator==(const cell& lhs, const cell& rhs) noexcept {
        return std::tie(lhs[0], lhs[1], lhs[2]) == std::tie(rhs[0], rhs[1], rhs[2]);
    }

    bool operator<(const cell& lhs, const cell& rhs) noexcept {
        return std::tie(lhs[0], lhs[1], lhs[2]) < std::tie(rhs[0], rhs[1], rhs[2]);
    }

    bool in(const cell& start, const cell& end, const cell& p) noexcept {
        return p[0] >= start[0] && p[0] <= end[0] &&
                p[1] >= start[1] && p[1] <= end[1] &&
                p[2] >= start[2] && p[2] <= end[2];
    }

    enum class toggle : bool {
        off = false,
        on = true
    };

    struct step {
        toggle state;
        point_3d start;
        point_3d end;
    };

    step parse_step(std::string_view s) {
        using namespace std::string_view_literals;
        static std::regex re {R"((on|off) x=(-?\d+)..(-?\d+),y=(-?\d+)..(-?\d+),z=(-?\d+)..(-?\d+))"};
        std::match_results<std::string_view::const_iterator> res;
        if (!std::regex_match(s.begin(), s.end(), res, re)) {
            throw std::runtime_error{fmt::format("Line did not match expected format:  {}", s)};
        }
        toggle state = static_cast<toggle>(res[1].str() == "on"sv);
        int x1 = parse<int>(res[2].str());
        int x2 = parse<int>(res[3].str());
        int y1 = parse<int>(res[4].str());
        int y2 = parse<int>(res[5].str());
        int z1 = parse<int>(res[6].str());
        int z2 = parse<int>(res[7].str());
        return {state,
                {std::min(x1, x2), std::min(y1, y2), std::min(z1, z2)},
                {std::max(x1, x2), std::max(y1, y2), std::max(z1, z2)}};
    }

    std::vector<step> get_input(const fs::path& input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_22_input.txt");
        return lines | std::views::transform(&parse_step) | to<std::vector<step>>();
    }

    bool step_covers_part1_region(const step& s) {
        constexpr int min_coord = -50, max_coord = 50;
        for (int i = 0; i < 3; ++i) {
            bool does_range_cover = (s.start[i] >= min_coord && s.start[i] <= max_coord) ||
                    (s.start[i] >= min_coord && s.start[i] <= max_coord) ||
                    (s.start[i] < min_coord && s.end[i] > max_coord);
            if (!does_range_cover) {
                return false;
            }
        }
        return true;
    }

    /*

    */
    std::string part_1(const std::filesystem::path& input_dir) {
        constexpr int min_coord = -50, max_coord = 50;
        constexpr int region_size = max_coord - min_coord + 1;
        const auto input = get_input(input_dir);
        std::vector<cell> states;
        states.reserve(region_size * region_size);
        for (const auto& s : input) {
            if (step_covers_part1_region(s)) {
                if (s.state == toggle::on) {
                    const auto orig_size = static_cast<ptrdiff_t>(states.size());
                    for (int x = std::max(s.start[0], min_coord); x <= std::min(s.end[0], max_coord); ++x) {
                        for (int y = std::max(s.start[1], min_coord); y <= std::min(s.end[1], max_coord); ++y) {
                            for (int z = std::max(s.start[2], min_coord); z <= std::min(s.end[2], max_coord); ++z) {
                                states.push_back({x, y, z});
                            }
                        }
                    }
                    std::inplace_merge(states.begin(), states.begin() + orig_size, states.end());
                    states.erase(std::unique(states.begin(), states.end()), states.end());
                }
                else {
                    states.erase(std::remove_if(states.begin(), states.end(),
                                                [&s](const cell& p){ return in(s.start, s.end, p); }
                                                ), states.end());
                }
            }
        }
        return std::to_string(states.size());
    }

    /*

    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);

        return std::to_string(-1);
    }

    aoc::registration r {2021, 22, part_1, part_2};

    TEST_SUITE("2021_day22") {
        TEST_CASE("2021_day22:example") {
            std::vector<std::string> lines = {
                    "on x=10..12,y=10..12,z=10..12",
                    "on x=11..13,y=11..13,z=11..13",
                    "off x=9..11,y=9..11,z=9..11",
                    "on x=10..10,y=10..10,z=10..10"
            };
            const auto input = lines | std::views::transform(&parse_step) | to<std::vector<step>>();
            const std::vector<std::size_t> step_sizes = {
                    27,
                    46,
                    38,
                    39
            };
            int idx = 0;

            constexpr int min_coord = -50, max_coord = 50;
            constexpr int region_size = max_coord - min_coord + 1;
            std::vector<cell> states;
            states.reserve(region_size * region_size);
            for (const auto& s : input) {
                if (step_covers_part1_region(s)) {
                    if (s.state == toggle::on) {
                        const auto orig_size = static_cast<ptrdiff_t>(states.size());
                        for (int x = std::max(s.start[0], min_coord); x <= std::min(s.end[0], max_coord); ++x) {
                            for (int y = std::max(s.start[1], min_coord); y <= std::min(s.end[1], max_coord); ++y) {
                                for (int z = std::max(s.start[2], min_coord); z <= std::min(s.end[2], max_coord); ++z) {
                                    states.push_back({x, y, z});
                                }
                            }
                        }
                        std::inplace_merge(states.begin(), states.begin() + orig_size, states.end());
                        states.erase(std::unique(states.begin(), states.end()), states.end());
                    }
                    else {
                        states.erase(std::remove_if(states.begin(), states.end(),
                                                    [&s](const cell& p){ return in(s.start, s.end, p); }
                        ), states.end());
                    }
                }
                REQUIRE_EQ(states.size(), step_sizes[idx++]);
            }
        }
    }

}