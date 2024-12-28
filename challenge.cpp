//
// Created by Daniel Garcia on 12/23/20.
//

#include "challenge.h"
#include "time_format.h"
#include "utilities.h"
#include "parse.h"

#include <tuple>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <format>
#include <filesystem>
#include <fstream>
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

        struct cached_input {
            int year = 0;
            int day = 0;
            std::string input{};
            std::vector<std::string> lines{};

            cached_input() = default;
            cached_input(int y, int d) : year{y}, day{d} {}

            auto operator<=>(const cached_input &rhs) const { return std::tie(year, day) <=> std::tie(rhs.year, rhs.day); }
            bool operator==(const cached_input &rhs) const { return std::tie(year, day) == std::tie(rhs.year, rhs.day); }
        };

        auto operator<=>(const cached_input &a, const challenge &b) { return std::tie(a.year, a.day) <=> std::tie(b.year, b.day); }
        auto operator<=>(const cached_input &a, const std::tuple<int, int, int> &b) { return std::tie(a.year, a.day) <=> std::tie(std::get<0>(b), std::get<1>(b)); }
        auto operator<=>(const cached_input &a, const std::tuple<int, int> &b) { return std::tie(a.year, a.day) <=> b; }
        auto operator<=>(const cached_input &a, const int b) { return a.year <=> b; }

        bool operator==(const cached_input &a, const challenge &b) { return std::tie(a.year, a.day) == std::tie(b.year, b.day); }
        bool operator==(const cached_input &a, const std::tuple<int, int, int> &b) { return std::tie(a.year, a.day) == std::tie(std::get<0>(b), std::get<1>(b)); }
        bool operator==(const cached_input &a, const std::tuple<int, int> &b) { return std::tie(a.year, a.day) == b; }
        bool operator==(const cached_input &a, const int b) { return a.year == b; }

        void run_challenge(const std::vector<cached_input>& inputs, const challenge& c, bool md_fmt, bool hide_answers) {
            try {
                const auto found = std::lower_bound(inputs.begin(), inputs.end(), c);

                if (md_fmt) {
                    std::cout << "#### ";
                }
                std::cout << "Year " << c.year << " - Day " << c.day << " - Challenge " << c.num << md_newline{md_fmt, true};
                if (found == inputs.end() || *found != c) {
                    throw std::runtime_error{std::format("No cached inputs found for year {} and day {}.", c.year, c.day)};
                }
                const auto start = std::chrono::steady_clock::now();
                const auto result = (c.func)(found->lines);
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

        std::optional<cached_input> cache_input(int y, int d, const std::filesystem::path& input_dir) {
            const auto input_file = input_dir / std::to_string(y) / std::format("day_{}_input.txt", d);
            if (!std::filesystem::exists(input_file)) {
                return std::nullopt;
            }
            else if (!std::filesystem::is_regular_file(input_file)) {
                return std::nullopt;
            }
            std::string line;
            cached_input input {y, d};
            std::ifstream in {input_file};
            while (std::getline(in, line)) {
                if (!input.input.empty()) {
                    input.input.push_back('\n');
                }
                input.input.insert(input.input.end(), line.begin(), line.end());
                input.lines.push_back(std::move(line));
            }
            return input;
        }

        std::optional<std::pair<int, int>> get_year_day(const std::filesystem::path& p) {
            const auto parent = p.parent_path();
            const auto year = try_parse<int>(parent.filename().string());
            const auto day_str = p.filename().string();
            std::string_view day_sv {day_str};
            day_sv.remove_prefix(4);
            const auto day = try_parse<int>(day_sv);
            if (year && day) {
                return std::pair{*year, *day};
            }
            else {
                return std::nullopt;
            }
        }

        std::vector<cached_input> cache_inputs(std::optional<int> y, std::optional<int> d, const std::filesystem::path& input_dir) {
            std::vector<cached_input> retval;
            if (!y) {
                for (const auto& ent : std::filesystem::recursive_directory_iterator{input_dir}) {
                    if (ent.exists() && ent.is_regular_file()) {
                        const auto yd = get_year_day(ent.path());
                        if (yd) {
                            auto res = cache_input(yd->first, yd->second, input_dir);
                            if (res) {
                                retval.push_back(std::move(*res));
                            }
                        }

                    }
                }
            }
            else if (y && !d) {
                for (const auto& ent : std::filesystem::directory_iterator{input_dir / std::to_string(*y)}) {
                    if (ent.exists() && ent.is_regular_file()) {
                        const auto yd = get_year_day(ent.path());
                        if (yd) {
                            auto res = cache_input(yd->first, yd->second, input_dir);
                            if (res) {
                                retval.push_back(std::move(*res));
                            }
                        }
                    }
                }
            }
            else {
                auto res = cache_input(*y, *d, input_dir);
                if (res) {
                    retval.push_back(std::move(*res));
                }
            }
            std::sort(retval.begin(), retval.end());
            return retval;
        }

    }

    static auto operator<=>(const challenge &a, const challenge &b) { return std::tie(a.year, a.day, a.num) <=> std::tie(b.year, b.day, b.num); }
    static auto operator<=>(const challenge &a, const std::tuple<int, int, int> &b) { return std::tie(a.year, a.day, a.num) <=> b; }
    static auto operator<=>(const challenge &a, const std::tuple<int, int> &b) { return std::tie(a.year, a.day) <=> b; }
    static auto operator<=>(const challenge &a, const int b) { return a.year <=> b; }

    static bool operator==(const challenge &a, const challenge &b) { return std::tie(a.year, a.day, a.num) == std::tie(b.year, b.day, b.num); }
    static bool operator==(const challenge &a, const std::tuple<int, int, int> &b) { return std::tie(a.year, a.day, a.num) == b; }
    static bool operator==(const challenge &a, const std::tuple<int, int> &b) { return std::tie(a.year, a.day) == b; }
    static bool operator==(const challenge &a, const int b) { return a.year == b; }

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
        const auto inputs = cache_inputs(y, d, input_dir);
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
        std::for_each(begin, end, [&inputs, use_markdown_output_fmt, hide_answers](const challenge& c) noexcept { run_challenge(inputs, c, use_markdown_output_fmt, hide_answers); });
        auto dur = std::chrono::system_clock::now() - start;
        if (use_markdown_output_fmt) {
            std::cout << "#### ";
        }
        std::cout << "Finished solutions in:  " << time_to_string(dur) << '\n';
    }

} /* namespace aoc */