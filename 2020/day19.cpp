//
// Created by Daniel Garcia on 12/19/20.
//

#include "day19.h"
#include "utilities.h"

#include <doctest/doctest.h>

#include <iostream>
#include <regex>
#include <vector>
#include <variant>
#include <iterator>
#include <algorithm>
#include <charconv>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        using sequence = std::vector<int>;
        using option_list = std::vector<sequence>;
        using term_opt = std::vector<std::string>;
        using rule = std::variant<option_list, char>;

        struct rule_entry {
            int id;
            rule entry;

            bool operator< (const rule_entry& rhs) const noexcept { return id <  rhs.id; }
            bool operator< (int rhs) const noexcept { return id <  rhs; }

            bool operator==(const rule_entry& rhs) const noexcept { return id == rhs.id; }
            bool operator==(int rhs) const noexcept { return id == rhs; }
        };

        bool operator< (int lhs, const rule_entry& rhs) { return lhs <  rhs.id; }
        bool operator==(int lhs, const rule_entry& rhs) { return lhs == rhs.id; }

        std::vector<int> unique_ids(const option_list& o) {
            std::vector<int> retval;
            for (const auto& s : o) {
                retval.insert(retval.end(), s.begin(), s.end());
            }
            std::sort(retval.begin(), retval.end());
            retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
            return retval;
        }

        rule_entry parse_rule(std::string_view s) {
            int id = 0;
            auto res = std::from_chars(s.data(), s.data() + s.size(), id);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            else if (res.ptr - s.data() + 2 >= s.size() || *res.ptr != ':' || !isspace(*(res.ptr + 1))) {
                throw std::runtime_error{"Rule after ID is invalid."};
            }
            s.remove_prefix(res.ptr - s.data() + 2); //Remove digit, colon, and space.
            if (s.front() == '"') {
                return { id, rule{s[1]} };
            }
            else {
                auto opt_strs = aoc::split(s, '|');
                option_list opts;
                for (const auto str : opt_strs) {
                    auto seq_str = aoc::split_no_empty(str, ' ');
                    sequence seq;
                    for (const auto ids : seq_str) {
                        int sid = 0;
                        auto sres = std::from_chars(ids.data(), ids.data() + ids.size(), sid);
                        if (sres.ec != std::errc{}) {
                            throw std::system_error{std::make_error_code(sres.ec)};
                        }
                        seq.push_back(sid);
                    }
                    opts.push_back(std::move(seq));
                }
                return { id, rule{ std::move(opts) }};
            }
        }

        std::pair<std::vector<rule_entry>, std::vector<std::string>> get_input(const fs::path &input_dir) {
            const auto lines = aoc::read_file_lines(input_dir / "2020" / "day_19_input.txt");
            const auto blank = std::find_if(lines.begin(), lines.end(), [](const std::string& s){ return s.empty(); });

            std::vector<rule_entry> rules;
            rules.reserve(std::distance(lines.begin(), blank));
            std::transform(lines.begin(), blank, std::back_inserter(rules), parse_rule);
            std::sort(rules.begin(), rules.end());
            return { {std::move(rules)}, {blank + 1, lines.end()} };
        }

        using index_list = std::vector<std::size_t>;

        index_list matches_rule(std::string_view str, const index_list& idxs, char c, const std::vector<rule_entry>& rules) {
            index_list valid_next;
            valid_next.reserve(idxs.size());
            for (auto idx : idxs) {
                if (idx < str.size() && str[idx] == c) {
                    valid_next.push_back(idx + 1);
                }
            }
            return valid_next;
        }

        index_list matches_rule(std::string_view str, const index_list& idxs, const rule_entry& rule, const std::vector<rule_entry>& rules);

        index_list matches_rule(std::string_view str, const index_list& idxs, const option_list& options, const std::vector<rule_entry>& rules) {
            std::vector<std::size_t> valid_next;
            valid_next.reserve(idxs.size() * options.size());
            for (const auto &opt : options) {
                bool matches_seq = true;
                index_list next = idxs;
                for (auto id : opt) {
                    index_list next_next;
                    //Assume rules are well formed, so don't check that we found it.
                    next = matches_rule(str, next, *std::find(rules.begin(), rules.end(), id), rules);
                    matches_seq = !next.empty();
                    if (!matches_seq) {
                        break;
                    }
                }
                if (matches_seq) {
                    valid_next.insert(valid_next.end(), next.begin(), next.end());
                }
            }
            std::sort(valid_next.begin(), valid_next.end());
            valid_next.erase(std::unique(valid_next.begin(), valid_next.end()), valid_next.end());
            return valid_next;
        }

        index_list matches_rule(std::string_view str, const index_list& idxs, const rule_entry& rule, const std::vector<rule_entry>& rules) {
            return std::visit([&](auto&& r){ return matches_rule(str, idxs, r, rules); }, rule.entry);
        }

        bool matches_rule_helper(std::string_view str, const rule_entry& rule, const std::vector<rule_entry>& rules) {
            auto matches = matches_rule(str, {0}, rule, rules);
            return std::any_of(matches.begin(), matches.end(), [str](std::size_t match_end){ return match_end == str.size(); });
        }

        bool matches_part_2(std::string_view str, const rule_entry& rule1, const rule_entry& rule2, const std::vector<rule_entry>& rules) {
            bool matching1 = true, matching2 = true;
            int num1 = 0, num2 = 0;
            index_list next = {0};
            while (matching1) {
                auto tmp = matches_rule(str, next, rule1, rules);
                matching1 = !tmp.empty();
                if (matching1) {
                    ++num1;
                    next = std::move(tmp);
                }
            }
            while (matching2) {
                auto tmp = matches_rule(str, next, rule2, rules);
                matching2 = !tmp.empty();
                if (matching2) {
                    ++num2;
                    next = std::move(tmp);
                }
            }
            return std::any_of(next.begin(), next.end(), [str](std::size_t idx){ return idx == str.size(); }) && num1 > 0 && num2 > 0 && num1 > num2;
        }

    }

    /*
    You land in an airport surrounded by dense forest. As you walk to your high-speed train, the Elves at the Mythical Information Bureau contact you again. They think their satellite has collected an image of a sea monster! Unfortunately, the connection to the satellite is having problems, and many of the messages sent back from the satellite have been corrupted.

    They sent you a list of the rules valid messages should obey and a list of received messages they've collected so far (your puzzle input).

    The rules for valid messages (the top part of your puzzle input) are numbered and build upon each other. For example:

    0: 1 2
    1: "a"
    2: 1 3 | 3 1
    3: "b"
    Some rules, like 3: "b", simply match a single character (in this case, b).

    The remaining rules list the sub-rules that must be followed; for example, the rule 0: 1 2 means that to match rule 0, the text being checked must match rule 1, and the text after the part that matched rule 1 must then match rule 2.

    Some of the rules have multiple lists of sub-rules separated by a pipe (|). This means that at least one list of sub-rules must match. (The ones that match might be different each time the rule is encountered.) For example, the rule 2: 1 3 | 3 1 means that to match rule 2, the text being checked must match rule 1 followed by rule 3 or it must match rule 3 followed by rule 1.

    Fortunately, there are no loops in the rules, so the list of possible matches will be finite. Since rule 1 matches a and rule 3 matches b, rule 2 matches either ab or ba. Therefore, rule 0 matches aab or aba.

    Here's a more interesting example:

    0: 4 1 5
    1: 2 3 | 3 2
    2: 4 4 | 5 5
    3: 4 5 | 5 4
    4: "a"
    5: "b"
    Here, because rule 4 matches a and rule 5 matches b, rule 2 matches two letters that are the same (aa or bb), and rule 3 matches two letters that are different (ab or ba).

    Since rule 1 matches rules 2 and 3 once each in either order, it must match two pairs of letters, one pair with matching letters and one pair with different letters. This leaves eight possibilities: aaab, aaba, bbab, bbba, abaa, abbb, baaa, or babb.

    Rule 0, therefore, matches a (rule 4), then any of the eight options from rule 1, then b (rule 5): aaaabb, aaabab, abbabb, abbbab, aabaab, aabbbb, abaaab, or ababbb.

    The received messages (the bottom part of your puzzle input) need to be checked against the rules so you can determine which are valid and which are corrupted. Including the rules and the messages together, this might look like:

    0: 4 1 5
    1: 2 3 | 3 2
    2: 4 4 | 5 5
    3: 4 5 | 5 4
    4: "a"
    5: "b"

    ababbb
    bababa
    abbbab
    aaabbb
    aaaabbb
    Your goal is to determine the number of messages that completely match rule 0. In the above example, ababbb and abbbab match, but bababa, aaabbb, and aaaabbb do not, producing the answer 2. The whole message must match all of rule 0; there can't be extra unmatched characters in the message. (For example, aaaabbb might appear to match rule 0 above, but it has an extra unmatched b on the end.)

    How many messages completely match rule 0?
    */
    void solve_day_19_1(const std::filesystem::path& input_dir) {
        auto [rules, messages] = get_input(input_dir);
        int matching = 0;
        const auto& rule0 = rules.front();
        for (const auto& m : messages) {
            if (matches_rule_helper(m, rule0, rules)) {
                ++matching;
            }
        }
        std::cout << '\t' << matching << '\n';
    }

    /*
    As you look over the list of messages, you realize your matching rules aren't quite right. To fix them, completely replace rules 8: 42 and 11: 42 31 with the following:

    8: 42 | 42 8
    11: 42 31 | 42 11 31
    This small change has a big impact: now, the rules do contain loops, and the list of messages they could hypothetically match is infinite. You'll need to determine how these changes affect which messages are valid.

    Fortunately, many of the rules are unaffected by this change; it might help to start by looking at which rules always match the same set of values and how those rules (especially rules 42 and 31) are used by the new versions of rules 8 and 11.

    (Remember, you only need to handle the rules you have; building a solution that could handle any hypothetical combination of rules would be significantly more difficult.)

    For example:

    42: 9 14 | 10 1
    9: 14 27 | 1 26
    10: 23 14 | 28 1
    1: "a"
    11: 42 31
    5: 1 14 | 15 1
    19: 14 1 | 14 14
    12: 24 14 | 19 1
    16: 15 1 | 14 14
    31: 14 17 | 1 13
    6: 14 14 | 1 14
    2: 1 24 | 14 4
    0: 8 11
    13: 14 3 | 1 12
    15: 1 | 14
    17: 14 2 | 1 7
    23: 25 1 | 22 14
    28: 16 1
    4: 1 1
    20: 14 14 | 1 15
    3: 5 14 | 16 1
    27: 1 6 | 14 18
    14: "b"
    21: 14 1 | 1 14
    25: 1 1 | 1 14
    22: 14 14
    8: 42
    26: 14 22 | 1 20
    18: 15 15
    7: 14 5 | 1 21
    24: 14 1

    abbbbbabbbaaaababbaabbbbabababbbabbbbbbabaaaa
    bbabbbbaabaabba
    babbbbaabbbbbabbbbbbaabaaabaaa
    aaabbbbbbaaaabaababaabababbabaaabbababababaaa
    bbbbbbbaaaabbbbaaabbabaaa
    bbbababbbbaaaaaaaabbababaaababaabab
    ababaaaaaabaaab
    ababaaaaabbbaba
    baabbaaaabbaaaababbaababb
    abbbbabbbbaaaababbbbbbaaaababb
    aaaaabbaabaaaaababaa
    aaaabbaaaabbaaa
    aaaabbaabbaaaaaaabbbabbbaaabbaabaaa
    babaaabbbaaabaababbaabababaaab
    aabbbbbaabbbaaaaaabbbbbababaaaaabbaaabba
    Without updating rules 8 and 11, these rules only match three messages: bbabbbbaabaabba, ababaaaaaabaaab, and ababaaaaabbbaba.

    However, after updating rules 8 and 11, a total of 12 messages match:

    bbabbbbaabaabba
    babbbbaabbbbbabbbbbbaabaaabaaa
    aaabbbbbbaaaabaababaabababbabaaabbababababaaa
    bbbbbbbaaaabbbbaaabbabaaa
    bbbababbbbaaaaaaaabbababaaababaabab
    ababaaaaaabaaab
    ababaaaaabbbaba
    baabbaaaabbaaaababbaababb
    abbbbabbbbaaaababbbbbbaaaababb
    aaaaabbaabaaaaababaa
    aaaabbaabbaaaaaaabbbabbbaaabbaabaaa
    aabbbbbaabbbaaaaaabbbbbababaaaaabbaaabba
    After updating rules 8 and 11, how many messages completely match rule 0?
    */
    void solve_day_19_2(const std::filesystem::path& input_dir) {
        auto [rules, messages] = get_input(input_dir);
        //We're going to take the easy way out.  The only place the changed rules are used is rule 0.
        //So we have from the challenge:
        //8: 42 | 42 8
        //11: 42 31 | 42 11 31
        //And from the input:
        //0: 8 11
        //Note that rule 11 by itself cannot be validated only through a regex because it requires counting.  But
        //we can note that rule 0 can meet the following regex:
        //0: (42)+ (31)+
        //Then we need to extract from the regex matches the number of captures of each sub-regex and make sure
        //that the number of the first is large than the number of the second, and that both are at least 1.
        const auto rule42 = std::find(rules.begin(), rules.end(), 42);
        const auto rule31 = std::find(rules.begin(), rules.end(), 31);
        if (rule42 == rules.end() || rule31 == rules.end()) {
            throw std::runtime_error{"Missing required rule."};
        }
        int matching = 0;
        for (const auto& m : messages) {
            if (matches_part_2(m, *rule42, *rule31, rules)) {
                ++matching;
            }
        }
        std::cout << '\t' << matching << '\n';
    }

    TEST_SUITE("day19" * doctest::description("Tests for day 19 challenges.")) {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        TEST_CASE("day19:matches_rule1" * doctest::description("Testing rule matching. (#1)")) {
            std::vector<rule_entry> rules = {
                    { 0, option_list{ {1, 2}, {1, 3} } },   //"abaaaaaba" or "aabaaaaba" or "bbbaaaaba" or "ababbaaba" or "aabbbaaba" or "bbbbbaaba" ! or "abaaaaaab" or "aabaaaaab" or "bbbaaaaab" or "ababbaaab" or "aabbbaaab" or "bbbbbaaab" !or  "abaaaabbb" or "aabaaabbb" or "bbbaaabbb" or "ababbabbb" or "aabbbabbb" or "bbbbbabbb" ! or "abaaaaab" or "aabaaaab" or "bbbaaaab" or "ababbaab" or "aabbbaab" or "bbbbbaab"
                    { 1, option_list{ {2, 4, 5} } },        //"abaaaa" or "aabaaa" or "bbbaaa" or "ababba" or "aabbba" or "bbbbba"
                    { 2, option_list{ {3, 5}, {4, 6} } },   //"aba" or "aab" or "bbb"
                    { 3, option_list{ {5, 6} } },           //"ab"
                    { 4, option_list{ {5, 5}, {6, 6} } },   //"aa" or "bb"
                    { 5, 'a' },
                    { 6, 'b' }
            };
            std::vector<std::string_view> valid = {
                    "abaaaaaba"sv,
                    "aabaaaaba"sv,
                    "bbbaaaaba"sv,
                    "ababbaaba"sv,
                    "aabbbaaba"sv,
                    "bbbbbaaba"sv,
                    "abaaaaaab"sv,
                    "aabaaaaab"sv,
                    "bbbaaaaab"sv,
                    "ababbaaab"sv,
                    "aabbbaaab"sv,
                    "bbbbbaaab"sv,
                    "abaaaabbb"sv,
                    "aabaaabbb"sv,
                    "bbbaaabbb"sv,
                    "ababbabbb"sv,
                    "aabbbabbb"sv,
                    "bbbbbabbb"sv,
                    "abaaaaab"sv,
                    "aabaaaab"sv,
                    "bbbaaaab"sv,
                    "ababbaab"sv,
                    "aabbbaab"sv,
                    "bbbbbaab"sv
            };
            for (auto str : valid) {
                CHECK_MESSAGE(matches_rule_helper(str, rules.front(), rules), str);
            }
        }
        TEST_CASE("day19:matches_rule2" * doctest::description("Testing rule matching. (#2)")) {
            std::vector<rule_entry> rules = {
                    { 0, 'a' }
            };
                    CHECK_FALSE_MESSAGE(matches_rule_helper(""sv, rules.front(), rules), "<Empty>");
                    CHECK_MESSAGE(matches_rule_helper("a"sv, rules.front(), rules), "a");
                    CHECK_FALSE_MESSAGE(matches_rule_helper("b"sv, rules.front(), rules), "b");
                    CHECK_FALSE_MESSAGE(matches_rule_helper("aa"sv, rules.front(), rules), "aa");
        }
        TEST_CASE("day19:matches_rule3" * doctest::description("Testing rule matching. (#3)")) {
            std::vector<rule_entry> rules = {
                    { 0, option_list{ {1, 2}, {1, 3}, {2, 4} } }, //"ab" or "ac" or "bd"
                    { 1, 'a' },
                    { 2, 'b' },
                    { 3, 'c' },
                    { 4, 'd' }
            };
            std::vector<std::pair<std::string_view, bool>> tests = {
                    { ""sv, false },
                    { "a"sv, false },
                    { "b"sv, false },
                    { "c"sv, false },
                    { "d"sv, false },
                    { "aa"sv, false },
                    { "ab"sv, true },
                    { "ac"sv, true },
                    { "ad"sv, false },
                    { "ba"sv, false },
                    { "bb"sv, false },
                    { "bc"sv, false },
                    { "bd"sv, true },
                    { "ca"sv, false },
                    { "cb"sv, false },
                    { "cc"sv, false },
                    { "cd"sv, false },
                    { "da"sv, false },
                    { "db"sv, false },
                    { "dc"sv, false },
                    { "dd"sv, false },
                    { "aaa"sv, false },
                    { "aba"sv, false },
                    { "aca"sv, false },
                    { "adb"sv, false },
                    { "bab"sv, false },
                    { "bbb"sv, false },
                    { "bcd"sv, false },
                    { "bdd"sv, false },
                    { "cad"sv, false },
                    { "cbd"sv, false },
                    { "ccc"sv, false },
                    { "cdc"sv, false },
                    { "dac"sv, false },
                    { "bdb"sv, false },
                    { "bdc"sv, false },
                    { "bdd"sv, false }
            };
            for (const auto test : tests) {
                        CHECK_MESSAGE(matches_rule_helper(test.first, rules.front(), rules) == test.second, test.first);
            }
        }
        TEST_CASE("day19:matches_rule4" * doctest::description("Testing rule matching. (#4)")) {
            std::vector<rule_entry> rules = {
                    {0, option_list{{1, 2}, {3, 4}}},
                    {1, option_list{{7}, {7, 8}}},
                    {2, option_list{{7, 7}}},
                    {3, option_list{{7}, {8, 8}}},
                    {4, option_list{{8}, {8, 8}}},
                    {7, 'a'},
                    {8, 'b'}
            };
            std::vector<std::pair<std::string_view, bool>> tests = {
                    {""sv, false},
                    {"aaa"sv, true},
                    {"abaa"sv, true},
                    {"ab"sv, true},
                    {"abb"sv, true},
                    {"bbb"sv, true},
                    {"bbbb"sv, true}
            };
            for (const auto test : tests) {
                        CHECK_MESSAGE(matches_rule_helper(test.first, rules.front(), rules) == test.second, test.first);
            }
        }
    }

} /* namespace aoc2020 */