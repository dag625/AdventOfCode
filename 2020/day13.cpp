//
// Created by Daniel Garcia on 12/13/20.
//

#include "registration.h"
#include "utilities.h"

#include <iostream>
#include <charconv>
#include <optional>
#include <cstdint>
#include <numeric>

namespace fs = std::filesystem;

namespace aoc2020 {

    using namespace aoc;

    namespace {

        struct bus {
            int idx = 0;
            int id = 0;
            bus() = default;
            bus(int index, int bus_id) : idx{index}, id{bus_id} {}
        };

        struct notes {
            int current_time = 0;
            std::vector<bus> bus_ids;
        };

        /*
         * Sources for part 2:
         * https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
         * http://homepages.math.uic.edu/~leon/mcs425-s08/handouts/chinese_remainder.pdf
         * https://medium.com/@astartekraus/the-chinese-remainder-theorem-ea110f48248c
         *
         * The first two got me most of the way there.  The final source pointed out the
         * correct way to handle negative answers.  I also think that one of the part 2
         * examples has the wrong answer.  I don't get '17,x,13,19' as 3417, I get 782
         * which seems to be valid.
         */

        int64_t get_coeff(int64_t a, int64_t b) {
            using pair = std::pair<int64_t, int64_t>;
            pair r {a, b};
            pair s {1, 0};
            pair t {0, 1};

            while (r.second != 0) {
                auto q = r.first / r.second;
                r = { r.second, r.first - q * r.second };
                s = { s.second, s.first - q * s.second };
                t = { t.second, t.first - q * t.second };
            }

            return s.first;
        }

        using iter = std::vector<bus>::const_iterator;

        int64_t find_crt_solution(const std::vector<bus>& buses) {
            const int64_t m = std::accumulate(buses.begin(), buses.end(), 1LL, [](int64_t acc, bus b){ return acc *= b.id; });
            int64_t retval = 0;
            for (auto b : buses) {
                const int64_t mb = m / b.id;
                retval += b.idx * get_coeff(mb, b.id) * mb;
            }
            return (retval > 0 ? retval : (-retval + m)) % m;
        }

        bool verify_part_2(const std::vector<bus>& buses, int64_t t) {
            bool good = true;
            for (auto b : buses) {
                if (t % b.id != b.idx) {
                    good = false;
                }
            }
            return good;
        }

        notes get_input(const std::vector<std::string>& lines) {
            using namespace std::string_view_literals;
            if (lines.size() < 2) {
                throw std::runtime_error{"File does not contain expected data."};
            }

            int current_time = 0;
            auto res = std::from_chars(lines[0].data(), lines[0].data() + lines[0].size(), current_time);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }

            auto ids = split(lines[1], ',');
            std::vector<bus> bus_ids;
            bus_ids.reserve(ids.size());
            for (int i = 0; i < ids.size(); ++i) {
                if (ids[i] == "x"sv) {
                    continue;
                }
                else {
                    int id = 0;
                    res = std::from_chars(ids[i].data(), ids[i].data() + ids[i].size(), id);
                    if (res.ec != std::errc{}) {
                        throw std::system_error{std::make_error_code(res.ec)};
                    }
                    bus_ids.emplace_back(i, id);
                }
            }

            return { current_time, std::move(bus_ids) };
        }

    }

    /************************* Part 1 *************************/
    std::string solve_day_13_1(const std::vector<std::string>& lines) {
        auto schedule = get_input(lines);
        std::vector<int> wait;
        wait.reserve(schedule.bus_ids.size());
        std::transform(schedule.bus_ids.begin(), schedule.bus_ids.end(), std::back_inserter(wait),
                       [t = schedule.current_time](bus b){ return b.id - (t % b.id); });
        auto min_wait = std::min_element(wait.begin(), wait.end());
        auto idx = std::distance(wait.begin(), min_wait);
        return std::to_string(*min_wait * schedule.bus_ids[idx].id);
    }

    /************************* Part 2 *************************/
    std::string solve_day_13_2(const std::vector<std::string>& lines) {
        auto schedule = get_input(lines);
        verify_part_2(schedule.bus_ids, find_crt_solution(schedule.bus_ids));
        return std::to_string(find_crt_solution(schedule.bus_ids));
    }

    static aoc::registration r {2020, 13, solve_day_13_1, solve_day_13_2};

} /* namespace aoc2020 */