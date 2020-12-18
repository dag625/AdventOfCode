//
// Created by Daniel Garcia on 12/17/20.
//

#include "day17.h"
#include "utilities.h"
#include "grid.h"
#include "point_nd.h"

#include <doctest/doctest.h>

#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace doctest {

    template<std::size_t D> struct StringMaker<aoc::point<D>> {
        static String convert(const aoc::point<D>& p) {
            std::stringstream ps;
            bool first = true;
            ps << '(';
            for (auto v : p) {
                if (!first) {
                    ps << ',';
                }
                first = false;
                ps << v;
            }
            ps << ')';
            return String{ps.str().c_str()};
        }
    };

} /* namespace doctest */

namespace aoc2020 {

    using namespace aoc;

    namespace {

        struct cell {
            bool active = false;
            bool was_active = false;
        };

        template <std::size_t S, std::size_t D>
        constexpr int GRID_SIZE = S * GRID_SIZE<S, D - 1>;

        template <std::size_t S>
        constexpr int GRID_SIZE<S, 1> = S;

        template <typename T, std::size_t D, std::size_t S>
        class n_grid {
            std::array<T, GRID_SIZE<S, D>> m_data;
            static constexpr int ZERO_OFFSET = S / 2;

        public:

            static std::size_t index(const point<D>& p) {
                std::size_t idx = 0;
                for (auto i : p) {
                    idx *= S;
                    idx += i + ZERO_OFFSET;
                }
                return idx;
            }

            static point<D> from_index(std::size_t idx) {
                point<D> retval{};
                for (int i = D - 1; i >= 0; --i) {
                    retval[i] = idx % S - ZERO_OFFSET;
                    idx /= S;
                }
                return retval;
            }

            static bool is_in(const point<D>& p) {
                for (auto v : p) {
                    auto adj = v + ZERO_OFFSET;
                    if (adj < 0) {
                        return false;
                    }
                    else if (adj >= S) {
                        return false;
                    }
                }
                return true;
            }

            auto begin() -> decltype(m_data.begin()) { return m_data.begin(); }
            auto end() -> decltype(m_data.end()) { return m_data.end(); }
            auto begin() const -> decltype(m_data.begin()) { return m_data.begin(); }
            auto end() const -> decltype(m_data.end()) { return m_data.end(); }

            auto data() -> decltype(m_data.data()) { return m_data.data(); }
            auto data() const -> decltype(m_data.data()) { return m_data.data(); }

            auto size() const -> decltype(m_data.size()) { return m_data.size(); }

            T& operator[](const point<D>& p) {
                return m_data[index(p)];
            }
            const T& operator[](const point<D>& p) const {
                return m_data[index(p)];
            }

            T& operator[](std::size_t idx) {
                return m_data[idx];
            }
            const T& operator[](std::size_t idx) const {
                return m_data[idx];
            }
        };

        template <std::size_t D, std::size_t S>
        int count_active(const n_grid<cell, D, S>& data, std::size_t idx) {
            auto p = n_grid<cell, D, S>::from_index(idx);
            int retval = 0;
            for (const auto& n : get_standard_directions<D>()) {
                if (n_grid<cell, D, S>::is_in(p + n) && data[p + n].was_active) {
                    ++retval;
                }
            }
            return retval;
        }

        template <std::size_t D, std::size_t S>
        void iterate(n_grid<cell, D, S>& data) {
            std::for_each(data.begin(), data.end(), [](cell& c){ c.was_active = c.active; });
            for (std::size_t idx = 0; idx < data.size(); ++idx) {
                auto num_active = count_active(data, idx);
                if (num_active == 3) {
                    data[idx].active = true;
                }
                else if (data[idx].active && num_active != 2) {
                    data[idx].active = false;
                }
            }
        }

        template <std::size_t D, std::size_t S>
        n_grid<cell, D, S> get_input(const fs::path &input_dir) {
            auto lines = read_file_lines(input_dir / "2020" / "day_17_input.txt");
            auto grid = to_grid(lines);
            n_grid<cell, D, S> retval{};
            for (const auto p : grid.list_positions()) {
                if (grid[p] == '#') {
                    retval[from_flat<D>(p.x, p.y)] = {true, true};
                }
            }
            return retval;
        }

    }

    /*

    */
    void solve_day_17_1(const std::filesystem::path& input_dir) {
        auto cells = get_input<3, 25>(input_dir);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        std::cout << '\t' << std::count_if(cells.begin(), cells.end(), [](const cell& c){ return c.active; }) << '\n';
    }

    /*

    */
    void solve_day_17_2(const std::filesystem::path& input_dir) {
        auto cells = get_input<4, 25>(input_dir);
        for (int i = 0; i < 6; ++i) {
            iterate(cells);
        }
        std::cout << '\t' << std::count_if(cells.begin(), cells.end(), [](const cell& c){ return c.active; }) << '\n';
    }

    TEST_SUITE("day17" * doctest::description("Tests for day 17 challenges.")) {
        TEST_CASE("day17:roundtrip_index_to_point" * doctest::description("Testing conversions between an index and a point and back.")) {
            using test_grid = n_grid<cell, 3, 24>;
            REQUIRE_EQ(test_grid::index(test_grid::from_index(0)), 0);
            REQUIRE_EQ(test_grid::index(test_grid::from_index(GRID_SIZE<24, 3> - 1)), GRID_SIZE<24, 3> - 1);
            REQUIRE_EQ(test_grid::from_index(0), point<3>{-12, -12, -12});
            REQUIRE_EQ(test_grid::index({0,0,0}), 12 * 24 * 24 + 12 * 24 + 12);
            REQUIRE_EQ(test_grid::from_index((1 + 12) * 24 * 24 + (2 + 12) * 24 + (3 + 12)), point<3>{1, 2, 3});
            REQUIRE_EQ(test_grid::index({3, 2, 1}), (3 + 12) * 24 * 24 + (2 + 12) * 24 + (1 + 12));
        }
    }

} /* namespace aoc2020 */