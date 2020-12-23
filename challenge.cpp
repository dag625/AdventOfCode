//
// Created by Daniel Garcia on 12/23/20.
//

#include "challenge.h"
#include "time_format.h"

#include <tuple>
#include <algorithm>
#include <iostream>
#include <chrono>

namespace aoc {

    namespace {

        void run_challenge(const std::filesystem::path& input_dir, const challenge& c) {
            try {
                std::cout << "Year " << c.year << " - Day " << c.day << " - Challenge " << c.num << '\n';
                const auto start = std::chrono::system_clock::now();
                std::cout << '\t' << (c.func)(input_dir) << '\n';
                std::cout << "\tChallenge time:  " << (std::chrono::system_clock::now() - start) << '\n';
            }
            catch (const std::exception& e) {
                std::cerr << "\tChallenge for Year " << c.year << " - Day " << c.day << " - Challenge " << c.num <<
                    " failed due to an exception:  " << e.what() << '\n';
            }
        }

    }

    static bool operator<(const challenge &a, const challenge &b) noexcept {
        return std::tie(a.year, a.day, a.num) < std::tie(b.year, b.day, b.num);
    }

    static bool operator==(const challenge &a, const std::tuple<int, int, int> &b) noexcept {
        return std::tie(a.year, a.day, a.num) == b;
    }

    static bool operator<(const challenge &a, const std::tuple<int, int, int> &b) noexcept {
        return std::tie(a.year, a.day, a.num) < b;
    }

    static bool operator<(const std::tuple<int, int, int> &a, const challenge &b) noexcept {
        return a < std::tie(b.year, b.day, b.num);
    }

    static bool operator==(const challenge &a, const std::tuple<int, int> &b) noexcept {
        return std::tie(a.year, a.day) == b;
    }

    static bool operator<(const challenge &a, const std::tuple<int, int> &b) noexcept {
        return std::tie(a.year, a.day) < b;
    }

    static bool operator<(const std::tuple<int, int> &a, const challenge &b) noexcept {
        return a < std::tie(b.year, b.day);
    }

    static bool operator==(const challenge &a, int b) noexcept {
        return a.year == b;
    }

    static bool operator<(const challenge &a, int b) noexcept {
        return a.year < b;
    }

    static bool operator<(int a, const challenge & b) noexcept {
        return a < b.year;
    }

    challenges& challenges::list() {
        static challenges impl;
        return impl;
    }

    void challenges::add(int y, int d, int n, challenge::function f) noexcept {
        m_challenges.emplace_back(y, d, n, f);
        std::sort(m_challenges.begin(), m_challenges.end());
    }

    void challenges::run_all(std::optional<int> y, std::optional<int> d, std::optional<int> n, const std::filesystem::path& input_dir) const noexcept {
        if (!y && (d || n)) {
            std::cerr << "Cannot specify day or challenge number without specifying the year.\n";
            return;
        }
        else if (!d && n) {
            std::cerr << "Cannot specify challenge number without specifying the day.\n";
            return;
        }
        std::vector<challenge>::const_iterator begin, end;
        if (!y) {
            begin = m_challenges.begin();
            end = m_challenges.end();
        }
        else if (y && !d) {
            begin = std::lower_bound(m_challenges.begin(), m_challenges.end(), *y);
            end = std::upper_bound(m_challenges.begin(), m_challenges.end(), *y);
        }
        else if (y && d && !n) {
            begin = std::lower_bound(m_challenges.begin(), m_challenges.end(), std::make_tuple(*y, *d));
            end = std::upper_bound(m_challenges.begin(), m_challenges.end(), std::make_tuple(*y, *d));
        }
        else {
            begin = std::lower_bound(m_challenges.begin(), m_challenges.end(), std::make_tuple(*y, *d, *n));
            end = std::upper_bound(m_challenges.begin(), m_challenges.end(), std::make_tuple(*y, *d, *n));
        }
        const auto start = std::chrono::system_clock::now();
        std::for_each(begin, end, [&input_dir](const challenge& c) noexcept { run_challenge(input_dir, c); });
        auto dur = std::chrono::system_clock::now() - start;
        std::cout << "Finished solutions in:  " << dur << '\n';
    }

} /* namespace aoc */