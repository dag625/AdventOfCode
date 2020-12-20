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
        using option = std::vector<sequence>;
        using term_opt = std::vector<std::string>;
        using rule = std::variant<option, char>;

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

        struct option_entry {
            int id;
            option entry;

            bool operator< (const option_entry& rhs) const noexcept { return id <  rhs.id; }
            bool operator< (int rhs) const noexcept { return id <  rhs; }

            bool operator==(const option_entry& rhs) const noexcept { return id == rhs.id; }
            bool operator==(int rhs) const noexcept { return id == rhs; }
        };

        bool operator< (int lhs, const option_entry& rhs) { return lhs <  rhs.id; }
        bool operator==(int lhs, const option_entry& rhs) { return lhs == rhs.id; }

        struct terminal {
            int id;
            term_opt options;

            bool operator< (const terminal& rhs) const noexcept { return id <  rhs.id; }
            bool operator< (int rhs) const noexcept { return id <  rhs; }

            bool operator==(const terminal& rhs) const noexcept { return id == rhs.id; }
            bool operator==(int rhs) const noexcept { return id == rhs; }
        };

        bool operator< (int lhs, const terminal& rhs) { return lhs <  rhs.id; }
        bool operator==(int lhs, const terminal& rhs) { return lhs == rhs.id; }

        std::vector<int> unique_ids(const option& o) {
            std::vector<int> retval;
            for (const auto& s : o) {
                retval.insert(retval.end(), s.begin(), s.end());
            }
            std::sort(retval.begin(), retval.end());
            retval.erase(std::unique(retval.begin(), retval.end()), retval.end());
            return retval;
        }

        std::vector<std::string> to_terminal_sequence(const std::vector<std::vector<terminal>::const_iterator>::iterator begin,
                                                      const std::vector<std::vector<terminal>::const_iterator>::iterator end)
        {
            if (begin == end) {
                return {};
            }
            auto after = to_terminal_sequence(begin + 1, end);
            std::vector<std::string> retval;
            if (after.empty()) {
                for (const auto &opt : (*begin)->options) {
                    retval.push_back(opt);
                }
            }
            else {
                const auto &terms = *begin;
                retval.reserve(after.size() * terms->options.size());
                for (const auto &opt : terms->options) {
                    for (const auto &t : after) {
                        retval.push_back(opt + t);
                    }
                }
            }
            return retval;
        }

        std::vector<std::string> to_terminal_sequence(const sequence& s, const std::vector<terminal>& terminals) {
            std::vector<std::vector<terminal>::const_iterator> terms;
            for (const auto& id : s) {
                terms.push_back(std::find(terminals.begin(), terminals.end(), id));
            }
            return to_terminal_sequence(terms.begin(), terms.end());
        }

        int num_non_terminals(const option& o, const std::vector<terminal>& terminals) {
            auto ids = unique_ids(o);
            return std::count_if(ids.begin(), ids.end(), [&terminals](int id){
                return std::none_of(terminals.begin(), terminals.end(), [id](const terminal& t){ return id == t.id; });
            });
        }

        std::vector<terminal> terminal_rules_from_rules(const std::vector<rule_entry>& rules) {
            std::vector<terminal> terminals;
            std::vector<option_entry> options;
            terminals.reserve(rules.size());
            options.reserve(rules.size());
            for (const auto& r : rules) {
                if (std::holds_alternative<char>(r.entry)) {
                    terminals.push_back({ r.id, { std::string{} + std::get<char>(r.entry) } });
                }
                else {
                    options.push_back({ r.id, std::get<option>(r.entry) });
                }
            }
            while (!options.empty()) {
                for (auto r = options.begin(); r != options.end();) {
                    if (num_non_terminals(r->entry, terminals) == 0) {
                        std::vector<std::string> opts;
                        for (const auto& e : r->entry) {
                            auto eopts = to_terminal_sequence(e, terminals);
                            auto mid = opts.size();
                            opts.insert(opts.end(), eopts.begin(), eopts.end());
                            std::inplace_merge(opts.begin(), opts.begin() + mid, opts.end());
                        }
                        terminals.push_back({ r->id, std::move(opts) });
                        r = options.erase(r);
                    }
                    else {
                        ++r;
                    }
                }
            }
            std::sort(terminals.begin(), terminals.end());
            return terminals;
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
                option opts;
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

        std::pair<std::vector<terminal>, std::vector<std::string>> get_input(const fs::path &input_dir) {
            const auto lines = aoc::read_file_lines(input_dir / "2020" / "day_19_input.txt");
            const auto blank = std::find_if(lines.begin(), lines.end(), [](const std::string& s){ return s.empty(); });

            std::vector<rule_entry> rules;
            rules.reserve(std::distance(lines.begin(), blank));
            std::transform(lines.begin(), blank, std::back_inserter(rules), parse_rule);
            return { terminal_rules_from_rules(rules), {blank + 1, lines.end()} };
        }

        std::pair<std::string_view, int> count_matches(std::string_view msg, const terminal& term) {
            int num = 0;
            while (!msg.empty()) {
                bool any = false;
                for (const auto& t : term.options) {
                    auto found = aoc::starts_with(msg, t);
                    if (found) {
                        any = true;
                        ++num;
                        msg = *found;
                        break;
                    }
                }
                if (!any) {
                    break;
                }
            }
            return { msg, num };
        }

        bool matches_part_2(std::string_view msg, const terminal& term1, const terminal& term2) {
            auto [rest, count1] = count_matches(msg, term1);
            auto [end, count2] = count_matches(rest, term2);
            if (!end.empty() || count1 < 2 || count2 < 1) {
                return false;
            }
            else {
                return count1 > count2;
            }
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
        const auto& rule0 = rules.front().options;
        const auto rbegin = rule0.begin();
        const auto rend = rule0.end();
        for (const auto& m : messages) {
            auto found = std::lower_bound(rbegin, rend, m);
            if (found != rend && *found == m) {
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
            if (matches_part_2(m, *rule42, *rule31)) {
                ++matching;
            }
        }
        std::cout << '\t' << matching << '\n';
    }

    TEST_SUITE("day19" * doctest::description("Tests for day 19 challenges.")) {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        TEST_CASE("day19:count_matches1" * doctest::description("Testing counting the number of times a terminal shows up at the start of a string. (#1)")) {
            terminal term { 0, { "a"s } };
                    REQUIRE_EQ(count_matches("a"sv, term).second, 1);
                    REQUIRE(count_matches("a"sv, term).first.empty());
                    REQUIRE_EQ(count_matches("aaaaa"sv, term).second, 5);
                    REQUIRE(count_matches("aaaaa"sv, term).first.empty());
                    REQUIRE_EQ(count_matches("aaaabb"sv, term).second, 4);
                    REQUIRE_EQ(count_matches("aaaabb"sv, term).first, "bb"sv);
                    REQUIRE_EQ(count_matches("aaaabbaa"sv, term).second, 4);
                    REQUIRE_EQ(count_matches("aaaabbaa"sv, term).first, "bbaa"sv);
        }
        TEST_CASE("day19:count_matches2" * doctest::description("Testing counting the number of times a terminal shows up at the start of a string. (#2)")) {
            terminal term { 0, { "ab"s, "ba"s } };
                    REQUIRE_EQ(count_matches("ab"sv, term).second, 1);
                    REQUIRE(count_matches("ab"sv, term).first.empty());
                    REQUIRE_EQ(count_matches("ba"sv, term).second, 1);
                    REQUIRE(count_matches("ba"sv, term).first.empty());
                    REQUIRE_EQ(count_matches("bababa"sv, term).second, 3);
                    REQUIRE(count_matches("bababa"sv, term).first.empty());
                    REQUIRE_EQ(count_matches("baabbaab"sv, term).second, 4);
                    REQUIRE(count_matches("baabbaab"sv, term).first.empty());
                    REQUIRE_EQ(count_matches("bababaaa"sv, term).second, 3);
                    REQUIRE_EQ(count_matches("bababaaa"sv, term).first, "aa"sv);
                    REQUIRE_EQ(count_matches("baabbb"sv, term).second, 2);
                    REQUIRE_EQ(count_matches("baabbb"sv, term).first, "bb"sv);
                    REQUIRE_EQ(count_matches("babbaa"sv, term).second, 1);
                    REQUIRE_EQ(count_matches("babbaa"sv, term).first, "bbaa"sv);
        }
        TEST_CASE("day19:count_matches3" * doctest::description("Testing counting the number of times a terminal shows up at the start of a string. (#3)")) {
            terminal term { 0, { "aba"s, "baa"s, "aab"s } };
                    REQUIRE_EQ(count_matches("a"sv, term).second, 0);
                    REQUIRE_EQ(count_matches("a"sv, term).first, "a"sv);
                    REQUIRE_EQ(count_matches("ba"sv, term).second, 0);
                    REQUIRE_EQ(count_matches("ba"sv, term).first, "ba"sv);
                    REQUIRE_EQ(count_matches("bab"sv, term).second, 0);
                    REQUIRE_EQ(count_matches("bab"sv, term).first, "bab"sv);
                    REQUIRE_EQ(count_matches("baa"sv, term).second, 1);
                    REQUIRE_EQ(count_matches("baa"sv, term).first, ""sv);
                    REQUIRE_EQ(count_matches("baabaabaa"sv, term).second, 3);
                    REQUIRE_EQ(count_matches("baabaabaa"sv, term).first, ""sv);
                    REQUIRE_EQ(count_matches("baaababaa"sv, term).second, 3);
                    REQUIRE_EQ(count_matches("baaababaa"sv, term).first, ""sv);
                    REQUIRE_EQ(count_matches("baaaabbaaaba"sv, term).second, 4);
                    REQUIRE_EQ(count_matches("baaaabbaaaba"sv, term).first, ""sv);
                    REQUIRE_EQ(count_matches("baabaabaabba"sv, term).second, 3);
                    REQUIRE_EQ(count_matches("baabaabaabba"sv, term).first, "bba"sv);
                    REQUIRE_EQ(count_matches("baaababaabbaab"sv, term).second, 3);
                    REQUIRE_EQ(count_matches("baaababaabbaab"sv, term).first, "bbaab"sv);
                    REQUIRE_EQ(count_matches("baaaabbaaabaaaabaa"sv, term).second, 4);
                    REQUIRE_EQ(count_matches("baaaabbaaabaaaabaa"sv, term).first, "aaabaa"sv);
        }
        TEST_CASE("day19:pt2_match1" * doctest::description("Testing match check for part 2. (#1)")) {
            terminal term1 { 0, { "a"s } }, term2 { 1, { "b"s } };
                    REQUIRE(matches_part_2("aab"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("ab"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("b"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("a"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2(""sv, term1, term2));
                    REQUIRE(matches_part_2("aaaaaaaaaaaaaab"sv, term1, term2));
                    REQUIRE(matches_part_2("aaaabbb"sv, term1, term2));
                    REQUIRE(matches_part_2("aaaaaaaaaaabbbbbb"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("aaaabbbbbbbbbbb"sv, term1, term2));
        }
        TEST_CASE("day19:pt2_match2" * doctest::description("Testing match check for part 2. (#2)")) {
            terminal term1 { 0, { "ab"s, "ba"s } }, term2 { 1, { "aa"s, "bb"s } };
                    REQUIRE_FALSE(matches_part_2(""sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("ab"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("ba"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("bb"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("aa"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("abbb"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("babb"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("abaa"sv, term1, term2));
                    REQUIRE_FALSE(matches_part_2("baaa"sv, term1, term2));
                    REQUIRE(matches_part_2("abbaaa"sv, term1, term2));
                    REQUIRE(matches_part_2("baabbb"sv, term1, term2));
                    REQUIRE(matches_part_2("ababaa"sv, term1, term2));
                    REQUIRE(matches_part_2("bababb"sv, term1, term2));
                    REQUIRE(matches_part_2("baabbababbaa"sv, term1, term2));
        }
    }

} /* namespace aoc2020 */