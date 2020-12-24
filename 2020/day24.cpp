//
// Created by Daniel Garcia on 12/24/20.
//

#include "registration.h"
#include "utilities.h"
#include "point.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <string_view>
#include <array>
#include <iostream>

namespace fs = std::filesystem;

namespace  {

    using namespace aoc;

    constexpr auto EAST = velocity{1, -1};
    constexpr auto SOUTHEAST = velocity{0, -1};
    constexpr auto SOUTHWEST = velocity{-1, 0};
    constexpr auto WEST = velocity{-1, 1};
    constexpr auto NORTHWEST = velocity{0, 1};
    constexpr auto NORTHEAST = velocity{1, 0};
    constexpr auto NEIGHBORS = std::array{EAST, SOUTHEAST, SOUTHWEST, WEST, NORTHWEST, NORTHEAST};

    position parse_position(std::string_view s) {
        position retval;
        while (!s.empty()) {
            if (aoc::starts_with(s, "e")) {
                retval += EAST;
                s.remove_prefix(1);
            }
            else if (aoc::starts_with(s, "ne")) {
                retval += NORTHEAST;
                s.remove_prefix(2);
            }
            else if (aoc::starts_with(s, "se")) {
                retval += SOUTHEAST;
                s.remove_prefix(2);
            }
            else if (aoc::starts_with(s, "w")) {
                retval += WEST;
                s.remove_prefix(1);
            }
            else if (aoc::starts_with(s, "nw")) {
                retval += NORTHWEST;
                s.remove_prefix(2);
            }
            else if (aoc::starts_with(s, "sw")) {
                retval += SOUTHWEST;
                s.remove_prefix(2);
            }
            else {
                throw std::runtime_error{"Invalid direction."};
            }
        }
        return retval;
    }

    struct status {
        position pos;
        bool is_black;
        status(position p, bool isb) : pos{p}, is_black{isb} {}
        void toggle() { is_black = !is_black; }

        bool operator==(const status& rhs) const noexcept { return pos == rhs.pos; }
        bool operator==(position rhs) const noexcept { return pos == rhs; }
        bool operator<(const status& rhs) const noexcept { return pos < rhs.pos; }
        bool operator<(position rhs) const noexcept { return pos < rhs; }
    };

    inline bool operator<(position lhs, const status& rhs) noexcept {
        return lhs < rhs.pos;
    }

    int count_black_neighbors(const position p, const std::vector<status>& tiles) {
        int count = 0;
        for (const auto& dir : NEIGHBORS) {
            const auto n = p + dir;
            auto found = std::lower_bound(tiles.begin(), tiles.end(), n);
            if (found != tiles.end() && found->pos == n && found->is_black) {
                ++count;
            }
        }
        return count;
    }

    void display(const std::vector<status>& tiles);

    void iterate(std::vector<status>& tiles) {
        auto edit_copy = tiles;
        for (const auto& t : tiles) {
            if (t.is_black) {
                for (const auto& dir : NEIGHBORS) {
                    const auto n = t.pos + dir;
                    auto found = std::lower_bound(edit_copy.begin(), edit_copy.end(), n);
                    if (found == edit_copy.end() || found->pos != n) {
                        edit_copy.emplace(found, n, false);
                    }
                }
            }
        }
        for (auto& t : edit_copy) {
            auto num_black = count_black_neighbors(t.pos, tiles);
            if (t.is_black && (num_black == 0 || num_black > 2)) {
                t.is_black = false;
            }
            else if (!t.is_black && num_black == 2) {
                t.is_black = true;
            }
        }
        edit_copy.swap(tiles);
    }

    position to_doublewidth(position p) {
        return { p.x - p.y, -p.x - p.y };
    }

