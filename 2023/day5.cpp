//
// Created by Dan on 12/4/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct range {
        int64_t dst;
        int64_t src;
        int64_t len;

        [[nodiscard]] int64_t delta() const noexcept { return dst - src; }

        [[nodiscard]] std::optional<int64_t> is_in(int64_t start, int64_t length) const noexcept {
            //Given range contains this range
            if (start <= src && start + length >= src + len) {
                return src;
            }
                //Given range is contained by this range
            else if (start > src && start + length < src + len) {
                return start;
            }
                //Given range starts before and ends in this range
            else if (start < src && start + length > src && start + length <= src + len) {
                return src;
            }
                //Given range starts in and ends after this range
            else if (start >= src && start < src + len && start + length > src + len) {
                return start;
            }
            return std::nullopt;
        }

        bool operator<(const range &rhs) const noexcept {
            return dst < rhs.dst;
        }
    };

    std::ostream &operator<<(std::ostream &os, const range &r) {
        os << r.src << " -> " << r.dst << " @ " << r.len;
        return os;
    }

}

template <> struct fmt::formatter<range> : ostream_formatter {};

namespace {

    bool sort_by_dst(const range& lhs, const range& rhs) noexcept {
        return lhs.dst < rhs.dst;
    }

    bool sort_by_src(const range& lhs, const range& rhs) noexcept {
        return lhs.src < rhs.src;
    }

    struct input {
        std::vector<int64_t> seeds;
        std::vector<std::vector<range>> conversions;
    };

    int64_t parse_num(std::string_view s) { return parse<int64_t>(s); }

    input parse_lines(const std::vector<std::string>& lines) {
        const auto seed_strs = split_no_empty(split(lines.front(), ':')[1], ' ');
        std::vector<std::vector<range>> conversions;
        for (int i = 3; i < lines.size(); ++i) {
            std::vector<range> r;
            for (; i < lines.size() && !lines[i].empty(); ++i) {
                if (!isdigit(lines[i].front())) {
                    continue;
                }
                const auto parts = split(lines[i], ' ');
                r.push_back({parse_num(parts[0]), parse_num(parts[1]), parse_num(parts[2])});
            }
            std::sort(r.begin(), r.end());
            conversions.push_back(std::move(r));
        }
        return {seed_strs | std::views::transform(&parse_num) | to<std::vector<int64_t>>(), std::move(conversions)};
    }

