//
// Created by Daniel Garcia on 12/18/20.
//

#include "day18.h"
#include "utilities.h"

#include <doctest/doctest.h>

#include <variant>
#include <vector>
#include <string_view>
#include <charconv>
#include <memory>
#include <numeric>
#include <iostream>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        struct add {};

        struct mul {};

        struct group {};

        struct node {
            using ptr = std::unique_ptr<node>;
            using value = std::variant<int64_t, add, mul, group>;
            value val;
            ptr left;
            ptr right;

            node() = default;
            explicit node(value v) : val{v}, left{}, right{} {}
            node(group g, ptr p) : val{g}, left{}, right{std::move(p)} {}
        };

        struct print_visitor {
            std::pair<std::string, bool> operator()(int64_t v) const { return {std::to_string(v), false}; }
            std::pair<std::string, bool> operator()(mul) const { return {"*", true}; }
            std::pair<std::string, bool> operator()(add) const { return {"+", true}; }
            std::pair<std::string, bool> operator()(group) const { return {"=", true}; }
        };

        std::ostream& operator<<(std::ostream& os, const node& n) {
            auto [str, next_level] = std::visit(print_visitor{}, n.val);
            if (next_level) {
                os << '(' << str << ' ';
                if (n.left) {
                    os << *n.left;
                }
                os << ' ';
                if (n.right) {
                    os << *n.right;
                }
                os << ')';
            }
            else {
                os << str;
            }
            return os;
        }

        std::string to_string(const node& n) {
            std::stringstream ns;
            ns << n;
            return ns.str();
        }

        std::pair<node::ptr, std::size_t> parse_group(std::string_view stmt);

        node::ptr parse(std::string_view stmt) {
            node::ptr retval{};
            for (std::size_t i = 0; i < stmt.size(); ++i) {
                if (isspace(stmt[i])) {
                    continue;
                }
                else if (isdigit(stmt[i])) {
                    int64_t val = 0;
                    auto res = std::from_chars(stmt.data() + i, stmt.data() + stmt.size(), val);
                    if (res.ec != std::errc{}) {
                        throw std::runtime_error{"Failed to parse literal in expression."};
                    }
                    if (retval) {
                        retval->right = std::make_unique<node>(val);
                    }
                    else {
                        retval = std::make_unique<node>(val);
                    }
                    i += res.ptr - stmt.data() - i - 1;
                }
                else if (stmt[i] == '(') {
                    auto [g, len] = parse_group(stmt.substr(i + 1));
                    if (retval) {
                        retval->right = std::move(g);
                    }
                    else {
                        retval = std::move(g);
                    }
                    i += len;
                }
                else if (stmt[i] == '+') {
                    auto left = std::move(retval);
                    retval = std::make_unique<node>(add{});
                    retval->left = std::move(left);
                }
                else if (stmt[i] == '*') {
                    auto left = std::move(retval);
                    retval = std::make_unique<node>(mul{});
                    retval->left = std::move(left);
                }
                else {
                    throw std::runtime_error{"Invalid token."};
                }
            }
            //std::cout << "From [" << stmt << "] to [" << *retval << "]\n";
            return retval;
        }

        std::pair<node::ptr, std::size_t> parse_group(std::string_view stmt) {
            std::size_t pos_end = 0;
            int level = 1;
            for (; pos_end < stmt.size() && level > 0; ++pos_end) {
                if (stmt[pos_end] == '(') {
                    ++level;
                }
                else if (stmt[pos_end] == ')') {
                    --level;
                }
            }
            return { std::make_unique<node>(group{}, parse(stmt.substr(0, pos_end - 1))), pos_end };
        }

        std::vector<node::ptr> get_input(const fs::path &input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_18_input.txt");
            std::vector<node::ptr> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse);
            return retval;
        }

        struct no_precedence {
            int operator()(int64_t) const { return 2; }
            int operator()(mul) const { return 0; }
            int operator()(add) const { return 0; }
            int operator()(group) const { return 2; }
        };

        struct add_precedence {
            int operator()(int64_t) const { return 2; }
            int operator()(mul) const { return 0; }
            int operator()(add) const { return 1; }
            int operator()(group) const { return 2; }
        };

        template <typename Precedence>
        void balance(node::ptr& n) {
            if (!n) { return; }
            else if (std::holds_alternative<group>(n->val)) {
                balance<Precedence>(n->right);
                return;
            }
            else if (!n->left) { return; }
            balance<Precedence>(n->right);
            balance<Precedence>(n->left);
            auto own = std::visit(Precedence{}, n->val);
            auto next = std::visit(Precedence{}, n->left->val);
            if (next < own) {
                auto lr = std::move(n->left->right);
                auto l = std::move(n->left);
                n->left = std::move(lr);
                auto r = std::move(n);
                n = std::move(l);
                n->right = std::move(r);
            }
        }

        int64_t evaluate(const node::ptr& n);

        struct evaluate_visitor {
            const node* current;
            explicit evaluate_visitor(const node::ptr& n) : current{n.get()} {}

            int64_t operator()(int64_t v) const { return v; }
            int64_t operator()(mul) const { return evaluate(current->right) * evaluate(current->left); }
            int64_t operator()(add) const { return evaluate(current->right) + evaluate(current->left); }
            int64_t operator()(group) const { return evaluate(current->right); }
        };

        int64_t evaluate(const node::ptr& n) {
            if (!n) {
                throw std::runtime_error{"Cannot evaluate null node."};
            }
            return std::visit(evaluate_visitor{n}, n->val);
        }

    }

    /*
    As you look out the window and notice a heavily-forested continent slowly appear over the horizon, you are interrupted by the child sitting next to you. They're curious if you could help them with their math homework.

    Unfortunately, it seems like this "math" follows different rules than you remember.

    The homework (your puzzle input) consists of a series of expressions that consist of addition (+), multiplication (*), and parentheses ((...)). Just like normal math, parentheses indicate that the expression inside must be evaluated before it can be used by the surrounding expression. Addition still finds the sum of the numbers on both sides of the operator, and multiplication still finds the product.

    However, the rules of operator precedence have changed. Rather than evaluating multiplication before addition, the operators have the same precedence, and are evaluated left-to-right regardless of the order in which they appear.

    For example, the steps to evaluate the expression 1 + 2 * 3 + 4 * 5 + 6 are as follows:

    1 + 2 * 3 + 4 * 5 + 6
      3   * 3 + 4 * 5 + 6
          9   + 4 * 5 + 6
             13   * 5 + 6
                 65   + 6
                     71
    Parentheses can override this order; for example, here is what happens if parentheses are added to form 1 + (2 * 3) + (4 * (5 + 6)):

    1 + (2 * 3) + (4 * (5 + 6))
    1 +    6    + (4 * (5 + 6))
         7      + (4 * (5 + 6))
         7      + (4 *   11   )
         7      +     44
                51
    Here are a few more examples:

    2 * 3 + (4 * 5) becomes 26.
    5 + (8 * 3 + 9 + 3 * 4 * 3) becomes 437.
    5 * 9 * (7 * 3 * 3 + 9 * 3 + (8 + 6 * 4)) becomes 12240.
    ((2 + 4 * 9) * (6 + 9 * 8 + 6) + 6) + 2 + 4 * 2 becomes 13632.
    Before you can help with the homework, you need to understand it yourself. Evaluate the expression on each line of the homework; what is the sum of the resulting values?
    */
    void solve_day_18_1(const std::filesystem::path& input_dir) {
        auto input = get_input(input_dir);
        std::for_each(input.begin(), input.end(), [](node::ptr& n){ balance<no_precedence>(n); });
        std::cout << '\t' << std::accumulate(input.begin(), input.end(), 0ll, [](int64_t acc, const node::ptr& n){
            return acc + evaluate(n);
        }) << '\n';
    }

    /*
    You manage to answer the child's questions and they finish part 1 of their homework, but get stuck when they reach the next section: advanced math.

    Now, addition and multiplication have different precedence levels, but they're not the ones you're familiar with. Instead, addition is evaluated before multiplication.

    For example, the steps to evaluate the expression 1 + 2 * 3 + 4 * 5 + 6 are now as follows:

    1 + 2 * 3 + 4 * 5 + 6
      3   * 3 + 4 * 5 + 6
      3   *   7   * 5 + 6
      3   *   7   *  11
         21       *  11
             231
    Here are the other examples from above:

    1 + (2 * 3) + (4 * (5 + 6)) still becomes 51.
    2 * 3 + (4 * 5) becomes 46.
    5 + (8 * 3 + 9 + 3 * 4 * 3) becomes 1445.
    5 * 9 * (7 * 3 * 3 + 9 * 3 + (8 + 6 * 4)) becomes 669060.
    ((2 + 4 * 9) * (6 + 9 * 8 + 6) + 6) + 2 + 4 * 2 becomes 23340.
    What do you get if you add up the results of evaluating the homework problems using these new rules?
    */
    void solve_day_18_2(const std::filesystem::path& input_dir) {
        auto input = get_input(input_dir);
        std::for_each(input.begin(), input.end(), [](node::ptr& n){ balance<add_precedence>(n); });
        std::cout << '\t' << std::accumulate(input.begin(), input.end(), 0ll, [](int64_t acc, const node::ptr& n){
            return acc + evaluate(n);
        }) << '\n';
    }

    TEST_SUITE("day18" * doctest::description("Tests for day 18 challenges.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day18:balance_test1" * doctest::description("Test 1 for parsing expressions.")) {
            auto str = "3 + (4 + 8 + 3 + 4 + 7 + 6) + 4 + 3 * 4 + ((5 * 6) + 2 * 5 * 2 + 8 * 3)"sv;
            auto root = parse(str);
            std::cout << *root << '\n';
            balance<add_precedence>(root);
            std::cout << *root << '\n';
            REQUIRE_EQ(to_string(*root), "(* (+ (+ (+ 3 (=  (+ (+ (+ (+ (+ 4 8) 3) 4) 7) 6))) 4) 3) (+ 4 (=  (* (* (* (+ (=  (* 5 6)) 2) 5) (+ 2 8)) 3))))");
        }
    }

} /* namespace aoc2020 */