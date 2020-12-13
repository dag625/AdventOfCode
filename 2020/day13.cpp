//
// Created by Daniel Garcia on 12/13/20.
//

#include "day13.h"
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
            const int64_t m = std::accumulate(buses.begin(), buses.end(), 1LL, [](int64_t acc, bus b){ return acc *= b.id; });
            //std::cout << "Upper bound = " << m << '\n';
            bool good = true;
            for (auto b : buses) {
                if (t % b.id != b.idx) {
                    good = false;
                }
                //std::cout << "Bus #" << b.idx << " (ID = " << b.id << ") has remainder " << t % b.id << ".\n";
            }
            return good;
        }

        notes get_input(const fs::path &input_dir) {
            using namespace std::string_view_literals;
            auto lines = read_file_lines(input_dir / "2020" / "day_13_input.txt");
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

    /*
    Your ferry can make it safely to a nearby port, but it won't get much further. When you call to book another ship, you discover that no ships embark from that port to your vacation island. You'll need to get from the port to the nearest airport.

    Fortunately, a shuttle bus service is available to bring you from the sea port to the airport! Each bus has an ID number that also indicates how often the bus leaves for the airport.

    Bus schedules are defined based on a timestamp that measures the number of minutes since some fixed reference point in the past. At timestamp 0, every bus simultaneously departed from the sea port. After that, each bus travels to the airport, then various other locations, and finally returns to the sea port to repeat its journey forever.

    The time this loop takes a particular bus is also its ID number: the bus with ID 5 departs from the sea port at timestamps 0, 5, 10, 15, and so on. The bus with ID 11 departs at 0, 11, 22, 33, and so on. If you are there when the bus departs, you can ride that bus to the airport!

    Your notes (your puzzle input) consist of two lines. The first line is your estimate of the earliest timestamp you could depart on a bus. The second line lists the bus IDs that are in service according to the shuttle company; entries that show x must be out of service, so you decide to ignore them.

    To save time once you arrive, your goal is to figure out the earliest bus you can take to the airport. (There will be exactly one such bus.)

    For example, suppose you have the following notes:

    939
    7,13,x,x,59,x,31,19
    Here, the earliest timestamp you could depart is 939, and the bus IDs in service are 7, 13, 59, 31, and 19. Near timestamp 939, these bus IDs depart at the times marked D:

    time   bus 7   bus 13  bus 59  bus 31  bus 19
    929      .       .       .       .       .
    930      .       .       .       D       .
    931      D       .       .       .       D
    932      .       .       .       .       .
    933      .       .       .       .       .
    934      .       .       .       .       .
    935      .       .       .       .       .
    936      .       D       .       .       .
    937      .       .       .       .       .
    938      D       .       .       .       .
    939      .       .       .       .       .
    940      .       .       .       .       .
    941      .       .       .       .       .
    942      .       .       .       .       .
    943      .       .       .       .       .
    944      .       .       D       .       .
    945      D       .       .       .       .
    946      .       .       .       .       .
    947      .       .       .       .       .
    948      .       .       .       .       .
    949      .       D       .       .       .
    The earliest bus you could take is bus ID 59. It doesn't depart until timestamp 944, so you would need to wait 944 - 939 = 5 minutes before it departs. Multiplying the bus ID by the number of minutes you'd need to wait gives 295.

    What is the ID of the earliest bus you can take to the airport multiplied by the number of minutes you'll need to wait for that bus?
    */
    void solve_day_13_1(const std::filesystem::path& input_dir) {
        auto schedule = get_input(input_dir);
        std::vector<int> wait;
        wait.reserve(schedule.bus_ids.size());
        std::transform(schedule.bus_ids.begin(), schedule.bus_ids.end(), std::back_inserter(wait),
                       [t = schedule.current_time](bus b){ return b.id - (t % b.id); });
        auto min_wait = std::min_element(wait.begin(), wait.end());
        auto idx = std::distance(wait.begin(), min_wait);
        std::cout << '\t' << *min_wait * schedule.bus_ids[idx].id << '\n';
    }

    /*
    --- Part Two ---

    The shuttle company is running a contest: one gold coin for anyone that can find the earliest timestamp such that the first bus ID departs at that time and each subsequent listed bus ID departs at that subsequent minute. (The first line in your input is no longer relevant.)

    For example, suppose you have the same list of bus IDs as above:

    7,13,x,x,59,x,31,19
    An x in the schedule means there are no constraints on what bus IDs must depart at that time.

    This means you are looking for the earliest timestamp (called t) such that:

    Bus ID 7 departs at timestamp t.
    Bus ID 13 departs one minute after timestamp t.
    There are no requirements or restrictions on departures at two or three minutes after timestamp t.
    Bus ID 59 departs four minutes after timestamp t.
    There are no requirements or restrictions on departures at five minutes after timestamp t.
    Bus ID 31 departs six minutes after timestamp t.
    Bus ID 19 departs seven minutes after timestamp t.
    The only bus departures that matter are the listed bus IDs at their specific offsets from t. Those bus IDs can depart at other times, and other bus IDs can depart at those times. For example, in the list above, because bus ID 19 must depart seven minutes after the timestamp at which bus ID 7 departs, bus ID 7 will always also be departing with bus ID 19 at seven minutes after timestamp t.

    In this example, the earliest timestamp at which this occurs is 1068781:

    time     bus 7   bus 13  bus 59  bus 31  bus 19
    1068773    .       .       .       .       .
    1068774    D       .       .       .       .
    1068775    .       .       .       .       .
    1068776    .       .       .       .       .
    1068777    .       .       .       .       .
    1068778    .       .       .       .       .
    1068779    .       .       .       .       .
    1068780    .       .       .       .       .
    1068781    D       .       .       .       .
    1068782    .       D       .       .       .
    1068783    .       .       .       .       .
    1068784    .       .       .       .       .
    1068785    .       .       D       .       .
    1068786    .       .       .       .       .
    1068787    .       .       .       D       .
    1068788    D       .       .       .       D
    1068789    .       .       .       .       .
    1068790    .       .       .       .       .
    1068791    .       .       .       .       .
    1068792    .       .       .       .       .
    1068793    .       .       .       .       .
    1068794    .       .       .       .       .
    1068795    D       D       .       .       .
    1068796    .       .       .       .       .
    1068797    .       .       .       .       .
    In the above example, bus ID 7 departs at timestamp 1068788 (seven minutes after t). This is fine; the only requirement on that minute is that bus ID 19 departs then, and it does.

    Here are some other examples:

    The earliest timestamp that matches the list 17,x,13,19 is 3417.
    67,7,59,61 first occurs at timestamp 754018.
    67,x,7,59,61 first occurs at timestamp 779210.
    67,7,x,59,61 first occurs at timestamp 1261476.
    1789,37,47,1889 first occurs at timestamp 1202161486.
    However, with so many bus IDs in your list, surely the actual earliest timestamp will be larger than 100000000000000!

    What is the earliest timestamp such that all of the listed bus IDs depart at offsets matching their positions in the list?
    */
    void solve_day_13_2(const std::filesystem::path& input_dir) {
        auto schedule = get_input(input_dir);
        verify_part_2(schedule.bus_ids, find_crt_solution(schedule.bus_ids));
        std::cout << '\t' << find_crt_solution(schedule.bus_ids) << '\n';
    }

} /* namespace aoc2020 */