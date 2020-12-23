//
// Created by Daniel Garcia on 12/12/20.
//

#include "registration.h"
#include "utilities.h"
#include "grid.h"

#include <numeric>
#include <iostream>
#include <unordered_map>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        grid<char> get_input(const fs::path &input_dir) {
            auto lines = read_file_lines(input_dir / "2020" / "day_11_input.txt");
            return to_grid(lines);
        }

        struct change {
            position p;
            char new_val = 0;

            change() = default;
            change(position pos, char val) : p{pos}, new_val{val} {}
        };

        struct ruleset_1 {
            static bool sit_down(const grid<char>& g, position p) {
                auto n = g.neighbors(p);
                return std::none_of(n.begin(), n.end(), [&g](position np){ return g[np] == '#'; });
            }
            static bool stand_up(const grid<char>& g, position p) {
                auto n = g.neighbors(p);
                return std::count_if(n.begin(), n.end(), [&g](position np){ return g[np] == '#'; }) > 3;
            }
        };

        struct ruleset_2 {
            static char first_visible(const grid<char>& g, position p, velocity v) {
                auto next_pos = p + v;
                if (!g.in(next_pos)) {
                    return '\0';
                }
                auto next = g[next_pos];
                if (next == '.') {
                    return first_visible(g, next_pos, v);
                }
                else {
                    return next;
                }
            }

            static bool sit_down(const grid<char>& g, position p) {
                auto n = g.neighbors(p);
                return std::none_of(std::begin(STANDARD_DIRECTIONS), std::end(STANDARD_DIRECTIONS),
                                    [&g, p](velocity v){ return first_visible(g, p, v) == '#'; });
            }
            static bool stand_up(const grid<char>& g, position p) {
                auto n = g.neighbors(p);
                return std::count_if(std::begin(STANDARD_DIRECTIONS), std::end(STANDARD_DIRECTIONS),
                                     [&g, p](velocity v){ return first_visible(g, p, v) == '#'; }) > 4;
            }
        };

        template <typename Ruleset>
        std::vector<change> iterate(const grid<char>& g) {
            std::vector<change> retval;
            retval.reserve(g.size());
            for (const auto p : g.list_positions()) {
                if (g[p] == 'L' && Ruleset::sit_down(g, p)) {
                    retval.emplace_back(p, '#');
                }
                else if  (g[p] == '#' && Ruleset::stand_up(g, p)) {
                    retval.emplace_back(p, 'L');
                }
            }
            return retval;
        }

        void apply_changes(grid<char>& g, const std::vector<change>& changes) {
            for (const auto& c : changes) {
                g[c.p] = c.new_val;
            }
        }

        auto num_occupied(const grid<char>& g) {
            return std::count_if(g.begin(), g.end(), [](char c){ return c == '#'; });
        }

    }

    /*
    Your plane lands with plenty of time to spare. The final leg of your journey is a ferry that goes directly to the tropical island where you can finally start your vacation. As you reach the waiting area to board the ferry, you realize you're so early, nobody else has even arrived yet!

    By modeling the process people use to choose (or abandon) their seat in the waiting area, you're pretty sure you can predict the best place to sit. You make a quick map of the seat layout (your puzzle input).

    The seat layout fits neatly on a grid. Each position is either floor (.), an empty seat (L), or an occupied seat (#). For example, the initial seat layout might look like this:

    L.LL.LL.LL
    LLLLLLL.LL
    L.L.L..L..
    LLLL.LL.LL
    L.LL.LL.LL
    L.LLLLL.LL
    ..L.L.....
    LLLLLLLLLL
    L.LLLLLL.L
    L.LLLLL.LL
    Now, you just need to model the people who will be arriving shortly. Fortunately, people are entirely predictable and always follow a simple set of rules. All decisions are based on the number of occupied seats adjacent to a given seat (one of the eight positions immediately up, down, left, right, or diagonal from the seat). The following rules are applied to every seat simultaneously:

    If a seat is empty (L) and there are no occupied seats adjacent to it, the seat becomes occupied.
    If a seat is occupied (#) and four or more seats adjacent to it are also occupied, the seat becomes empty.
    Otherwise, the seat's state does not change.
    Floor (.) never changes; seats don't move, and nobody sits on the floor.

    After one round of these rules, every seat in the example layout becomes occupied:

    #.##.##.##
    #######.##
    #.#.#..#..
    ####.##.##
    #.##.##.##
    #.#####.##
    ..#.#.....
    ##########
    #.######.#
    #.#####.##
    After a second round, the seats with four or more occupied adjacent seats become empty again:

    #.LL.L#.##
    #LLLLLL.L#
    L.L.L..L..
    #LLL.LL.L#
    #.LL.LL.LL
    #.LLLL#.##
    ..L.L.....
    #LLLLLLLL#
    #.LLLLLL.L
    #.#LLLL.##
    This process continues for three more rounds:

    #.##.L#.##
    #L###LL.L#
    L.#.#..#..
    #L##.##.L#
    #.##.LL.LL
    #.###L#.##
    ..#.#.....
    #L######L#
    #.LL###L.L
    #.#L###.##
    #.#L.L#.##
    #LLL#LL.L#
    L.L.L..#..
    #LLL.##.L#
    #.LL.LL.LL
    #.LL#L#.##
    ..L.L.....
    #L#LLLL#L#
    #.LLLLLL.L
    #.#L#L#.##
    #.#L.L#.##
    #LLL#LL.L#
    L.#.L..#..
    #L##.##.L#
    #.#L.LL.LL
    #.#L#L#.##
    ..L.L.....
    #L#L##L#L#
    #.LLLLLL.L
    #.#L#L#.##
    At this point, something interesting happens: the chaos stabilizes and further applications of these rules cause no seats to change state! Once people stop moving around, you count 37 occupied seats.

    Simulate your seating area by applying the seating rules repeatedly until no seats change state. How many seats end up occupied?
    */
    std::string solve_day_11_1(const std::filesystem::path& input_dir) {
        auto grid = get_input(input_dir);
        int num_changes = 1, num_iters = 0;
        while (num_changes > 0) {
            auto changes = iterate<ruleset_1>(grid);
            apply_changes(grid, changes);
            num_changes = changes.size();
            ++num_iters;
            //std::cout << "Iteration " << num_iters << " (# Changes = " << num_changes << "):\n" << grid << '\n';
        }
        return std::to_string(num_occupied(grid));
    }

    /*
    As soon as people start to arrive, you realize your mistake. People don't just care about adjacent seats - they care about the first seat they can see in each of those eight directions!

    Now, instead of considering just the eight immediately adjacent seats, consider the first seat in each of those eight directions. For example, the empty seat below would see eight occupied seats:

    .......#.
    ...#.....
    .#.......
    .........
    ..#L....#
    ....#....
    .........
    #........
    ...#.....
    The leftmost empty seat below would only see one empty seat, but cannot see any of the occupied ones:

    .............
    .L.L.#.#.#.#.
    .............
    The empty seat below would see no occupied seats:

    .##.##.
    #.#.#.#
    ##...##
    ...L...
    ##...##
    #.#.#.#
    .##.##.
    Also, people seem to be more tolerant than you expected: it now takes five or more visible occupied seats for an occupied seat to become empty (rather than four or more from the previous rules). The other rules still apply: empty seats that see no occupied seats become occupied, seats matching no rule don't change, and floor never changes.

    Given the same starting layout as above, these new rules cause the seating area to shift around as follows:

    L.LL.LL.LL
    LLLLLLL.LL
    L.L.L..L..
    LLLL.LL.LL
    L.LL.LL.LL
    L.LLLLL.LL
    ..L.L.....
    LLLLLLLLLL
    L.LLLLLL.L
    L.LLLLL.LL
    #.##.##.##
    #######.##
    #.#.#..#..
    ####.##.##
    #.##.##.##
    #.#####.##
    ..#.#.....
    ##########
    #.######.#
    #.#####.##
    #.LL.LL.L#
    #LLLLLL.LL
    L.L.L..L..
    LLLL.LL.LL
    L.LL.LL.LL
    L.LLLLL.LL
    ..L.L.....
    LLLLLLLLL#
    #.LLLLLL.L
    #.LLLLL.L#
    #.L#.##.L#
    #L#####.LL
    L.#.#..#..
    ##L#.##.##
    #.##.#L.##
    #.#####.#L
    ..#.#.....
    LLL####LL#
    #.L#####.L
    #.L####.L#
    #.L#.L#.L#
    #LLLLLL.LL
    L.L.L..#..
    ##LL.LL.L#
    L.LL.LL.L#
    #.LLLLL.LL
    ..L.L.....
    LLLLLLLLL#
    #.LLLLL#.L
    #.L#LL#.L#
    #.L#.L#.L#
    #LLLLLL.LL
    L.L.L..#..
    ##L#.#L.L#
    L.L#.#L.L#
    #.L####.LL
    ..#.#.....
    LLL###LLL#
    #.LLLLL#.L
    #.L#LL#.L#
    #.L#.L#.L#
    #LLLLLL.LL
    L.L.L..#..
    ##L#.#L.L#
    L.L#.LL.L#
    #.LLLL#.LL
    ..#.L.....
    LLL###LLL#
    #.LLLLL#.L
    #.L#LL#.L#
    Again, at this point, people stop shifting around and the seating area reaches equilibrium. Once this occurs, you count 26 occupied seats.

    Given the new visibility method and the rule change for occupied seats becoming empty, once equilibrium is reached, how many seats end up occupied?
    */
    std::string solve_day_11_2(const std::filesystem::path& input_dir) {
        auto grid = get_input(input_dir);
        int num_changes = 1, num_iters = 0;
        while (num_changes > 0) {
            auto changes = iterate<ruleset_2>(grid);
            apply_changes(grid, changes);
            num_changes = changes.size();
            ++num_iters;
            //std::cout << "Iteration " << num_iters << " (# Changes = " << num_changes << "):\n" << grid << '\n';
        }
        return std::to_string(num_occupied(grid));
    }

    static aoc::registration r {2020, 11, solve_day_11_1, solve_day_11_2};

} /* namespace aoc2020 */