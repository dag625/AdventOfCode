//
// Created by Dan on 12/23/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <utility>
#include <vector>
#include <array>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2024/day/23
     */

    struct triple {
        std::array<std::string, 3> points{};

        triple(std::string va, std::string vb, std::string vc) : points{std::move(va), std::move(vb), std::move(vc)} { std::sort(points.begin(), points.end()); }

        [[nodiscard]] bool any_starts_with(const char c) const { return std::any_of(points.begin(), points.end(),
                                                                                    [c](const std::string& s){ return s.starts_with(c); }); }

        auto operator<=>(const triple& rhs) const { return points <=> rhs.points; }
        bool operator==(const triple& rhs) const { return points == rhs.points; }
    };

    struct edge {
        std::string a;
        std::string b;

        edge(std::string_view va, std::string_view vb) : a{va < vb ? va : vb}, b{va < vb ? vb : va} {}

        [[nodiscard]] bool any_starts_with(const char c) const { return a.starts_with(c) || b.starts_with(c); }

        auto operator<=>(const edge& rhs) const { return std::tie(a, b) <=> std::tie(rhs.a, rhs.b); }
        bool operator==(const edge& rhs) const { return std::tie(a, b) == std::tie(rhs.a, rhs.b); }

        bool operator==(const std::string& rhs) const { return a == rhs || b == rhs; }

        [[nodiscard]] std::optional<std::pair<edge, triple>> get_third(const edge& e) const {
            if (*this == e) {
                return std::nullopt;
            }
            else if (a == e.a) {
                return std::pair<edge, triple>{{b, e.b}, {a, b, e.b}};
            }
            else if (a == e.b) {
                return std::pair<edge, triple>{{b, e.a}, {a, b, e.a}};
            }
            else if (b == e.a) {
                return std::pair<edge, triple>{{a, e.b}, {b, a, e.b}};
            }
            else if (b == e.b) {
                return std::pair<edge, triple>{{a, e.a}, {b, a, e.a}};
            }
            else {
                return std::nullopt;
            }
        }
    };

    edge parse_edge(std::string_view s) {
        const auto parts = split(s, '-');
        return {parts[0], parts[1]};
    }

    std::vector<edge> get_input(const std::vector<std::string>& lines) {
        auto retval = lines | std::views::transform(&parse_edge) | std::ranges::to<std::vector>();
        std::sort(retval.begin(), retval.end());
        return retval;
    }

    int count_triples_with_t(const std::vector<edge>& edges) {
        std::vector<triple> retval;
        for (int i = 0; i < edges.size(); ++i) {
            for (int j = i + 1; j < edges.size(); ++j) {
                const auto third = edges[i].get_third(edges[j]);
                if (third) {
                    const auto found = std::lower_bound(edges.begin(), edges.end(), third->first);
                    if (found != edges.end() && *found == third->first && third->second.any_starts_with('t')) {
                        const auto tfound = std::lower_bound(retval.begin(), retval.end(), third->second);
                        if (tfound == retval.end() || *tfound != third->second) {
                            retval.insert(tfound, third->second);
                        }
                    }
                }
            }
        }
        return static_cast<int>(retval.size());
    }

    std::vector<std::string> get_nodes(const std::vector<edge>& edges) {
        std::vector<std::string> retval;
        for (const auto& e : edges) {
            retval.push_back(e.a);
            retval.push_back(e.b);
        }
        std::sort(retval.begin(), retval.end());
        retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
        return retval;
    }

    bool have_edge(const std::string& a, const std::string& b, const std::vector<edge>& edges) {
        edge e {a, b};
        const auto found = std::lower_bound(edges.begin(), edges.end(), e);
        return found != edges.end() && *found == e;
    }

    std::vector<std::string> find_group(const std::vector<std::string>& nodes, const std::vector<edge>& edges) {
        std::vector<std::string> retval;
        if (!nodes.empty()) {
            retval.push_back(nodes.front());
            for (int i = 1; i < nodes.size(); ++i) {
                if (std::all_of(retval.begin(), retval.end(), [&a = nodes[i], &edges](const std::string& b){ return have_edge(a, b, edges); })) {
                    retval.push_back(nodes[i]);
                }
            }
        }
        return retval;
    }

    std::vector<std::string> find_largest(const std::vector<edge>& edges) {
        auto nodes = get_nodes(edges);
        std::vector<std::string> retval;
        while (!nodes.empty()) {
            /*
             * This works, but assumes no nodes are members of two of these maximally connected groups.  So a
             * bit of happy accident or kind input(s) perhaps.  And somehow my part 2 is faster than my part
             * 1 which feels odd, or maybe my part 1 is terrible.
             */
            auto group = find_group(nodes, edges);
            std::vector<std::string> new_nodes;
            std::set_difference(nodes.begin(), nodes.end(), group.begin(), group.end(), std::back_inserter(new_nodes));
            nodes.swap(new_nodes);
            if (group.size() > retval.size()) {
                retval.swap(group);
            }
        }
        return retval;
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto res = count_triples_with_t(input);
        return std::to_string(res);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        const auto group = find_largest(input);
        return fmt::format("{}", fmt::join(group, ","));
    }

    aoc::registration r{2024, 23, part_1, part_2};

    TEST_SUITE("2024_day23") {
        TEST_CASE("2024_day23:example") {
            const std::vector<std::string> lines {
                    "kh-tc",
                    "qp-kh",
                    "de-cg",
                    "ka-co",
                    "yn-aq",
                    "qp-ub",
                    "cg-tb",
                    "vc-aq",
                    "tb-ka",
                    "wh-tc",
                    "yn-cg",
                    "kh-ub",
                    "ta-co",
                    "de-co",
                    "tc-td",
                    "tb-wq",
                    "wh-td",
                    "ta-ka",
                    "td-qp",
                    "aq-cg",
                    "wq-ub",
                    "ub-vc",
                    "de-ta",
                    "wq-aq",
                    "wq-vc",
                    "wh-yn",
                    "ka-de",
                    "kh-ta",
                    "co-tc",
                    "wh-qp",
                    "tb-vc",
                    "td-yn"
            };
            auto input = lines | std::views::transform(&parse_edge) | std::ranges::to<std::vector>();
            std::sort(input.begin(), input.end());
            const auto res = count_triples_with_t(input);
            CHECK_EQ(res, 7);
        }
    }

} /* namespace <anon> */