    input get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2023" / "day_5_input.txt");
        return parse_lines(lines);
    }

    int64_t convert(const int64_t in, const std::vector<range>& conversion) {
        int64_t out = in;
        for (const auto& r : conversion) {
            if (out >= r.src && out < r.src + r.len) {
                const auto del = r.dst - r.src;
                out += del;
                break;
            }
        }
        return out;
    }

    std::vector<range> pad_src(const std::vector<range>& in) {
        auto retval = in;
        const auto [min, max] = std::minmax_element(in.begin(), in.end(), [](const range& a, const range& b){ return a.src < b.src; });
        retval.insert(retval.begin(), {0, 0, min->src});
        retval.push_back({max->src + max->len, max->src + max->len, std::numeric_limits<int64_t>::max() - max->src - max->len});
        return retval;
    }

    std::vector<range> combine(std::vector<range> from, std::vector<range> to) {
        std::sort(from.begin(), from.end(), &sort_by_src);
        std::sort(to.begin(), to.end(), &sort_by_src);
        std::vector<range> retval;
        retval.reserve(from.size() + to.size());
        for (const auto& f : from) {
            const auto fdelta = f.dst - f.src;
            auto src = f.src;
            auto dst = f.dst;
            auto remaining = f.len;
            for (const auto& t : to) {
                if (remaining == 0) {
                    break;
                }
                const auto tdelta = t.dst - t.src;
                const auto delta = fdelta + tdelta;
                //to fully containts from
                if (t.src <= dst && t.src + t.len >= dst + remaining) {
                    retval.push_back({src + delta, src, remaining});
                    src += remaining;
                    dst += remaining;
                    remaining = 0;
                    break;
                }
                //to and from are disjoint, to comes before from
                else if (t.src + t.len <= dst) {
                    continue;
                }
                //to and from are disjoint, to comes after from
                else if (t.src >= dst + remaining) {
                    retval.push_back({dst, src, remaining});
                    src += remaining;
                    dst += remaining;
                    remaining = 0;
                    break;
                }
                //to begins before from, ends inside it
                else if (t.src <= dst && t.src + t.len < dst + remaining) {
                    const auto len = t.src + t.len - dst;
                    retval.push_back({src + delta, src, len});
                    src += len;
                    dst += len;
                    remaining -= len;
                }
                //to begins after and inside from
                else if (t.src > dst && t.src < dst + remaining) {
                    const auto passthru_len = t.src - dst;
                    retval.push_back({dst, src, passthru_len});
                    src += passthru_len;
                    dst += passthru_len;
                    remaining -= passthru_len;

                    const auto len = std::min(t.len, remaining);
                    retval.push_back({src + delta, src, len});
                    src += len;
                    dst += len;
                    remaining -= len;
                }
            }
            if (remaining > 0) {
                retval.push_back({dst, src, remaining});
            }
        }
        return retval;
    }

    int64_t lowest_in_range(int64_t start, int64_t len, const std::vector<range>& conversion) {
        auto lowest = std::numeric_limits<int64_t>::max();
        for (const auto& r : conversion) {
            const auto first = r.is_in(start, len);
            if (first) {
                const auto conv = *first + r.delta();
                if (conv < lowest) {
                    lowest = conv;
                }
            }
        }
        return lowest;
    }

    /*
    --- Day 5: If You Give A Seed A Fertilizer ---
    You take the boat and find the gardener right where you were told he would be: managing a giant "garden" that looks more to you like a farm.

    "A water source? Island Island is the water source!" You point out that Snow Island isn't receiving any water.

    "Oh, we had to stop the water because we ran out of sand to filter it with! Can't make snow with dirty water. Don't worry, I'm sure we'll get more sand soon; we only turned off the water a few days... weeks... oh no." His face sinks into a look of horrified realization.

    "I've been so busy making sure everyone here has food that I completely forgot to check why we stopped getting more sand! There's a ferry leaving soon that is headed over in that direction - it's much faster than your boat. Could you please go check it out?"

    You barely have time to agree to this request when he brings up another. "While you wait for the ferry, maybe you can help us with our food production problem. The latest Island Island Almanac just arrived and we're having trouble making sense of it."

    The almanac (your puzzle input) lists all of the seeds that need to be planted. It also lists what type of soil to use with each kind of seed, what type of fertilizer to use with each kind of soil, what type of water to use with each kind of fertilizer, and so on. Every type of seed, soil, fertilizer and so on is identified with a number, but numbers are reused by each category - that is, soil 123 and fertilizer 123 aren't necessarily related to each other.

    For example:

    seeds: 79 14 55 13

    seed-to-soil map:
    50 98 2
    52 50 48

    soil-to-fertilizer map:
    0 15 37
    37 52 2
    39 0 15

    fertilizer-to-water map:
    49 53 8
    0 11 42
    42 0 7
    57 7 4

    water-to-light map:
    88 18 7
    18 25 70

    light-to-temperature map:
    45 77 23
    81 45 19
    68 64 13

    temperature-to-humidity map:
    0 69 1
    1 0 69

    humidity-to-location map:
    60 56 37
    56 93 4
    The almanac starts by listing which seeds need to be planted: seeds 79, 14, 55, and 13.

    The rest of the almanac contains a list of maps which describe how to convert numbers from a source category into numbers in a destination category. That is, the section that starts with seed-to-soil map: describes how to convert a seed number (the source) to a soil number (the destination). This lets the gardener and his team know which soil to use with which seeds, which water to use with which fertilizer, and so on.

    Rather than list every source number and its corresponding destination number one by one, the maps describe entire ranges of numbers that can be converted. Each line within a map contains three numbers: the destination range start, the source range start, and the range length.

    Consider again the example seed-to-soil map:

    50 98 2
    52 50 48
    The first line has a destination range start of 50, a source range start of 98, and a range length of 2. This line means that the source range starts at 98 and contains two values: 98 and 99. The destination range is the same length, but it starts at 50, so its two values are 50 and 51. With this information, you know that seed number 98 corresponds to soil number 50 and that seed number 99 corresponds to soil number 51.

    The second line means that the source range starts at 50 and contains 48 values: 50, 51, ..., 96, 97. This corresponds to a destination range starting at 52 and also containing 48 values: 52, 53, ..., 98, 99. So, seed number 53 corresponds to soil number 55.

    Any source numbers that aren't mapped correspond to the same destination number. So, seed number 10 corresponds to soil number 10.

    So, the entire list of seed numbers and their corresponding soil numbers looks like this:

    seed  soil
    0     0
    1     1
    ...   ...
    48    48
    49    49
    50    52
    51    53
    ...   ...
    96    98
    97    99
    98    50
    99    51
    With this map, you can look up the soil number required for each initial seed number:

    Seed number 79 corresponds to soil number 81.
    Seed number 14 corresponds to soil number 14.
    Seed number 55 corresponds to soil number 57.
    Seed number 13 corresponds to soil number 13.
    The gardener and his team want to get started as soon as possible, so they'd like to know the closest location that needs a seed. Using these maps, find the lowest location number that corresponds to any of the initial seeds. To do this, you'll need to convert each seed number through other categories until you can find its corresponding location number. In this example, the corresponding types are:

    Seed 79, soil 81, fertilizer 81, water 81, light 74, temperature 78, humidity 78, location 82.
    Seed 14, soil 14, fertilizer 53, water 49, light 42, temperature 42, humidity 43, location 43.
    Seed 55, soil 57, fertilizer 57, water 53, light 46, temperature 82, humidity 82, location 86.
    Seed 13, soil 13, fertilizer 52, water 41, light 34, temperature 34, humidity 35, location 35.
    So, the lowest location number in this example is 35.

    What is the lowest location number that corresponds to any of the initial seed numbers?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        int64_t lowest = std::numeric_limits<int64_t>::max();
        for (int64_t val : input.seeds) {
            for (const auto& c : input.conversions) {
                val = convert(val, c);
            }
            if (val < lowest) {
                lowest = val;
            }
        }
        return std::to_string(lowest);
    }

    /*
    --- Part Two ---
    Everyone will starve if you only plant such a small number of seeds. Re-reading the almanac, it looks like the seeds: line actually describes ranges of seed numbers.

    The values on the initial seeds: line come in pairs. Within each pair, the first value is the start of the range and the second value is the length of the range. So, in the first line of the example above:

    seeds: 79 14 55 13
    This line describes two ranges of seed numbers to be planted in the garden. The first range starts with seed number 79 and contains 14 values: 79, 80, ..., 91, 92. The second range starts with seed number 55 and contains 13 values: 55, 56, ..., 66, 67.

    Now, rather than considering four seed numbers, you need to consider a total of 27 seed numbers.

    In the above example, the lowest location number can be obtained from seed number 82, which corresponds to soil 84, fertilizer 84, water 84, light 77, temperature 45, humidity 46, and location 46. So, the lowest location number is 46.

    Consider all of the initial seed numbers listed in the ranges on the first line of the almanac. What is the lowest location number that corresponds to any of the initial seed numbers?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        auto total = pad_src(input.conversions.front());
        for (const auto& r : input.conversions | std::views::drop(1)) {
            total = combine(total, r);
        }

        auto lowest = std::numeric_limits<int64_t>::max();
        for (const auto pair : input.seeds | std::views::chunk(2)) {
            const auto l = lowest_in_range(pair[0], pair[1], total);
            if (l < lowest) {
                lowest = l;
            }
        }
        return std::to_string(lowest);
    }

    aoc::registration r{2023, 5, part_1, part_2};

    TEST_SUITE("2023_day05") {
        TEST_CASE("2023_day05:example") {
            using namespace std::string_literals;
            std::vector<std::string> lines {
                    "seeds: 79 14 55 13"s,
                    ""s,
                    "seed-to-soil map:"s,
                    "50 98 2"s,
                    "52 50 48"s,
                    ""s,
                    "soil-to-fertilizer map:"s,
                    "0 15 37"s,
                    "37 52 2"s,
                    "39 0 15"s,
                    ""s,
                    "fertilizer-to-water map:"s,
                    "49 53 8"s,
                    "0 11 42"s,
                    "42 0 7"s,
                    "57 7 4"s,
                    ""s,
                    "water-to-light map:"s,
                    "88 18 7"s,
                    "18 25 70"s,
                    ""s,
                    "light-to-temperature map:"s,
                    "45 77 23"s,
                    "81 45 19"s,
                    "68 64 13"s,
                    ""s,
                    "temperature-to-humidity map:"s,
                    "0 69 1"s,
                    "1 0 69"s,
                    ""s,
                    "humidity-to-location map:"s,
                    "60 56 37"s,
                    "56 93 4"s
            };
            const auto input = parse_lines(lines);
            int64_t lowest = std::numeric_limits<int64_t>::max();
            for (int64_t val : input.seeds) {
                fmt::print("Seed = {}\n", val);
                for (const auto& c : input.conversions) {
                    const auto tmp = convert(val, c);
                    fmt::print("\tFrom {} -> {}\n", val, tmp);
                    val = tmp;
                }
                if (val < lowest) {
                    lowest = val;
                }
            }
            CHECK_EQ(lowest, 35);

            auto total = pad_src(input.conversions.front());
            for (const auto& r : input.conversions | std::views::drop(1)) {
                fmt::print("\nStep\n");
                for (const auto& t : total) {
                    fmt::print("{}\n", t);
                }
                total = combine(total, r);
            }
            fmt::print("\nFINAL\n");
            for (const auto& r : total) {
                fmt::print("{}\n", r);
            }
            auto p2_lowest = std::numeric_limits<int64_t>::max();
            for (const auto pair : input.seeds | std::views::chunk(2)) {
                const auto l = lowest_in_range(pair[0], pair[1], total);
                if (l < p2_lowest) {
                    p2_lowest = l;
                }
            }
            CHECK_EQ(p2_lowest, 46);
        }
        TEST_CASE("2023_day05:combine") {
            std::vector<range> to {
                    {12, 10, 6},
                    {10, 16, 2}
                },
                from{
                    //{0, 0, 5},
                    {8, 5, 9},
                    {5, 14, 3}//,
                    //{17, 17, std::numeric_limits<int64_t>::max() - 17}
            };
            from = pad_src(from);
            const auto result = combine(from, to);
            for (const auto& r : result) {
                fmt::print("{}\n", r);
            }
            for (int64_t val = 0; val < 20; ++val) {
                const auto intermediate = convert(val, from);
                const auto indirect = convert(intermediate, to);
                const auto direct = convert(val, result);
                CHECK_MESSAGE(indirect == direct, "Value ", val, " conversion did not match:  Indirect = ", indirect, "; Direct = ", direct);
            }
        }
    }

}