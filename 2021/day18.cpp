//
// Created by Dan on 12/17/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <variant>
#include <memory>
#include <charconv>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct pair;

    using value = std::variant<int, std::unique_ptr<pair>>;

    value copy_value(const value& val);

    struct reduce_result {
        enum result_enum {
            clean,
            dirty,
            split,
            explode
        };
        result_enum result = clean;
        std::optional<int> left = std::nullopt;
        std::optional<int> right = std::nullopt;

        reduce_result() = default;
        reduce_result(result_enum res) : result{res} {}
        reduce_result(result_enum res, std::optional<int> lv, std::optional<int> rv) : result{res}, left{lv}, right{rv} {}
    };

    struct exploder {
        int depth;
        reduce_result operator()(int v) { return {}; }
        reduce_result operator()(std::unique_ptr<pair>& v) const;
    };

    struct splitter {
        int depth;
        reduce_result operator()(int v);
        reduce_result operator()(std::unique_ptr<pair>& v) const;
    };

    struct magnitude_calculator {
        int64_t operator()(int v) { return v; }
        int64_t operator()(const std::unique_ptr<pair>& v);
    };

    struct printer {
        std::string operator()(int v) { return std::to_string(v); }
        std::string operator()(const std::unique_ptr<pair>& v);
    };

    class pair {
        value left;
        value right;

        void add_to_left(int val, bool first) {
            std::visit([val, first](auto& v){
                if constexpr (std::is_same_v<decltype(v), int&>) {
                    v += val;
                }
                else {
                    if (first) {
                        v->add_to_right(val, false);
                    }
                    else {
                        v->add_to_left(val, false);
                    }
                }
            }, left);
        }

        void add_to_right(int val, bool first) {
            std::visit([val, first](auto& v){
                if constexpr (std::is_same_v<decltype(v), int&>) {
                    v += val;
                }
                else {
                    if (first) {
                        v->add_to_left(val, false);
                    }
                    else {
                        v->add_to_right(val, false);
                    }
                }
            }, right);
        }

        friend struct exploder;
        friend struct splitter;
        friend struct magnitude_calculator;

        reduce_result explode(int depth) {
            auto lres = std::visit(exploder{depth}, left);
            if (lres.result == reduce_result::explode) {
                if (lres.left && lres.right) {
                    left = 0;
                    add_to_right(*lres.right, true);
                    lres.right = std::nullopt;
                    return lres;
                }
                else if (lres.left) {
                    return lres;
                }
                else if (lres.right) {
                    add_to_right(*lres.right, true);
                }
                return {reduce_result::dirty};
            }
            else if (lres.result == reduce_result::dirty) {
                return lres;
            }

            auto rres = std::visit(exploder{depth}, right);
            if (rres.result == reduce_result::explode) {
                if (rres.left && rres.right) {
                    right = 0;
                    add_to_left(*rres.left, true);
                    rres.left = std::nullopt;
                    return rres;
                }
                else if (rres.right) {
                    return rres;
                }
                else if (rres.left) {
                    add_to_left(*rres.left, true);
                }
                return {reduce_result::dirty};
            }
            return rres;
        }

        reduce_result split(int depth) {
            auto lres = std::visit(splitter{depth}, left);
            if (lres.result == reduce_result::split) {
                left = std::make_unique<pair>(*lres.left, *lres.right);
                return {reduce_result::dirty};
            }
            else if (lres.result == reduce_result::dirty) {
                return lres;
            }
            auto rres = std::visit(splitter{depth}, right);
            if (rres.result == reduce_result::split) {
                right = std::make_unique<pair>(*rres.left, *rres.right);
                return {reduce_result::dirty};
            }
            return rres;
        }

    public:
        pair() = default;
        pair(const pair& other) : left{copy_value(other.left)}, right{copy_value(other.right)} {}
        pair(pair&&) = default;
        pair(value l, value r) : left{std::move(l)}, right{std::move(r)} {}

        pair& operator=(pair rhs) noexcept {
            using std::swap;
            swap(left, rhs.left);
            swap(right, rhs.right);
            return *this;
        }

        pair& operator+=(const pair& rhs) {
            left = std::make_unique<pair>(*this);
            right = std::make_unique<pair>(rhs);
            reduce();
            return *this;
        }

        void reduce() {
            bool need_reduce = true;
            while (need_reduce) {
                const auto eres = explode(0);
                need_reduce = eres.result != reduce_result::clean;
                if (need_reduce) {
                    continue;
                }

                const auto sres = split(0);
                need_reduce = sres.result != reduce_result::clean;
            }
        }

        [[nodiscard]] int64_t magnitude() const {
            return 3 * std::visit(magnitude_calculator{}, left) + 2 * std::visit(magnitude_calculator{}, right);
        }

        [[nodiscard]] std::string print() const {
            return fmt::format("[{},{}]",
                               std::visit(printer{}, left),
                               std::visit(printer{}, right));
        }
    };

    reduce_result exploder::operator()(std::unique_ptr<pair>& v) const {
        if (depth >= 3) {
            return {reduce_result::explode, std::get<int>(v->left), std::get<int>(v->right)};
        }
        else {
            return v->explode(depth + 1);
        }
    }

    reduce_result splitter::operator()(int v) {
        if (v >= 10) {
            const auto h = v / 2;
            const auto r = v % 2;
            return {reduce_result::split, h, h + r};
        }
        else {
            return {};
        }
    }

    reduce_result splitter::operator()(std::unique_ptr<pair>& v) const {
        return v->split(depth + 1);
    }

    int64_t magnitude_calculator::operator()(const std::unique_ptr<pair>& v) {
        return v->magnitude();
    }

    std::string printer::operator()(const std::unique_ptr<pair>& v) {
        return v->print();
    }


    struct value_copier {
        value operator()(int v) { return v; }
        value operator()(const std::unique_ptr<pair>& v) { return std::make_unique<pair>(*v); }
    };

    value copy_value(const value& val) {
        return std::visit(value_copier{}, val);
    }

    pair operator+(const pair& a, const pair& b) {
        pair retval {std::make_unique<pair>(a), std::make_unique<pair>(b)};
        retval.reduce();
        return retval;
    }

    std::pair<std::string_view, std::string_view> split_pair(std::string_view s) {
        s = s.substr(1, s.size() - 2);
        int depth = 0, mid = 0;
        for (int idx = 0; idx < s.size(); ++idx) {
            if (s[idx] == '[') {
                ++depth;
            }
            else if (s[idx] == ']') {
                --depth;
            }
            else if (s[idx] == ',' && depth == 0) {
                mid = idx;
                break;
            }
        }
        return {s.substr(0, mid), s.substr(mid + 1)};
    }

    pair parse_pair(std::string_view s);

    value parse_value(std::string_view s) {
        if (s.starts_with('[')) {
            return std::make_unique<pair>(parse_pair(s));
        }
        else {
            int val = 0;
            const auto res = std::from_chars(s.data(), s.data() + s.size(), val);
            if (const auto ec = std::make_error_code(res.ec); ec) {
                throw std::system_error(ec);
            }
            return val;
        }
    }

    pair parse_pair(std::string_view s) {
        const auto [left, right] = split_pair(s);
        return {parse_value(left), parse_value(right)};
    }

    std::vector<pair> get_input(const std::vector<std::string>& lines) {
        return lines |
            std::views::transform([](std::string_view s){ return parse_pair(s); }) |
            to<std::vector<pair>>();
    }

    /*
    --- Day 18: Snailfish ---
    You descend into the ocean trench and encounter some snailfish. They say they saw the sleigh keys! They'll even tell you which direction the keys went if you help one of the smaller snailfish with his math homework.

    Snailfish numbers aren't like regular numbers. Instead, every snailfish number is a pair - an ordered list of two elements. Each element of the pair can be either a regular number or another pair.

    Pairs are written as [x,y], where x and y are the elements within the pair. Here are some example snailfish numbers, one snailfish number per line:

    [1,2]
    [[1,2],3]
    [9,[8,7]]
    [[1,9],[8,5]]
    [[[[1,2],[3,4]],[[5,6],[7,8]]],9]
    [[[9,[3,8]],[[0,9],6]],[[[3,7],[4,9]],3]]
    [[[[1,3],[5,3]],[[1,3],[8,7]]],[[[4,9],[6,9]],[[8,2],[7,3]]]]
    This snailfish homework is about addition. To add two snailfish numbers, form a pair from the left and right parameters of the addition operator. For example, [1,2] + [[3,4],5] becomes [[1,2],[[3,4],5]].

    There's only one problem: snailfish numbers must always be reduced, and the process of adding two snailfish numbers can result in snailfish numbers that need to be reduced.

    To reduce a snailfish number, you must repeatedly do the first action in this list that applies to the snailfish number:

    If any pair is nested inside four pairs, the leftmost such pair explodes.
    If any regular number is 10 or greater, the leftmost such regular number splits.
    Once no action in the above list applies, the snailfish number is reduced.

    During reduction, at most one action applies, after which the process returns to the top of the list of actions. For example, if split produces a pair that meets the explode criteria, that pair explodes before other splits occur.

    To explode a pair, the pair's left value is added to the first regular number to the left of the exploding pair (if any), and the pair's right value is added to the first regular number to the right of the exploding pair (if any). Exploding pairs will always consist of two regular numbers. Then, the entire exploding pair is replaced with the regular number 0.

    Here are some examples of a single explode action:

    [[[[[9,8],1],2],3],4] becomes [[[[0,9],2],3],4] (the 9 has no regular number to its left, so it is not added to any regular number).
    [7,[6,[5,[4,[3,2]]]]] becomes [7,[6,[5,[7,0]]]] (the 2 has no regular number to its right, and so it is not added to any regular number).
    [[6,[5,[4,[3,2]]]],1] becomes [[6,[5,[7,0]]],3].
    [[3,[2,[1,[7,3]]]],[6,[5,[4,[3,2]]]]] becomes [[3,[2,[8,0]]],[9,[5,[4,[3,2]]]]] (the pair [3,2] is unaffected because the pair [7,3] is further to the left; [3,2] would explode on the next action).
    [[3,[2,[8,0]]],[9,[5,[4,[3,2]]]]] becomes [[3,[2,[8,0]]],[9,[5,[7,0]]]].
    To split a regular number, replace it with a pair; the left element of the pair should be the regular number divided by two and rounded down, while the right element of the pair should be the regular number divided by two and rounded up. For example, 10 becomes [5,5], 11 becomes [5,6], 12 becomes [6,6], and so on.

    Here is the process of finding the reduced result of [[[[4,3],4],4],[7,[[8,4],9]]] + [1,1]:

    after addition: [[[[[4,3],4],4],[7,[[8,4],9]]],[1,1]]
    after explode:  [[[[0,7],4],[7,[[8,4],9]]],[1,1]]
    after explode:  [[[[0,7],4],[15,[0,13]]],[1,1]]
    after split:    [[[[0,7],4],[[7,8],[0,13]]],[1,1]]
    after split:    [[[[0,7],4],[[7,8],[0,[6,7]]]],[1,1]]
    after explode:  [[[[0,7],4],[[7,8],[6,0]]],[8,1]]
    Once no reduce actions apply, the snailfish number that remains is the actual result of the addition operation: [[[[0,7],4],[[7,8],[6,0]]],[8,1]].

    The homework assignment involves adding up a list of snailfish numbers (your puzzle input). The snailfish numbers are each listed on a separate line. Add the first snailfish number and the second, then add that result and the third, then add that result and the fourth, and so on until all numbers in the list have been used once.

    For example, the final sum of this list is [[[[1,1],[2,2]],[3,3]],[4,4]]:

    [1,1]
    [2,2]
    [3,3]
    [4,4]
    The final sum of this list is [[[[3,0],[5,3]],[4,4]],[5,5]]:

    [1,1]
    [2,2]
    [3,3]
    [4,4]
    [5,5]
    The final sum of this list is [[[[5,0],[7,4]],[5,5]],[6,6]]:

    [1,1]
    [2,2]
    [3,3]
    [4,4]
    [5,5]
    [6,6]
    Here's a slightly larger example:

    [[[0,[4,5]],[0,0]],[[[4,5],[2,6]],[9,5]]]
    [7,[[[3,7],[4,3]],[[6,3],[8,8]]]]
    [[2,[[0,8],[3,4]]],[[[6,7],1],[7,[1,6]]]]
    [[[[2,4],7],[6,[0,5]]],[[[6,8],[2,8]],[[2,1],[4,5]]]]
    [7,[5,[[3,8],[1,4]]]]
    [[2,[2,2]],[8,[8,1]]]
    [2,9]
    [1,[[[9,3],9],[[9,0],[0,7]]]]
    [[[5,[7,4]],7],1]
    [[[[4,2],2],6],[8,7]]
    The final sum [[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]] is found after adding up the above snailfish numbers:

      [[[0,[4,5]],[0,0]],[[[4,5],[2,6]],[9,5]]]
    + [7,[[[3,7],[4,3]],[[6,3],[8,8]]]]
    = [[[[4,0],[5,4]],[[7,7],[6,0]]],[[8,[7,7]],[[7,9],[5,0]]]]

      [[[[4,0],[5,4]],[[7,7],[6,0]]],[[8,[7,7]],[[7,9],[5,0]]]]
    + [[2,[[0,8],[3,4]]],[[[6,7],1],[7,[1,6]]]]
    = [[[[6,7],[6,7]],[[7,7],[0,7]]],[[[8,7],[7,7]],[[8,8],[8,0]]]]

      [[[[6,7],[6,7]],[[7,7],[0,7]]],[[[8,7],[7,7]],[[8,8],[8,0]]]]
    + [[[[2,4],7],[6,[0,5]]],[[[6,8],[2,8]],[[2,1],[4,5]]]]
    = [[[[7,0],[7,7]],[[7,7],[7,8]]],[[[7,7],[8,8]],[[7,7],[8,7]]]]

      [[[[7,0],[7,7]],[[7,7],[7,8]]],[[[7,7],[8,8]],[[7,7],[8,7]]]]
    + [7,[5,[[3,8],[1,4]]]]
    = [[[[7,7],[7,8]],[[9,5],[8,7]]],[[[6,8],[0,8]],[[9,9],[9,0]]]]

      [[[[7,7],[7,8]],[[9,5],[8,7]]],[[[6,8],[0,8]],[[9,9],[9,0]]]]
    + [[2,[2,2]],[8,[8,1]]]
    = [[[[6,6],[6,6]],[[6,0],[6,7]]],[[[7,7],[8,9]],[8,[8,1]]]]

      [[[[6,6],[6,6]],[[6,0],[6,7]]],[[[7,7],[8,9]],[8,[8,1]]]]
    + [2,9]
    = [[[[6,6],[7,7]],[[0,7],[7,7]]],[[[5,5],[5,6]],9]]

      [[[[6,6],[7,7]],[[0,7],[7,7]]],[[[5,5],[5,6]],9]]
    + [1,[[[9,3],9],[[9,0],[0,7]]]]
    = [[[[7,8],[6,7]],[[6,8],[0,8]]],[[[7,7],[5,0]],[[5,5],[5,6]]]]

      [[[[7,8],[6,7]],[[6,8],[0,8]]],[[[7,7],[5,0]],[[5,5],[5,6]]]]
    + [[[5,[7,4]],7],1]
    = [[[[7,7],[7,7]],[[8,7],[8,7]]],[[[7,0],[7,7]],9]]

      [[[[7,7],[7,7]],[[8,7],[8,7]]],[[[7,0],[7,7]],9]]
    + [[[[4,2],2],6],[8,7]]
    = [[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]]
    To check whether it's the right answer, the snailfish teacher only checks the magnitude of the final sum. The magnitude of a pair is 3 times the magnitude of its left element plus 2 times the magnitude of its right element. The magnitude of a regular number is just that number.

    For example, the magnitude of [9,1] is 3*9 + 2*1 = 29; the magnitude of [1,9] is 3*1 + 2*9 = 21. Magnitude calculations are recursive: the magnitude of [[9,1],[1,9]] is 3*29 + 2*21 = 129.

    Here are a few more magnitude examples:

    [[1,2],[[3,4],5]] becomes 143.
    [[[[0,7],4],[[7,8],[6,0]]],[8,1]] becomes 1384.
    [[[[1,1],[2,2]],[3,3]],[4,4]] becomes 445.
    [[[[3,0],[5,3]],[4,4]],[5,5]] becomes 791.
    [[[[5,0],[7,4]],[5,5]],[6,6]] becomes 1137.
    [[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]] becomes 3488.
    So, given this example homework assignment:

    [[[0,[5,8]],[[1,7],[9,6]]],[[4,[1,2]],[[1,4],2]]]
    [[[5,[2,8]],4],[5,[[9,9],0]]]
    [6,[[[6,2],[5,6]],[[7,6],[4,7]]]]
    [[[6,[0,7]],[0,9]],[4,[9,[9,0]]]]
    [[[7,[6,4]],[3,[1,3]]],[[[5,5],1],9]]
    [[6,[[7,3],[3,2]]],[[[3,8],[5,7]],4]]
    [[[[5,4],[7,7]],8],[[8,3],8]]
    [[9,3],[[9,9],[6,[4,9]]]]
    [[2,[[7,7],7]],[[5,8],[[9,3],[0,2]]]]
    [[[[5,2],5],[8,[3,7]]],[[5,[7,5]],[4,4]]]
    The final sum is:

    [[[[6,6],[7,6]],[[7,7],[7,0]]],[[[7,7],[7,7]],[[7,8],[9,9]]]]
    The magnitude of this final sum is 4140.

    Add up all of the snailfish numbers from the homework assignment in the order they appear. What is the magnitude of the final sum?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        pair sum = input.front();
        for (const pair& p : input | std::views::drop(1)) {
            sum += p;
        }
        return std::to_string(sum.magnitude());
    }

    /*
    --- Part Two ---
    You notice a second question on the back of the homework assignment:

    What is the largest magnitude you can get from adding only two of the snailfish numbers?

    Note that snailfish addition is not commutative - that is, x + y and y + x can produce different results.

    Again considering the last example homework assignment above:

    [[[0,[5,8]],[[1,7],[9,6]]],[[4,[1,2]],[[1,4],2]]]
    [[[5,[2,8]],4],[5,[[9,9],0]]]
    [6,[[[6,2],[5,6]],[[7,6],[4,7]]]]
    [[[6,[0,7]],[0,9]],[4,[9,[9,0]]]]
    [[[7,[6,4]],[3,[1,3]]],[[[5,5],1],9]]
    [[6,[[7,3],[3,2]]],[[[3,8],[5,7]],4]]
    [[[[5,4],[7,7]],8],[[8,3],8]]
    [[9,3],[[9,9],[6,[4,9]]]]
    [[2,[[7,7],7]],[[5,8],[[9,3],[0,2]]]]
    [[[[5,2],5],[8,[3,7]]],[[5,[7,5]],[4,4]]]
    The largest magnitude of the sum of any two snailfish numbers in this list is 3993. This is the magnitude of [[2,[[7,7],7]],[[5,8],[[9,3],[0,2]]]] + [[[0,[5,8]],[[1,7],[9,6]]],[[4,[1,2]],[[1,4],2]]], which reduces to [[[[7,8],[6,6]],[[6,0],[7,7]]],[[[7,8],[8,8]],[[7,9],[0,6]]]].

    What is the largest magnitude of any sum of two different snailfish numbers from the homework assignment?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        int64_t max = -1;
        for (int i = 0; i < input.size(); ++i) {
            for (int j = i + 1; j < input.size(); ++j) {
                const auto ij = (input[i] + input[j]).magnitude();
                const auto ji = (input[j] + input[i]).magnitude();
                max = std::max(std::max(ij, ji), max);
            }
        }
        return std::to_string(max);
    }

    aoc::registration r {2021, 18, part_1, part_2};

    TEST_SUITE("2021_day18") {
        TEST_CASE("2021_day18:example_1") {
            using namespace std::string_view_literals;
            const auto a = parse_pair("[[[[4,3],4],4],[7,[[8,4],9]]]"sv);
            const auto b = parse_pair("[1,1]"sv);
            const auto c = a + b;
            REQUIRE_EQ(c.print(), "[[[[0,7],4],[[7,8],[6,0]]],[8,1]]"sv);
        }

        TEST_CASE("2021_day18:example2") {
            using namespace std::string_view_literals;
            const std::vector<std::string> lines = {
                    "[[[0,[4,5]],[0,0]],[[[4,5],[2,6]],[9,5]]]",
                    "[7,[[[3,7],[4,3]],[[6,3],[8,8]]]]",
                    "[[2,[[0,8],[3,4]]],[[[6,7],1],[7,[1,6]]]]",
                    "[[[[2,4],7],[6,[0,5]]],[[[6,8],[2,8]],[[2,1],[4,5]]]]",
                    "[7,[5,[[3,8],[1,4]]]]",
                    "[[2,[2,2]],[8,[8,1]]]",
                    "[2,9]",
                    "[1,[[[9,3],9],[[9,0],[0,7]]]]",
                    "[[[5,[7,4]],7],1]",
                    "[[[[4,2],2],6],[8,7]]"
            };
            const auto input = lines |
                               std::views::transform([](std::string_view s){ return parse_pair(s); }) |
                               to<std::vector<pair>>();
            pair sum = input.front();

            sum += input[1];
            REQUIRE_EQ(sum.print(), "[[[[4,0],[5,4]],[[7,7],[6,0]]],[[8,[7,7]],[[7,9],[5,0]]]]"sv);

            sum += input[2];
            REQUIRE_EQ(sum.print(), "[[[[6,7],[6,7]],[[7,7],[0,7]]],[[[8,7],[7,7]],[[8,8],[8,0]]]]"sv);

            sum += input[3];
            REQUIRE_EQ(sum.print(), "[[[[7,0],[7,7]],[[7,7],[7,8]]],[[[7,7],[8,8]],[[7,7],[8,7]]]]"sv);

            sum += input[4];
            REQUIRE_EQ(sum.print(), "[[[[7,7],[7,8]],[[9,5],[8,7]]],[[[6,8],[0,8]],[[9,9],[9,0]]]]"sv);

            sum += input[5];
            REQUIRE_EQ(sum.print(), "[[[[6,6],[6,6]],[[6,0],[6,7]]],[[[7,7],[8,9]],[8,[8,1]]]]"sv);

            sum += input[6];
            REQUIRE_EQ(sum.print(), "[[[[6,6],[7,7]],[[0,7],[7,7]]],[[[5,5],[5,6]],9]]"sv);

            sum += input[7];
            REQUIRE_EQ(sum.print(), "[[[[7,8],[6,7]],[[6,8],[0,8]]],[[[7,7],[5,0]],[[5,5],[5,6]]]]"sv);

            sum += input[8];
            REQUIRE_EQ(sum.print(), "[[[[7,7],[7,7]],[[8,7],[8,7]]],[[[7,0],[7,7]],9]]"sv);

            sum += input[9];
            REQUIRE_EQ(sum.print(), "[[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]]"sv);
        }

        TEST_CASE("2021_day18:last_example") {
            using namespace std::string_view_literals;
            const auto expected = parse_pair("[[[[6,6],[7,6]],[[7,7],[7,0]]],[[[7,7],[7,7]],[[7,8],[9,9]]]]"sv);
            REQUIRE_EQ(expected.magnitude(), 4140);

            const std::vector<std::string> lines = {
                    "[[[0,[5,8]],[[1,7],[9,6]]],[[4,[1,2]],[[1,4],2]]]",
                    "[[[5,[2,8]],4],[5,[[9,9],0]]]",
                    "[6,[[[6,2],[5,6]],[[7,6],[4,7]]]]",
                    "[[[6,[0,7]],[0,9]],[4,[9,[9,0]]]]",
                    "[[[7,[6,4]],[3,[1,3]]],[[[5,5],1],9]]",
                    "[[6,[[7,3],[3,2]]],[[[3,8],[5,7]],4]]",
                    "[[[[5,4],[7,7]],8],[[8,3],8]]",
                    "[[9,3],[[9,9],[6,[4,9]]]]",
                    "[[2,[[7,7],7]],[[5,8],[[9,3],[0,2]]]]",
                    "[[[[5,2],5],[8,[3,7]]],[[5,[7,5]],[4,4]]]"
            };
            const auto input = lines |
                               std::views::transform([](std::string_view s){ return parse_pair(s); }) |
                               to<std::vector<pair>>();
            pair sum = input.front();
            fmt::print("\n");
            for (const pair& p : input | std::views::drop(1)) {
                sum += p;
                fmt::print("Step:  {}\n", sum.print());
            }
            fmt::print("Out:   {}\n", sum.print());
            REQUIRE_EQ(sum.print(), "[[[[6,6],[7,6]],[[7,7],[7,0]]],[[[7,7],[7,7]],[[7,8],[9,9]]]]"sv);
            REQUIRE_EQ(sum.magnitude(), 4140);
        }
    }

}