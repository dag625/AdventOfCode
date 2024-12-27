//
// Created by Daniel Garcia on 6/7/21.
//

#include "registration.h"
#include "utilities.h"

#include <doctest/doctest.h>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    std::pair<int, int> get_lengths(const std::string_view s) {
        auto contents = s;
        contents.remove_prefix(1);
        contents.remove_suffix(1);
        int sub = 0;
        for (auto idx = 0u; idx < contents.size(); ++idx) {
            if (contents[idx] == '\\') {
                ++idx;
                if (contents[idx] == '\\' || contents[idx] == '"') {
                    ++sub;
                }
                else if (contents[idx] == 'x') {
                    sub += 3;
                    idx += 2;
                }
            }
        }
        return {static_cast<int>(s.size()), static_cast<int>(contents.size() - sub)};
    }

    std::pair<int, int> get_expanded_lengths(const std::string_view s) {
        int add = 2;
        for (char c : s) {
            if (c == '\"' || c == '\\') {
                ++add;
            }
        }
        return {static_cast<int>(s.size()), static_cast<int>(s.size() + add)};
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        int sum = 0;
        for (const auto& s : lines) {
            auto [total, display] = get_lengths(s);
            sum += (total - display);
        }
        return std::to_string(sum);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        int sum = 0;
        for (const auto& s : lines) {
            auto [orig, display] = get_expanded_lengths(s);
            sum += (display - orig);
        }
        return std::to_string(sum);
    }

    aoc::registration r {2015, 8, part_1, part_2};

    TEST_SUITE("2015_day8") {
        TEST_CASE ("2015_day8:example") {
            auto [t1, d1] = get_lengths("\"\"");
            REQUIRE_EQ(t1, 2);
            REQUIRE_EQ(d1, 0);

            auto [t2, d2] = get_lengths("\"abc\"");
            REQUIRE_EQ(t2, 5);
            REQUIRE_EQ(d2, 3);

            auto [t3, d3] = get_lengths("\"aaa\\\"aaa\"");
            REQUIRE_EQ(t3, 10);
            REQUIRE_EQ(d3, 7);

            auto [t4, d4] = get_lengths("\"\\x27\"");
            REQUIRE_EQ(t4, 6);
            REQUIRE_EQ(d4, 1);
        }
    }

}

