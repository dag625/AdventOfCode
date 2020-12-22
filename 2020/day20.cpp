//
// Created by Daniel Garcia on 12/20/20.
//

#include "day20.h"
#include "utilities.h"
#include "grid.h"
#include "stride_span.h"

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
            bool flipped = false;
        };

        struct tile {
            int id = 0;
            grid<char> data;
            std::array<neighbor_info, 4> neighbors;
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
            retval[static_cast<std::size_t>(border::top)] = stride_span<const char>{tile[0]};
            retval[static_cast<std::size_t>(border::right)] = tile.column_span(tile.num_cols() - 1);
            retval[static_cast<std::size_t>(border::bottom)] = stride_span<const char>{tile[tile.num_rows() - 1]};
            retval[static_cast<std::size_t>(border::left)] = tile.column_span(0);
            return retval;
        }

        std::vector<matching_border> find_matching_borders(const tile& a, const tile& b) {
            std::vector<matching_border> retval;
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
                        bool match = rotate_match || flip_match;
                        match_type mt = match_type::both;
                        if (!flip_match) {
                            mt = match_type::rotate;
                        }
                        else if (!rotate_match) {
                            mt = match_type::flip;
                        }
                        if (match) {
                            if (a.id < b.id) {
                                retval.push_back({{a.id, static_cast<border>(ai)},
                                                  {b.id, static_cast<border>(bi)},
                                                  mt});
                            } else {
                                retval.push_back({{b.id, static_cast<border>(bi)},
                                                  {a.id, static_cast<border>(ai)},
                                                  mt});
                            }
                        }
                    }
                }
            }
            return retval;
        }

        std::vector<matching_border> find_matching_borders(const tile& t, const std::vector<tile>& tiles) {
            std::vector<matching_border> retval;
            for (const auto& tb : tiles) {
                auto b = find_matching_borders(t, tb);
                retval.insert(retval.end(), b.begin(), b.end());
            }
            return retval;
        }

        stride_span<const char> get_tile_row(const tile& t, std::size_t row, int rotate, bool flip) {
            stride_span<const char> retval;
            if (rotate == 0) {
                retval = stride_span<const char>(t.data[row]);
            }
            else if (rotate == 1) {
                retval = t.data.column_span(0);
            }
            else if (rotate == 2) {
                retval = stride_span<const char>(t.data[t.data.num_rows() - row - 1]).reverse();
            }
            else if (rotate == 3) {
                retval = t.data.column_span(t.data.num_cols() - 1).reverse();
            }
            if (flip) {
                retval = retval.reverse();
            }
            return retval;
        }

        constexpr int PREVIOUS_ID_FIRST_COL = -1;
        constexpr int PREVIOUS_ID_FIRST_ROW = -2;
        constexpr int PREVIOUS_ID_TOP_LEFT = -3;

        std::tuple<int, int, bool> get_next(int current_id, int prev_id, int current_rotate, bool current_flip, const std::vector<tile>& tiles, const std::vector<std::pair<int, std::vector<matching_border>>>& borders_map) {
            const auto& borders = std::find_if(borders_map.begin(), borders_map.end(), [id = current_id](const auto& p){ return id == p.first; })->second;
            const auto next_border = get_border(direction, current_rotate, current_flip);
            const auto& next = *std::find_if(borders.begin(), borders.end(), [next_border, id = current_id](const matching_border& b){
                return std::get<0>(b).id == id && std::get<0>(b).side == next_border ||
                       std::get<1>(b).id == id && std::get<1>(b).side == next_border;
            });
            const auto [nb, mt] = get_match_not_for(next, current_id);
            if (mt == match_type::flip || mt == match_type::both) {
                current_flip = !current_flip;
            }
            if (mt == match_type::rotate || mt == match_type::both) {
                current_rotate += get_rotate(nb.side, border::left);
            }
            return {nb.id, current_rotate, current_flip};
        }

        void build_row(std::vector<char>& data, const std::size_t total_size, const std::size_t num_cols, const std::size_t row, const std::size_t col,
                       const tile& current, int prev_id, int current_rotate, bool current_flip,
                       const std::vector<tile>& tiles, const std::vector<std::pair<int, std::vector<matching_border>>>& borders_map)
        {
            const auto start_row = row == 0 ? 0 : 1;
            for (std::size_t data_row = start_row; data_row < current.data.num_rows(); ++data_row) {
                const auto row_data = get_tile_row(current, data_row, current_rotate, current_flip);
                const auto col_size = row_data.size() - 1;
                if (col == 0) {
                    data.insert(data.begin() + total_size * row, row_data.begin(), row_data.end());
                }
                else {
                    data.insert(data.begin() + total_size * row + col * col_size + 1, row_data.begin() + 1, row_data.end());
                }
            }
            if (col == num_cols - 1) {
                return;
            }

            //auto [next_id, next_rotate, next_flip] = get_next(current.id, border::right, current_rotate, current_flip, tiles, borders_map);
            auto tuple = get_next(current.id, prev_id, current_rotate, current_flip, tiles, borders_map);
            auto [next_id, next_rotate, next_flip] = tuple;
            build_row(data, total_size, num_cols, row, col + 1, *std::find(tiles.begin(), tiles.end(), next_id), current.id, next_rotate, next_flip, tiles, borders_map);
        }

        std::vector<char> build(const std::size_t size, const std::vector<tile>& tiles, const std::vector<std::pair<int, std::vector<matching_border>>>& borders_map) {
            const auto total_size = size * (tiles.front().data.num_cols() - 1) + 1;
            std::vector<char> data;
            data.resize(total_size * total_size);
            const auto& first_borders = *std::find_if(borders_map.begin(), borders_map.end(),[](const std::pair<int, std::vector<matching_border>>& p){
                return p.second.size() == 2;
            });

            auto [b1, t1] = get_match_for(first_borders.second[0], first_borders.first);
            auto [b2, t2] = get_match_for(first_borders.second[1], first_borders.first);
            auto r1 = get_rotate(b1, border::right);
            auto r2 = get_rotate(b2, border::right);
            int rotate = 0, next_id = first_borders.first;
            if (r1 == 0 && r2 == 3 || r2 == 0 && r1 == 3) {
                //No rotation needed, either b1 is right and b2 is bottom (1st condition) or b2 is right and b1 is bottom (2nd condition).
            }
            else if (r1 > r2) {
                //Rotate by r1, r1 is seed for first row and r2 is the seed for the second row.
                rotate = r1;
            }
            else {
                //Rotate by r2, r2 is seed for first row and r1 is the seed for the second row.
                rotate = r2;
            }

            auto row_start = std::find(tiles.begin(), tiles.end(), first_borders.first);
            bool flip = false;
            int prev_id = PREVIOUS_ID_FIRST_ROW;
            for (std::size_t i = 0; i < size; ++i) {
                build_row(data, total_size, size, i, 0, *row_start, i == 0 ? PREVIOUS_ID_TOP_LEFT : PREVIOUS_ID_FIRST_COL, rotate, flip, tiles, borders_map);
                if (i < size - 1) {
                    std::tie(next_id, rotate, flip) =
                            get_next(row_start->id, prev_id, rotate, flip, tiles, borders_map);
                    prev_id = row_start->id;
                    row_start = std::find(tiles.begin(), tiles.end(), next_id);
                }
            }
            return data;
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
        for (const auto& t : tiles) {
            auto borders = get_borders(t.data);
            auto mb = find_matching_borders(t, tiles);
            if (mb.size() == 2) {
                acc *= t.id;
            }
        }
        std::cout << '\t' << acc << '\n';
    }

    /*

    */
    void solve_day_20_2(const std::filesystem::path& input_dir) {
        auto tiles = get_input(input_dir);
        int i = 0;
        while (i * i < tiles.size()) { ++i; }
        const auto size = static_cast<std::size_t>(i);
        std::vector<std::pair<int, std::vector<matching_border>>> borders_map;
        int start = -1;
        for (const auto& t : tiles) {
            auto borders = get_borders(t.data);
            auto mb = find_matching_borders(t, tiles);
            if (mb.size() == 2 && start < 0) {
                start = t.id;
            }
            borders_map.emplace_back(t.id, std::move(mb));
        }
        grid<char> full {build(size, tiles, borders_map), size};
        std::cout << "Full grid:\n";
        full.display(std::cout);
        std::cout << '\t' << 0 << '\n';
    }

} /* namespace aoc2020 */