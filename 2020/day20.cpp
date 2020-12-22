//
// Created by Daniel Garcia on 12/20/20.
//

#include "day20.h"
#include "utilities.h"
#include "grid.h"
#include "stride_span.h"

#include <doctest/doctest.h>

#include <iostream>
#include <vector>
#include <charconv>
#include <array>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;
    using namespace std::string_view_literals;

    namespace {

        enum class border : std::size_t {
            top = 0,
            right = 1,
            bottom = 2,
            left = 3,
            num_borders = 4
        };

        constexpr auto NEED_REVERSE_COMPARE = std::array{
                true, // top , top
                true, // top , right
                false, // top , bottom
                false, // top , left

                true, // right , top
                true, // right , right
                false, // right , bottom
                false, // right , left

                false, // bottom , top
                false, // bottom , right
                true, // bottom , bottom
                true, // bottom , left

                false, // left , top
                false, // left , right
                true, // left , bottom
                true // left , left
        };

        struct neighbor_info {
            int id = 0;
            border side = border::num_borders;
            bool flipped = false;
        };

        struct tile {
            int id = 0;
            grid<char> data;
            std::array<std::optional<neighbor_info>, 4> neighbors;
        };

        bool operator==(const tile& a, int id) {
            return a.id == id;
        }

        struct tile_border {
            int id = 0;
            border side = border::top;
        };

        bool operator==(const tile_border& a, const tile_border& b) {
            return std::tie(a.id, a.side) == std::tie(b.id, b.side);
        }
        bool operator< (const tile_border& a, const tile_border& b) {
            return std::tie(a.id, a.side) <  std::tie(b.id, b.side);
        }

        enum class match_type {
            both,
            rotate,
            flip
        };

        using matching_border = std::tuple<tile_border, tile_border, match_type>;

//        bool operator==(const matching_border& a, const matching_border& b) {
//            return std::tie(a.first, a.second) == std::tie(b.first, b.second);
//        }
//        bool operator< (const matching_border& a, const matching_border& b) {
//            return std::tie(a.first, a.second) <  std::tie(b.first, b.second);
//        }

        std::pair<border, match_type> get_match_for(const matching_border& mb, int id) {
            if (std::get<0>(mb).id == id) {
                return {std::get<0>(mb).side, std::get<2>(mb)};
            }
            else {
                return {std::get<1>(mb).side, std::get<2>(mb)};
            }
        }

        std::pair<tile_border, match_type> get_match_not_for(const matching_border& mb, int id) {
            if (std::get<0>(mb).id == id) {
                return {std::get<1>(mb), std::get<2>(mb)};
            }
            else {
                return {std::get<0>(mb), std::get<2>(mb)};
            }
        }

        int get_rotate(border from, border to) {
            constexpr auto num = static_cast<int>(border::num_borders);
            auto diff = static_cast<int>(to) - static_cast<int>(from);
            while (diff < 0) {
                diff += num;
            }
            while (diff >= num) {
                diff -= num;
            }
            return diff;
        }

        border rotate(border from, int diff) {
            constexpr auto num = static_cast<int>(border::num_borders);
            auto to = static_cast<int>(from) + diff;
            while (to >= num) {
                to -= num;
            }
            while (to < 0) {
                to += num;
            }
            return static_cast<border>(to);
        }

        border get_border(border desired, int rotation, bool flipped) {
            auto rot = rotate(desired, -rotation);
            if (flipped) {
                switch (rot) {
                    case border::top: return border::bottom;
                    case border::bottom: return border::top;
                    case border::right: return border::left;
                    case border::left: return border::right;
                    default: throw std::runtime_error{"Invalid rotation."};
                }
            }
            else {
                return rot;
            }
        }

        std::array<aoc::stride_span<const char>, 4> get_borders(const grid<char>& tile) {
            std::array<aoc::stride_span<const char>, 4> retval;
            retval[static_cast<std::size_t>(border::top)] = tile.row_span(0);
            retval[static_cast<std::size_t>(border::right)] = tile.column_span(tile.num_cols() - 1);
            retval[static_cast<std::size_t>(border::bottom)] = tile.row_span(tile.num_rows() - 1);
            retval[static_cast<std::size_t>(border::left)] = tile.column_span(0);
            return retval;
        }

        void find_matching_borders(tile& a, tile& b) {
            constexpr auto bsize = static_cast<std::size_t>(border::num_borders);
            if (a.id != b.id) {
                const auto a_borders = get_borders(a.data);
                const auto b_borders = get_borders(b.data);
                for (std::size_t ai = 0; ai < bsize; ++ai) {
                    for (std::size_t bi = 0; bi < bsize; ++bi) {
                        bool rotate_match = false, flip_match = false;
                        if (NEED_REVERSE_COMPARE[ai * bsize + bi]) {
                            rotate_match = std::equal(a_borders[ai].begin(), a_borders[ai].end(), b_borders[bi].reverse().begin());
                            flip_match = std::equal(a_borders[ai].begin(), a_borders[ai].end(), b_borders[bi].begin());
                        }
                        else {
                            rotate_match = std::equal(a_borders[ai].begin(), a_borders[ai].end(), b_borders[bi].begin());
                            flip_match = std::equal(a_borders[ai].begin(), a_borders[ai].end(), b_borders[bi].reverse().begin());
                        }
                        if (rotate_match && flip_match) {
                            throw std::runtime_error{"A match cannot be both"};
                        }
                        else if (rotate_match || flip_match) {
                            a.neighbors[ai] = neighbor_info{b.id, static_cast<border>(bi), flip_match};
                            b.neighbors[bi] = neighbor_info{a.id, static_cast<border>(ai), flip_match};
                        }
                    }
                }
            }
        }

        void find_matching_borders(std::vector<tile>& tiles) {
            for (std::size_t i = 0; i < tiles.size(); ++i) {
                for (std::size_t j = i + 1; j < tiles.size(); ++j) {
                    find_matching_borders(tiles[i], tiles[j]);
                }
            }
        }

        border opposite(border b) {
            switch (b) {
                case border::left: return border::right;
                case border::right: return border::left;
                case border::top: return border::bottom;
                case border::bottom: return border::top;
                default: return border::num_borders;
            }
        }

        border rotate_direction_clockwise(border prev_row_from) {
            switch (prev_row_from) {
                case border::top: return border::right;
                case border::right: return border::bottom;
                case border::bottom: return border::left;
                case border::left: return border::top;
                default: return border::num_borders;
            }
        }

        stride_span<const char> get_next_tile_in_row(const tile& t, std::size_t row, border from, bool flip) {
            stride_span<const char> retval;
            if (from == border::left) {
                if (!flip) {
                    retval = t.data.row_span(row);
                }
                else {
                    retval = t.data.row_span(t.data.num_rows() - row - 1);
                }
            }
            else if (from == border::bottom) {
                if (!flip) {
                    retval = t.data.column_span(row).reverse();
                }
                else {
                    retval = t.data.column_span(t.data.num_cols() - row - 1).reverse();
                }
            }
            else if (from == border::right) {
                if (!flip) {
                    retval = t.data.row_span(t.data.num_rows() - row - 1).reverse();
                }
                else {
                    retval = t.data.row_span(row).reverse();
                }
            }
            else if (from == border::top) {
                if (!flip) {
                    retval = t.data.column_span(t.data.num_cols() - row - 1);
                }
                else {
                    retval = t.data.column_span(row);
                }
            }
            return retval;
        }

        void build_row(std::vector<char>& data, const std::size_t row, const std::size_t col, const std::size_t total_size,
                       const tile& current, border from, bool flip, const std::vector<tile>& tiles)
        {
//            const auto start_row = row == 0 ? 0 : 1;
//            for (std::size_t data_row = start_row; data_row < current.data.num_rows(); ++data_row) {
//               const auto row_data = get_tile_row(current, data_row, from, flipped);
//               const auto col_size = row_data.size() - 1;
//               if (col == 0) {
//                   auto dest = total_size * (row * col_size + data_row + start_row);
//                   std::copy(row_data.begin(), row_data.end(), data.begin() + dest);
//               }
//               else {
//                   auto dest = total_size * (row * col_size + data_row + start_row) + col * col_size + 1;
//                   std::copy(row_data.begin() + 1, row_data.end(), data.begin() + dest);
//               }
//            }
            for (std::size_t data_row = 0; data_row < current.data.num_rows(); ++data_row) {
                const auto row_data = get_next_tile_in_row(current, data_row, from, flip);
                auto dest = (row * current.data.num_rows() + data_row) * total_size + col * current.data.num_cols();
                std::copy(row_data.begin(), row_data.end(), data.begin() + dest);
            }

            const auto& next_info = current.neighbors[static_cast<std::size_t>(opposite(from))];
            if (!next_info) {
                return;
            }
            flip = flip != next_info->flipped;
            const tile& next = *std::find(tiles.begin(), tiles.end(), next_info->id);
            build_row(data, row, col + 1, total_size, next, next_info->side, flip, tiles);
        }

        std::vector<char> build(const std::vector<tile>& tiles, const std::size_t num_tiles_1d) {
            //const auto num_chars_1d = num_tiles_1d * (tiles.front().data.num_cols() - 1) + 1;
            const auto num_chars_1d = num_tiles_1d * tiles.front().data.num_cols();
            std::vector<char> retval (num_chars_1d * num_chars_1d, '\0');
            auto row_start = std::find_if(tiles.begin(), tiles.end(), [](const tile& t){
                return std::count_if(t.neighbors.begin(), t.neighbors.end(),
                                     [](const std::optional<neighbor_info>& i){ return i.has_value(); }) == 2;
            });
            //We assume here that everything is well formed, meaning the two neighbors are adjacent.
            border left = border::left, down = border::bottom;
            if (row_start->neighbors[static_cast<std::size_t>(border::bottom)].has_value() &&
                row_start->neighbors[static_cast<std::size_t>(border::left)].has_value())
            {
                left = border::top;
                down = border::left;
            }
            else if (row_start->neighbors[static_cast<std::size_t>(border::left)].has_value() &&
                     row_start->neighbors[static_cast<std::size_t>(border::top)].has_value())
            {
                left = border::right;
                down = border::top;
            }
            else if (row_start->neighbors[static_cast<std::size_t>(border::top)].has_value() &&
                     row_start->neighbors[static_cast<std::size_t>(border::right)].has_value())
            {
                left = border::bottom;
                down = border::right;
            }

            bool flip = false;
            for (std::size_t row = 0; row < num_tiles_1d; ++row) {
                build_row(retval, row, 0, num_chars_1d, *row_start, left, flip, tiles);
                const auto& next_info = row_start->neighbors[static_cast<std::size_t>(down)];
                int prev_id = row_start->id;
                row_start = std::find(tiles.begin(), tiles.end(), next_info->id);
                std::size_t from = 0;
                for (; from < static_cast<std::size_t>(border::num_borders); ++from) {
                    if (row_start->neighbors[from].has_value() && row_start->neighbors[from]->id == prev_id) {
                        break;
                    }
                }
                flip = next_info->flipped != flip;
                left = rotate_direction_clockwise(flip ? static_cast<border>(from) : opposite(static_cast<border>(from)));
                down = opposite(next_info->side);
            }
            return retval;
        }

        std::vector<tile> get_input(const fs::path &input_dir) {
            auto lines = read_file_lines(input_dir / "2020" / "day_20_input.txt");
            auto current = lines.begin();
            const auto end = lines.end();
            std::vector<tile> tiles;
            while (current != end) {
                constexpr auto tile_prefix = "Tile "sv;
                if (!aoc::starts_with(*current, tile_prefix)) {
                    throw std::runtime_error{"Missing tile ID."};
                }
                int id = 0;
                auto res = std::from_chars(current->data() + tile_prefix.size(), current->data() + current->size(), id);
                if (res.ec != std::errc{}) {
                    throw std::system_error{std::make_error_code(res.ec)};
                }

                const auto cend = std::find_if(current, end, [](const std::string& s){ return s.empty(); });
                tiles.push_back({ id, to_grid(current + 1, cend)});
                if (cend != end) {
                    current = cend + 1;
                }
                else {
                    current = end;
                }
            }
            return tiles;
        }

    }

    /*

    */
    void solve_day_20_1(const std::filesystem::path& input_dir) {
        auto tiles = get_input(input_dir);
        int64_t acc = 1;
        find_matching_borders(tiles);
        for (const auto& t : tiles) {
            if (std::count_if(t.neighbors.begin(), t.neighbors.end(), [](const std::optional<neighbor_info>& i){ return i.has_value(); }) == 2) {
                acc *= t.id;
            }
        }
        std::cout << '\t' << acc << '\n';
    }

    /*

    */
    void solve_day_20_2(const std::filesystem::path& input_dir) {
        auto tiles = get_input(input_dir);
        find_matching_borders(tiles);
        int i = 0;
        while (i * i < tiles.size()) { ++i; }
        const auto size = static_cast<std::size_t>(i);
        //grid<char> full {build(tiles, size), size * (tiles.front().data.num_cols() - 1) + 1};
        grid<char> full {build(tiles, size), size * tiles.front().data.num_cols()};
        std::cout << "Full grid:\n";
        full.display(std::cout);
        std::cout << '\t' << 0 << '\n';
    }

    TEST_SUITE("day20" * doctest::description("Tests for day 20 challenges.")) {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        TEST_CASE("day20:stride_span_reverse0" * doctest::description("Testing reversing stride_spans (column 0).")) {
            std::vector<int> data;
            int current = 0;
            std::generate_n(std::back_inserter(data), 100, [&current](){ return current++; });
            grid<int> g {data, 10};

            auto col0 = g.column_span(0);
                    REQUIRE_EQ(col0.size(), 10);
                    REQUIRE_EQ(*(col0.begin()), 0);
                    REQUIRE_EQ(*(col0.begin() + 1), 10);
                    REQUIRE_EQ(*(col0.begin() + 2), 20);
                    REQUIRE_EQ(*(col0.begin() + 3), 30);
                    REQUIRE_EQ(*(col0.begin() + 4), 40);
                    REQUIRE_EQ(*(col0.begin() + 5), 50);
                    REQUIRE_EQ(*(col0.begin() + 6), 60);
                    REQUIRE_EQ(*(col0.begin() + 7), 70);
                    REQUIRE_EQ(*(col0.begin() + 8), 80);
                    REQUIRE_EQ(*(col0.begin() + 9), 90);
            auto col0r = col0.reverse();
                    REQUIRE_EQ(col0r.size(), 10);
                    REQUIRE_EQ(*(col0r.begin()), 90);
                    REQUIRE_EQ(*(col0r.begin() + 1), 80);
                    REQUIRE_EQ(*(col0r.begin() + 2), 70);
                    REQUIRE_EQ(*(col0r.begin() + 3), 60);
                    REQUIRE_EQ(*(col0r.begin() + 4), 50);
                    REQUIRE_EQ(*(col0r.begin() + 5), 40);
                    REQUIRE_EQ(*(col0r.begin() + 6), 30);
                    REQUIRE_EQ(*(col0r.begin() + 7), 20);
                    REQUIRE_EQ(*(col0r.begin() + 8), 10);
                    REQUIRE_EQ(*(col0r.begin() + 9), 0);
            auto col0rr = col0r.reverse();
                    REQUIRE_EQ(col0rr.size(), 10);
                    REQUIRE_EQ(*(col0rr.begin()), 0);
                    REQUIRE_EQ(*(col0rr.begin() + 1), 10);
                    REQUIRE_EQ(*(col0rr.begin() + 2), 20);
                    REQUIRE_EQ(*(col0rr.begin() + 3), 30);
                    REQUIRE_EQ(*(col0rr.begin() + 4), 40);
                    REQUIRE_EQ(*(col0rr.begin() + 5), 50);
                    REQUIRE_EQ(*(col0rr.begin() + 6), 60);
                    REQUIRE_EQ(*(col0rr.begin() + 7), 70);
                    REQUIRE_EQ(*(col0rr.begin() + 8), 80);
                    REQUIRE_EQ(*(col0rr.begin() + 9), 90);
        }
        TEST_CASE("day20:stride_span_reverse1" * doctest::description("Testing reversing stride_spans (column 1).")) {
            std::vector<int> data;
            int current = 0;
            std::generate_n(std::back_inserter(data), 100, [&current](){ return current++; });
            grid<int> g {data, 10};

            auto col1 = g.column_span(1);
                    REQUIRE_EQ(col1.size(), 10);
                    REQUIRE_EQ(*(col1.begin()), 1);
                    REQUIRE_EQ(*(col1.begin() + 1), 11);
                    REQUIRE_EQ(*(col1.begin() + 2), 21);
                    REQUIRE_EQ(*(col1.begin() + 3), 31);
                    REQUIRE_EQ(*(col1.begin() + 4), 41);
                    REQUIRE_EQ(*(col1.begin() + 5), 51);
                    REQUIRE_EQ(*(col1.begin() + 6), 61);
                    REQUIRE_EQ(*(col1.begin() + 7), 71);
                    REQUIRE_EQ(*(col1.begin() + 8), 81);
                    REQUIRE_EQ(*(col1.begin() + 9), 91);
            auto col1r = col1.reverse();
                    REQUIRE_EQ(col1r.size(), 10);
                    REQUIRE_EQ(*(col1r.begin()), 91);
                    REQUIRE_EQ(*(col1r.begin() + 1), 81);
                    REQUIRE_EQ(*(col1r.begin() + 2), 71);
                    REQUIRE_EQ(*(col1r.begin() + 3), 61);
                    REQUIRE_EQ(*(col1r.begin() + 4), 51);
                    REQUIRE_EQ(*(col1r.begin() + 5), 41);
                    REQUIRE_EQ(*(col1r.begin() + 6), 31);
                    REQUIRE_EQ(*(col1r.begin() + 7), 21);
                    REQUIRE_EQ(*(col1r.begin() + 8), 11);
                    REQUIRE_EQ(*(col1r.begin() + 9), 1);
            auto col1rr = col1r.reverse();
                    REQUIRE_EQ(col1rr.size(), 10);
                    REQUIRE_EQ(*(col1rr.begin()), 1);
                    REQUIRE_EQ(*(col1rr.begin() + 1), 11);
                    REQUIRE_EQ(*(col1rr.begin() + 2), 21);
                    REQUIRE_EQ(*(col1rr.begin() + 3), 31);
                    REQUIRE_EQ(*(col1rr.begin() + 4), 41);
                    REQUIRE_EQ(*(col1rr.begin() + 5), 51);
                    REQUIRE_EQ(*(col1rr.begin() + 6), 61);
                    REQUIRE_EQ(*(col1rr.begin() + 7), 71);
                    REQUIRE_EQ(*(col1rr.begin() + 8), 81);
                    REQUIRE_EQ(*(col1rr.begin() + 9), 91);
        }
        TEST_CASE("day20:stride_span_reverse9" * doctest::description("Testing reversing stride_spans (column 9).")) {
            std::vector<int> data;
            int current = 0;
            std::generate_n(std::back_inserter(data), 100, [&current](){ return current++; });
            grid<int> g {data, 10};

            auto col9 = g.column_span(9);
                    REQUIRE_EQ(col9.size(), 10);
                    REQUIRE_EQ(*(col9.begin()), 9);
                    REQUIRE_EQ(*(col9.begin() + 1), 19);
                    REQUIRE_EQ(*(col9.begin() + 2), 29);
                    REQUIRE_EQ(*(col9.begin() + 3), 39);
                    REQUIRE_EQ(*(col9.begin() + 4), 49);
                    REQUIRE_EQ(*(col9.begin() + 5), 59);
                    REQUIRE_EQ(*(col9.begin() + 6), 69);
                    REQUIRE_EQ(*(col9.begin() + 7), 79);
                    REQUIRE_EQ(*(col9.begin() + 8), 89);
                    REQUIRE_EQ(*(col9.begin() + 9), 99);
            auto col9r = col9.reverse();
                    REQUIRE_EQ(col9r.size(), 10);
                    REQUIRE_EQ(*(col9r.begin()), 99);
                    REQUIRE_EQ(*(col9r.begin() + 1), 89);
                    REQUIRE_EQ(*(col9r.begin() + 2), 79);
                    REQUIRE_EQ(*(col9r.begin() + 3), 69);
                    REQUIRE_EQ(*(col9r.begin() + 4), 59);
                    REQUIRE_EQ(*(col9r.begin() + 5), 49);
                    REQUIRE_EQ(*(col9r.begin() + 6), 39);
                    REQUIRE_EQ(*(col9r.begin() + 7), 29);
                    REQUIRE_EQ(*(col9r.begin() + 8), 19);
                    REQUIRE_EQ(*(col9r.begin() + 9), 9);
            auto col9rr = col9r.reverse();
                    REQUIRE_EQ(col9rr.size(), 10);
                    REQUIRE_EQ(*(col9rr.begin()), 9);
                    REQUIRE_EQ(*(col9rr.begin() + 1), 19);
                    REQUIRE_EQ(*(col9rr.begin() + 2), 29);
                    REQUIRE_EQ(*(col9rr.begin() + 3), 39);
                    REQUIRE_EQ(*(col9rr.begin() + 4), 49);
                    REQUIRE_EQ(*(col9rr.begin() + 5), 59);
                    REQUIRE_EQ(*(col9rr.begin() + 6), 69);
                    REQUIRE_EQ(*(col9rr.begin() + 7), 79);
                    REQUIRE_EQ(*(col9rr.begin() + 8), 89);
                    REQUIRE_EQ(*(col9rr.begin() + 9), 99);
        }
    }

} /* namespace aoc2020 */