    [[maybe_unused]] void display(const std::vector<status>& tiles) {
        std::vector<status> grid;
        grid.reserve(tiles.size());
        int min_x = 0, min_y = 0, max_x = 0, max_y = 0;
        std::transform(tiles.begin(), tiles.end(), std::back_inserter(grid),
                       [&min_x, &min_y, &max_x, &max_y](const status& t){
                            auto dw_pos = to_doublewidth(t.pos);
                            if (dw_pos.x < min_x) { min_x = dw_pos.x; }
                            if (dw_pos.y < min_y) { min_y = dw_pos.y; }
                            if (dw_pos.x > max_x) { max_x = dw_pos.x; }
                            if (dw_pos.y > max_y) { max_y = dw_pos.y; }
                            return status{ dw_pos, t.is_black };
                        });
        std::sort(grid.begin(), grid.end(), [](const status& a, const status& b){
            return std::tie(a.pos.y, a.pos.x) < std::tie(b.pos.y, b.pos.x);
        });
        auto next = grid.begin();
        for (int y = min_y; y <= max_y; ++y) {
            auto offset = abs(y % 2);
            for (int x = min_x; x <= max_x; ++x) {
                if (abs(x % 2) == offset) {
                    if (next->pos == position{x, y}) {
                        if (next->is_black) {
                            std::cout << '#';
                        }
                        else {
                            std::cout << '.';
                        }
                        ++next;
                    }
                    else {
                        std::cout << ',';
                    }
                }
                else {
                    std::cout << '_';
                }
                std::cout.flush();
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    std::vector<position> get_input(const fs::path &input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2020" / "day_24_input.txt");
        std::vector<position> retval;
        retval.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_position);
        return retval;
    }

    std::vector<status> to_status(const std::vector<position>& init_data) {
        std::vector<status> retval;
        retval.reserve(init_data.size());
        for (const auto p : init_data) {
            auto found = std::find(retval.begin(), retval.end(), p);
            if (found != retval.end()) {
                found->toggle();
            }
            else {
                retval.emplace_back(p, true);
            }
        }
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    std::string part_1(const fs::path& input_dir) {
        auto tiles = to_status(get_input(input_dir));
        return std::to_string(std::count_if(tiles.begin(), tiles.end(), [](const status& s){ return s.is_black; }));
    }

    std::string part_2(const fs::path& input_dir) {
        auto tiles = to_status(get_input(input_dir));
        for (int i = 0; i < 100; ++i) {
            iterate(tiles);
        }
        return std::to_string(std::count_if(tiles.begin(), tiles.end(), [](const status& t){ return t.is_black; }));
    }

    registration r {2020, 24, part_1, part_2};

    TEST_SUITE("day24" * doctest::description("Tests for the challenges for day 24.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day24:example" * doctest::description("Testing my solution on the example given in the challenge.")) {
            const auto lines = std::array{
                "sesenwnenenewseeswwswswwnenewsewsw"sv,
                "neeenesenwnwwswnenewnwwsewnenwseswesw"sv,
                "seswneswswsenwwnwse"sv,
                "nwnwneseeswswnenewneswwnewseswneseene"sv,
                "swweswneswnenwsewnwneneseenw"sv,
                "eesenwseswswnenwswnwnwsewwnwsene"sv,
                "sewnenenenesenwsewnenwwwse"sv,
                "wenwwweseeeweswwwnwwe"sv,
                "wsweesenenewnwwnwsenewsenwwsesesenwne"sv,
                "neeswseenwwswnwswswnw"sv,
                "nenwswwsewswnenenewsenwsenwnesesenew"sv,
                "enewnwewneswsewnwswenweswnenwsenwsw"sv,
                "sweneswneswneneenwnewenewwneswswnese"sv,
                "swwesenesewenwneswnwwneseswwne"sv,
                "enesenwswwswneneswsenwnewswseenwsese"sv,
                "wnwnesenesenenwwnenwsewesewsesesew"sv,
                "nenewswnwewswnenesenwnesewesw"sv,
                "eneswnwswnwsenenwnwnwwseeswneewsenese"sv,
                "neswnwewnwnwseenwseesewsenwsweewe"sv,
                "wseweeenwnesenwwwswnew"sv
            };
            std::vector<position> positions;
            positions.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(positions), parse_position);
            auto tiles = to_status(positions);
            //Part 1:
                    REQUIRE_EQ(tiles.size(), 15);
                    REQUIRE_EQ(std::count_if(tiles.begin(), tiles.end(), [](const status& s){ return s.is_black; }), 10);

            //Part 2:
            for (int i = 0; i < 100; ++i) {
                iterate(tiles);
            }
                    REQUIRE_EQ(std::count_if(tiles.begin(), tiles.end(), [](const status& s){ return s.is_black; }), 2208);
        }
    }

} /* namespace */