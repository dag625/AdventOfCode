//
// Created by Daniel Garcia on 6/15/21.
//


#include "registration.h"
#include "utilities.h"
#include "parse.h"

#include <doctest/doctest.h>

#include <string_view>
#include <regex>
#include <array>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    std::regex regex_input {R"(^([A-Za-z]+) can fly (\d+) km/s for (\d+) seconds, but then must rest for (\d+) seconds.$)"};

    struct reindeer {
        std::string name;
        int speed;
        int speed_max_t;
        int rest_t;
        int period;
        reindeer(std::string n, int s, int st, int rt) : name{std::move(n)}, speed{s}, speed_max_t{st}, rest_t{rt}, period{st + rt} {}
    };

    struct racer : public reindeer {
        int dist = 0;
        int points = 0;
        racer(reindeer&& r) : reindeer{std::move(r)} {}
        racer(const reindeer& r) : reindeer{r} {}
        [[nodiscard]] bool operator<(const racer& r) const noexcept {
            return dist < r.dist;
        }
        int iterate(const int tm) {
            dist += (tm % period < speed_max_t ? speed : 0);
            return dist;
        }
    };

    reindeer parse(std::string_view s) {
        std::match_results<std::string_view::iterator> match;
        if (std::regex_match(s.begin(), s.end(), match, regex_input) && match.size() == 5) {
            return reindeer{match[1].str(), aoc::parse<int>(match[2].str()), aoc::parse<int>(match[3].str()), aoc::parse<int>(match[4].str()) };
        }
        else {
            throw std::runtime_error{"Failed to parse reindeer entry."};
        }
    }

    std::vector<reindeer> get_input(const std::filesystem::path& input_dir) {
        auto lines = aoc::read_file_lines(input_dir / "2015" / "day_14_input.txt");
        std::vector<reindeer> retval;
        retval.reserve(lines.size());
        std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse);
        return retval;
    }

    std::vector<racer> to_racers(std::vector<reindeer> deer) {
        std::vector<racer> retval;
        retval.reserve(deer.size());
        for (auto& d : deer) {
            retval.emplace_back(std::move(d));
        }
        return retval;
    }

    int distance_after(const reindeer& r, const int seconds) {
        const int period_dist = r.speed_max_t * r.speed;
        const int num_periods = seconds / r.period, rem = seconds % r.period;
        return num_periods * period_dist + (rem < r.speed_max_t ? rem * r.speed : period_dist);
    }

    /*
    This year is the Reindeer Olympics! Reindeer can fly at high speeds, but must rest occasionally to recover their energy. Santa would like to know which of his reindeer is fastest, and so he has them race.

    Reindeer can only either be flying (always at their top speed) or resting (not moving at all), and always spend whole seconds in either state.

    For example, suppose you have the following Reindeer:

    Comet can fly 14 km/s for 10 seconds, but then must rest for 127 seconds.
    Dancer can fly 16 km/s for 11 seconds, but then must rest for 162 seconds.
    After one second, Comet has gone 14 km, while Dancer has gone 16 km. After ten seconds, Comet has gone 140 km, while Dancer has gone 160 km. On the eleventh second, Comet begins resting (staying at 140 km), and Dancer continues on for a total distance of 176 km. On the 12th second, both reindeer are resting. They continue to rest until the 138th second, when Comet flies for another ten seconds. On the 174th second, Dancer flies for another 11 seconds.

    In this example, after the 1000th second, both reindeer are resting, and Comet is in the lead at 1120 km (poor Dancer has only gotten 1056 km by that point). So, in this situation, Comet would win (if the race ended at 1000 seconds).

    Given the descriptions of each reindeer (in your puzzle input), after exactly 2503 seconds, what distance has the winning reindeer traveled?
     */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto deer = get_input(input_dir);
        std::vector<int> dist;
        dist.reserve(deer.size());
        std::transform(deer.begin(), deer.end(), std::back_inserter(dist), [](const reindeer& r){ return distance_after(r, 2503); });
        return std::to_string(*std::max_element(dist.begin(), dist.end()));
    }

    /*
    Seeing how reindeer move in bursts, Santa decides he's not pleased with the old scoring system.

    Instead, at the end of each second, he awards one point to the reindeer currently in the lead. (If there are multiple reindeer tied for the lead, they each get one point.) He keeps the traditional 2503 second time limit, of course, as doing otherwise would be entirely ridiculous.

    Given the example reindeer from above, after the first second, Dancer is in the lead and gets one point. He stays in the lead until several seconds into Comet's second burst: after the 140th second, Comet pulls into the lead and gets his first point. Of course, since Dancer had been in the lead for the 139 seconds before that, he has accumulated 139 points by the 140th second.

    After the 1000th second, Dancer has accumulated 689 points, while poor Comet, our old champion, only has 312. So, with the new scoring system, Dancer would win (if the race ended at 1000 seconds).

    Again given the descriptions of each reindeer (in your puzzle input), after exactly 2503 seconds, how many points does the winning reindeer have?
     */
    std::string part_2(const std::filesystem::path& input_dir) {
        auto racers = to_racers(get_input(input_dir));
        for (int i = 0; i < 2503; ++i) {
            int best = 0;
            racer* best_r = nullptr;
            for (auto& r : racers) {
                auto d = r.iterate(i);
                if (d > best) {
                    best = d;
                    best_r = &r;
                }
            }
            ++best_r->points;
        }
        return std::to_string(std::max_element(racers.begin(), racers.end(),
                                               [](const racer& a, const racer& b){ return a.points < b.points; })
                                               ->points);
    }

    aoc::registration r {2015, 14, part_1, part_2};

    TEST_SUITE("2015_day14") {
        TEST_CASE("2015_day14:example") {

        }
    }

}

