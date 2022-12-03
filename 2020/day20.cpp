//
// Created by Daniel Garcia on 12/20/20.
//

#include "registration.h"
#include "utilities.h"
#include "grid.h"
#include "stride_span.h"

#include <doctest/doctest.h>

#include <iostream>
#include <vector>
#include <charconv>
#include <array>
#include <iterator>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;
    using namespace std::string_view_literals;

    namespace {

        constexpr bool KEEP_TILE_BORDERS = false;
        constexpr bool SHOW_FINAL_GRID = false;

        constexpr auto MONSTER_TOP = "                  # "sv;
        constexpr auto MONSTER_MID = "#    ##    ##    ###"sv;
        constexpr auto MONSTER_BOT = " #  #  #  #  #  #   "sv;

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
            if constexpr (!KEEP_TILE_BORDERS) {
                //Key point I missed for a few days:  the borders aren't part of the image!
                for (std::size_t data_row = 1; data_row < current.data.num_rows() - 1; ++data_row) {
                    const auto col_size = current.data.num_cols() - 2;
                    const auto row_data = get_next_tile_in_row(current, data_row, from, flip).sub_span(1, col_size);
                    std::copy(row_data.begin(), row_data.end(),
                              data.begin() + ((row * col_size + data_row - 1) * total_size + col * col_size));
                }
            }
            else {
                for (std::size_t data_row = 0; data_row < current.data.num_rows(); ++data_row) {
                    const auto row_data = get_next_tile_in_row(current, data_row, from, flip);
                    auto dest = (row * current.data.num_rows() + data_row) * total_size + col * current.data.num_cols();
                    std::copy(row_data.begin(), row_data.end(), data.begin() + dest);
                }
            }

            const auto& next_info = current.neighbors[static_cast<std::size_t>(opposite(from))];
            if (!next_info) {
                return;
            }
            flip = flip != next_info->flipped;
            const tile& next = *std::find(tiles.begin(), tiles.end(), next_info->id);
            build_row(data, row, col + 1, total_size, next, next_info->side, flip, tiles);
        }

        std::size_t grid_total_size(const std::size_t num_tiles_1d, const std::size_t tile_size_1d) {
            if constexpr (KEEP_TILE_BORDERS) {
                return num_tiles_1d * tile_size_1d;
            }
            else {
                //Key point I missed for a few days:  the borders aren't part of the image!
                return num_tiles_1d * (tile_size_1d - 2);
            }
        }

        std::vector<char> build(const std::vector<tile>& tiles, const std::size_t num_tiles_1d) {
            const auto num_chars_1d = grid_total_size(num_tiles_1d, tiles.front().data.num_cols());
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
                if (!next_info.has_value()) {
                    continue;
                }
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

        stride_span<const char> get_row(const grid<char>& g, std::size_t row, std::size_t offset, std::size_t len) {
            return g.row_span(row).sub_span(offset, len);
        }

        stride_span<const char> get_col(const grid<char>& g, std::size_t col, std::size_t offset, std::size_t len) {
            return g.column_span(col).sub_span(offset, len);
        }

        stride_span<char> get_mutable_row(grid<char>& g, std::size_t row, std::size_t offset, std::size_t len) {
            return g.row_span(row).sub_span(offset, len);
        }

        stride_span<char> get_mutable_col(grid<char>& g, std::size_t col, std::size_t offset, std::size_t len) {
            return g.column_span(col).sub_span(offset, len);
        }

        bool equal_char(char a, char b) {
            if (a == ' ' || b == ' ') {
                return true;
            }
            return a == b;
        }

        template <typename T>
        using get_seq = stride_span<T> (*)(std::conditional_t<std::is_const_v<T>, std::add_const_t<grid<std::remove_const_t<T>>>, grid<std::remove_const_t<T>>>&,
                std::size_t, std::size_t, std::size_t);

        template <typename T>
        std::pair<std::vector<position>, std::vector<position>> find_appearances(const grid<std::remove_const_t<T>>& g, std::string_view seq, get_seq<T> func) {
            const auto seq_len = seq.size();
            const auto grid_size = g.num_cols();//We assume squareness here.
            const auto max_offset = grid_size - seq_len;
            std::pair<std::vector<position>, std::vector<position>> retval;
            for (std::size_t idx = 0; idx < grid_size; ++idx) {
                for (std::size_t off = 0; off < max_offset; ++off) {
                    auto span = func(g, idx, off, seq_len);
                    if (std::equal(seq.begin(), seq.end(), span.begin(), equal_char)) {
                        retval.first.push_back({static_cast<int>(idx), static_cast<int>(off)});
                    }
                    if (std::equal(seq.begin(), seq.end(), span.reverse().begin(), equal_char)) {
                        retval.second.push_back({static_cast<int>(idx), static_cast<int>(off)});
                    }
                }
            }
            return retval;
        }

        std::vector<std::pair<std::array<position, 3>, bool>> find_monsters(const std::vector<position>& top, const std::vector<position>& mid, const std::vector<position>& bot, bool backwards) {
            std::vector<std::pair<std::array<position, 3>, bool>> found;
            for (const auto& tp : top) {
                auto m_up = std::find_if(mid.begin(), mid.end(), [&tp](const position& mp){
                   return mp.y == tp.y && mp.x == tp.x + 1;
                });
                auto m_down = std::find_if(mid.begin(), mid.end(), [&tp](const position& mp){
                    return mp.y == tp.y && mp.x == tp.x - 1;
                });
                auto b_up = std::find_if(bot.begin(), bot.end(), [&tp](const position& mp){
                    return mp.y == tp.y && mp.x == tp.x + 2;
                });
                auto b_down = std::find_if(bot.begin(), bot.end(), [&tp](const position& mp){
                    return mp.y == tp.y && mp.x == tp.x - 2;
                });
                if (m_up != mid.end() && b_up != bot.end()) {
                    found.push_back({{tp, *m_up, *b_up}, backwards});
                }
                if (m_down != mid.end() && b_down != bot.end()) {
                    found.push_back({{tp, *m_down, *b_down}, backwards});
                }
            }
            return found;
        }

        template <typename T>
        std::vector<std::pair<std::array<position, 3>, bool>> find_monsters(const grid<std::remove_const_t<T>>& g, get_seq<T> func) {
            auto [forwards1, backwards1] = find_appearances(g, MONSTER_TOP, func);
            auto [forwards2, backwards2] = find_appearances(g, MONSTER_MID, func);
            auto [forwards3, backwards3] = find_appearances(g, MONSTER_BOT, func);
            auto forwards = find_monsters(forwards1, forwards2, forwards3, false);
            auto backwards = find_monsters(backwards1, backwards2, backwards3, true);
            forwards.insert(forwards.end(), backwards.begin(), backwards.end());
            return forwards;
        }

        std::size_t count_monsters(const grid<char>& g) {
            return find_monsters(g, get_row).size() + find_monsters(g, get_col).size();
        }

        void mark_sequence(stride_span<char>& seq, std::string_view pattern, char to_mark, char mark_char) {
            for (std::size_t idx = 0; idx < seq.size() && idx < pattern.size(); ++idx) {
                if (pattern[idx] == to_mark) {
                    seq[idx] = mark_char;
                }
            }
        }

        void mark_monsters(grid<char>& g, const std::vector<std::pair<std::array<position, 3>, bool>>& found, get_seq<char> func) {
            for (const auto& monster : found) {
                auto top = func(g, monster.first[0].x, monster.first[0].y, MONSTER_TOP.size());
                auto mid = func(g, monster.first[1].x, monster.first[1].y, MONSTER_MID.size());
                auto bot = func(g, monster.first[2].x, monster.first[2].y, MONSTER_BOT.size());
                if (monster.second) {
                    top = top.reverse();
                    mid = mid.reverse();
                    bot = bot.reverse();
                }
                mark_sequence(top, MONSTER_TOP, '#', 'x');
                mark_sequence(mid, MONSTER_MID, '#', 'x');
                mark_sequence(bot, MONSTER_BOT, '#', 'x');
            }
        }

        void mark_monsters(grid<char>& g) {
            auto row_monsters = find_monsters(g, get_row);
            auto col_monsters = find_monsters(g, get_col);
            mark_monsters(g, row_monsters, get_mutable_row);
            mark_monsters(g, col_monsters, get_mutable_col);
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
    The high-speed train leaves the forest and quickly carries you south. You can even see a desert in the distance! Since you have some spare time, you might as well see if there was anything interesting in the image the Mythical Information Bureau satellite captured.

    After decoding the satellite messages, you discover that the data actually contains many small images created by the satellite's camera array. The camera array consists of many cameras; rather than produce a single square image, they produce many smaller square image tiles that need to be reassembled back into a single image.

    Each camera in the camera array returns a single monochrome image tile with a random unique ID number. The tiles (your puzzle input) arrived in a random order.

    Worse yet, the camera array appears to be malfunctioning: each image tile has been rotated and flipped to a random orientation. Your first task is to reassemble the original image by orienting the tiles so they fit together.

    To show how the tiles should be reassembled, each tile's image data includes a border that should line up exactly with its adjacent tiles. All tiles have this border, and the border lines up exactly when the tiles are both oriented correctly. Tiles at the edge of the image also have this border, but the outermost edges won't line up with any other tiles.

    For example, suppose you have the following nine tiles:

    Tile 2311:
    ..##.#..#.
    ##..#.....
    #...##..#.
    ####.#...#
    ##.##.###.
    ##...#.###
    .#.#.#..##
    ..#....#..
    ###...#.#.
    ..###..###

    Tile 1951:
    #.##...##.
    #.####...#
    .....#..##
    #...######
    .##.#....#
    .###.#####
    ###.##.##.
    .###....#.
    ..#.#..#.#
    #...##.#..

    Tile 1171:
    ####...##.
    #..##.#..#
    ##.#..#.#.
    .###.####.
    ..###.####
    .##....##.
    .#...####.
    #.##.####.
    ####..#...
    .....##...

    Tile 1427:
    ###.##.#..
    .#..#.##..
    .#.##.#..#
    #.#.#.##.#
    ....#...##
    ...##..##.
    ...#.#####
    .#.####.#.
    ..#..###.#
    ..##.#..#.

    Tile 1489:
    ##.#.#....
    ..##...#..
    .##..##...
    ..#...#...
    #####...#.
    #..#.#.#.#
    ...#.#.#..
    ##.#...##.
    ..##.##.##
    ###.##.#..

    Tile 2473:
    #....####.
    #..#.##...
    #.##..#...
    ######.#.#
    .#...#.#.#
    .#########
    .###.#..#.
    ########.#
    ##...##.#.
    ..###.#.#.

    Tile 2971:
    ..#.#....#
    #...###...
    #.#.###...
    ##.##..#..
    .#####..##
    .#..####.#
    #..#.#..#.
    ..####.###
    ..#.#.###.
    ...#.#.#.#

    Tile 2729:
    ...#.#.#.#
    ####.#....
    ..#.#.....
    ....#..#.#
    .##..##.#.
    .#.####...
    ####.#.#..
    ##.####...
    ##..#.##..
    #.##...##.

    Tile 3079:
    #.#.#####.
    .#..######
    ..#.......
    ######....
    ####.#..#.
    .#...#.##.
    #.#####.##
    ..#.###...
    ..#.......
    ..#.###...
    By rotating, flipping, and rearranging them, you can find a square arrangement that causes all adjacent borders to line up:

    #...##.#.. ..###..### #.#.#####.
    ..#.#..#.# ###...#.#. .#..######
    .###....#. ..#....#.. ..#.......
    ###.##.##. .#.#.#..## ######....
    .###.##### ##...#.### ####.#..#.
    .##.#....# ##.##.###. .#...#.##.
    #...###### ####.#...# #.#####.##
    .....#..## #...##..#. ..#.###...
    #.####...# ##..#..... ..#.......
    #.##...##. ..##.#..#. ..#.###...

    #.##...##. ..##.#..#. ..#.###...
    ##..#.##.. ..#..###.# ##.##....#
    ##.####... .#.####.#. ..#.###..#
    ####.#.#.. ...#.##### ###.#..###
    .#.####... ...##..##. .######.##
    .##..##.#. ....#...## #.#.#.#...
    ....#..#.# #.#.#.##.# #.###.###.
    ..#.#..... .#.##.#..# #.###.##..
    ####.#.... .#..#.##.. .######...
    ...#.#.#.# ###.##.#.. .##...####

    ...#.#.#.# ###.##.#.. .##...####
    ..#.#.###. ..##.##.## #..#.##..#
    ..####.### ##.#...##. .#.#..#.##
    #..#.#..#. ...#.#.#.. .####.###.
    .#..####.# #..#.#.#.# ####.###..
    .#####..## #####...#. .##....##.
    ##.##..#.. ..#...#... .####...#.
    #.#.###... .##..##... .####.##.#
    #...###... ..##...#.. ...#..####
    ..#.#....# ##.#.#.... ...##.....
    For reference, the IDs of the above tiles are:

    1951    2311    3079
    2729    1427    2473
    2971    1489    1171
    To check that you've assembled the image correctly, multiply the IDs of the four corner tiles together. If you do this with the assembled tiles from the example above, you get 1951 * 3079 * 2971 * 1171 = 20899048083289.

    Assemble the tiles into an image. What do you get if you multiply together the IDs of the four corner tiles?
    */
    std::string solve_day_20_1(const std::filesystem::path& input_dir) {
        auto tiles = get_input(input_dir);
        int64_t acc = 1;
        find_matching_borders(tiles);
        for (const auto& t : tiles) {
            if (std::count_if(t.neighbors.begin(), t.neighbors.end(), [](const std::optional<neighbor_info>& i){ return i.has_value(); }) == 2) {
                acc *= t.id;
            }
        }
        return std::to_string(acc);
    }

    /*
    Now, you're ready to check the image for sea monsters.

    The borders of each tile are not part of the actual image; start by removing them.

    In the example above, the tiles become:

    .#.#..#. ##...#.# #..#####
    ###....# .#....#. .#......
    ##.##.## #.#.#..# #####...
    ###.#### #...#.## ###.#..#
    ##.#.... #.##.### #...#.##
    ...##### ###.#... .#####.#
    ....#..# ...##..# .#.###..
    .####... #..#.... .#......

    #..#.##. .#..###. #.##....
    #.####.. #.####.# .#.###..
    ###.#.#. ..#.#### ##.#..##
    #.####.. ..##..## ######.#
    ##..##.# ...#...# .#.#.#..
    ...#..#. .#.#.##. .###.###
    .#.#.... #.##.#.. .###.##.
    ###.#... #..#.##. ######..

    .#.#.### .##.##.# ..#.##..
    .####.## #.#...## #.#..#.#
    ..#.#..# ..#.#.#. ####.###
    #..####. ..#.#.#. ###.###.
    #####..# ####...# ##....##
    #.##..#. .#...#.. ####...#
    .#.###.. ##..##.. ####.##.
    ...###.. .##...#. ..#..###
    Remove the gaps to form the actual image:

    .#.#..#.##...#.##..#####
    ###....#.#....#..#......
    ##.##.###.#.#..######...
    ###.#####...#.#####.#..#
    ##.#....#.##.####...#.##
    ...########.#....#####.#
    ....#..#...##..#.#.###..
    .####...#..#.....#......
    #..#.##..#..###.#.##....
    #.####..#.####.#.#.###..
    ###.#.#...#.######.#..##
    #.####....##..########.#
    ##..##.#...#...#.#.#.#..
    ...#..#..#.#.##..###.###
    .#.#....#.##.#...###.##.
    ###.#...#..#.##.######..
    .#.#.###.##.##.#..#.##..
    .####.###.#...###.#..#.#
    ..#.#..#..#.#.#.####.###
    #..####...#.#.#.###.###.
    #####..#####...###....##
    #.##..#..#...#..####...#
    .#.###..##..##..####.##.
    ...###...##...#...#..###
    Now, you're ready to search for sea monsters! Because your image is monochrome, a sea monster will look like this:

                      #
    #    ##    ##    ###
     #  #  #  #  #  #
    When looking for this pattern in the image, the spaces can be anything; only the # need to match. Also, you might need to rotate or flip your image before it's oriented correctly to find sea monsters. In the above image, after flipping and rotating it to the appropriate orientation, there are two sea monsters (marked with O):

    .####...#####..#...###..
    #####..#..#.#.####..#.#.
    .#.#...#.###...#.##.O#..
    #.O.##.OO#.#.OO.##.OOO##
    ..#O.#O#.O##O..O.#O##.##
    ...#.#..##.##...#..#..##
    #.##.#..#.#..#..##.#.#..
    .###.##.....#...###.#...
    #.####.#.#....##.#..#.#.
    ##...#..#....#..#...####
    ..#.##...###..#.#####..#
    ....#.##.#.#####....#...
    ..##.##.###.....#.##..#.
    #...#...###..####....##.
    .#.##...#.##.#.#.###...#
    #.###.#..####...##..#...
    #.###...#.##...#.##O###.
    .O##.#OO.###OO##..OOO##.
    ..O#.O..O..O.#O##O##.###
    #.#..##.########..#..##.
    #.#####..#.#...##..#....
    #....##..#.#########..##
    #...#.....#..##...###.##
    #..###....##.#...##.##.#
    Determine how rough the waters are in the sea monsters' habitat by counting the number of # that are not part of a sea monster. In the above example, the habitat's water roughness is 273.

    How many # are not part of a sea monster?
    */
    std::string solve_day_20_2(const std::filesystem::path& input_dir) {
        auto tiles = get_input(input_dir);
        find_matching_borders(tiles);
        int i = 0;
        while (i * i < tiles.size()) { ++i; }
        const auto size = static_cast<std::size_t>(i);
        grid<char> full {build(tiles, size), grid_total_size(size, tiles.front().data.num_cols())};
        mark_monsters(full);
        if constexpr (SHOW_FINAL_GRID) {
            std::cout << "Full grid:\n";
            full.display(std::cout);
        }
        return std::to_string(std::count_if(full.begin(), full.end(), [](char c){ return c == '#'; }));
    }

    static aoc::registration r {2020, 20, solve_day_20_1, solve_day_20_2};

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
        TEST_CASE("day20:stride_span_sub" * doctest::description("Testing getting sub stride_spans.")) {
            std::vector<int> data;
            int current = 0;
            std::generate_n(std::back_inserter(data), 100, [&current](){ return current++; });
            grid<int> g {data, 10};

            auto row0 = g.row_span(0).sub_span(2, 3);
                    REQUIRE_EQ(row0.size(), 3);
                    REQUIRE_EQ(row0[0], 2);
                    REQUIRE_EQ(row0[1], 3);
                    REQUIRE_EQ(row0[2], 4);

            auto col3 = g.column_span(3).sub_span(3, 5);
                    REQUIRE_EQ(col3.size(), 5);
                    REQUIRE_EQ(col3[0], 33);
                    REQUIRE_EQ(col3[1], 43);
                    REQUIRE_EQ(col3[2], 53);
                    REQUIRE_EQ(col3[3], 63);
                    REQUIRE_EQ(col3[4], 73);

            auto col4 = g.column_span(4).sub_span(7).reverse();
                    REQUIRE_EQ(col4.size(), 3);
                    REQUIRE_EQ(col4[0], 94);
                    REQUIRE_EQ(col4[1], 84);
                    REQUIRE_EQ(col4[2], 74);
        }
        TEST_CASE("day20:part2_example" * doctest::description("Testing part 2 example.")) {
            const char* ex_text = ".#.#..#.##...#.##..#####"
                                  "###....#.#....#..#......"
                                  "##.##.###.#.#..######..."
                                  "###.#####...#.#####.#..#"
                                  "##.#....#.##.####...#.##"
                                  "...########.#....#####.#"
                                  "....#..#...##..#.#.###.."
                                  ".####...#..#.....#......"
                                  "#..#.##..#..###.#.##...."
                                  "#.####..#.####.#.#.###.."
                                  "###.#.#...#.######.#..##"
                                  "#.####....##..########.#"
                                  "##..##.#...#...#.#.#.#.."
                                  "...#..#..#.#.##..###.###"
                                  ".#.#....#.##.#...###.##."
                                  "###.#...#..#.##.######.."
                                  ".#.#.###.##.##.#..#.##.."
                                  ".####.###.#...###.#..#.#"
                                  "..#.#..#..#.#.#.####.###"
                                  "#..####...#.#.#.###.###."
                                  "#####..#####...###....##"
                                  "#.##..#..#...#..####...#"
                                  ".#.###..##..##..####.##."
                                  "...###...##...#...#..###";
            std::string ex_str {ex_text};
            grid<char> data {std::vector<char>{ex_str.begin(), ex_str.end()}, 24};

                    CHECK_EQ(count_monsters(data), 2);

                    mark_monsters(data);

                    CHECK_EQ(std::count_if(data.begin(), data.end(), [](char c){ return c == '#'; }), 273);
        }
    }

} /* namespace aoc2020 */