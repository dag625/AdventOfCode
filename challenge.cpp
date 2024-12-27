//
// Created by Daniel Garcia on 12/23/20.
//

#include "challenge.h"
#include "time_format.h"
#include "utilities.h"

#include <tuple>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <format>

namespace aoc {

    namespace {

        std::ostream& md_tab(std::ostream& os, bool md_fmt) {
            if (!md_fmt) {
                os << '\t';
            }
            return os;
        }

        class md_newline {
            bool fmt;
            bool need_nl_esacpe = true;
        public:
            explicit md_newline(bool md_fmt) : fmt{md_fmt} {}
            explicit md_newline(bool md_fmt, bool is_para_end) : fmt{md_fmt}, need_nl_esacpe{!is_para_end} {}
            std::ostream& write(std::ostream& os) const {
                if (!fmt || !need_nl_esacpe) {
                    os << '\n';
                }
                else {
                    os << "\\\n";
                }
                return os;
            }
        };

        std::ostream& operator<<(std::ostream& os, md_newline nl) {
            return nl.write(os);
        }

        void run_challenge(const std::filesystem::path& input_dir, const challenge& c, bool md_fmt, bool hide_answers) {
            try {
                //read_file_lines(input_dir / "2024" / "day_1_input.txt")
                const auto input_file = input_dir / std::to_string(c.year) / std::format("day_{}_input.txt", c.day);
                const auto input_lines = read_file_lines(input_file, false);

                if (md_fmt) {
                    std::cout << "#### ";
                }
                std::cout << "Year " << c.year << " - Day " << c.day << " - Challenge " << c.num << md_newline{md_fmt, true};
                const auto start = std::chrono::steady_clock::now();
                const auto result = (c.func)(input_lines);
                if (!hide_answers) {
                    md_tab(std::cout, md_fmt) << result << md_newline{md_fmt};
                }
                md_tab(std::cout, md_fmt) << "Challenge time:  " << time_to_string(std::chrono::steady_clock::now() - start) << md_newline{md_fmt, true};
                std::cout.flush();
            }
            catch (const std::exception& e) {
                md_tab(std::cerr, md_fmt) << "Challenge for Year " << c.year << " - Day " << c.day << " - Challenge " << c.num <<
                    " failed due to an exception:  " << e.what() << md_newline{md_fmt, true};
                std::cout.flush();
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

    void challenges::run_all(std::optional<int> y, std::optional<int> d, std::optional<int> n, const std::filesystem::path& input_dir, bool use_markdown_output_fmt, bool hide_answers) const noexcept {
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
        std::for_each(begin, end, [&input_dir, use_markdown_output_fmt, hide_answers](const challenge& c) noexcept { run_challenge(input_dir, c, use_markdown_output_fmt, hide_answers); });
        auto dur = std::chrono::system_clock::now() - start;
        if (use_markdown_output_fmt) {
            std::cout << "#### ";
        }
        std::cout << "Finished solutions in:  " << time_to_string(dur) << '\n';
    }

} /* namespace aoc */