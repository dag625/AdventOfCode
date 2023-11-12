//
// Created by Dan on 11/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>

#include "utilities.h"
#include "ranges.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    struct section {
        std::string text;
        bool in_brackets = false;
    };

    using address = std::vector<section>;

    address parse_address(std::string_view s) {
        bool in_brackets = false;
        address retval;
        while (!s.empty()) {
            const auto end = s.find_first_of(in_brackets ? ']' : '[');
            if (end == std::string_view::npos) {
                retval.push_back({std::string{s}, in_brackets});
                in_brackets = !in_brackets;
                s.remove_prefix(s.size());
            }
            else {
                retval.push_back({std::string{s.substr(0, end)}, in_brackets});
                in_brackets = !in_brackets;
                s.remove_prefix(end + 1);
            }
        }
        return retval;
    }

    std::vector<address> get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_7_input.txt");
        return lines | std::views::transform(&parse_address) | to<std::vector<address>>();
    }

    bool has_abba(std::string_view s) {
        while (s.size() >= 4) {
            if (s[0] == s[3] && s[1] == s[2] && s[0] != s[1]) {
                return true;
            }
            s.remove_prefix(1);
        }
        return false;
    }

    using aba = std::array<char, 2>;

    void find_abas(std::string_view s, std::vector<aba>& abas) {
        while (s.size() >= 3) {
            if (s[0] == s[2] && s[0] != s[1]) {
                abas.push_back({s[0], s[1]});
            }
            s.remove_prefix(1);
        }
    }

    bool has_bab(std::string_view s, const aba& v) {
        while (s.size() >= 3) {
            if (s[0] == s[2] && s[0] == v[1] && s[1] == v[0]) {
                return true;
            }
            s.remove_prefix(1);
        }
        return false;
    }

    bool supports_tls(const address& addr) {
        return std::any_of(addr.begin(), addr.end(), [](const section& s){ return has_abba(s.text) && !s.in_brackets; }) &&
            std::none_of(addr.begin(), addr.end(), [](const section& s){ return has_abba(s.text) && s.in_brackets; });
    }

    bool supports_ssl(const address& addr) {
        std::vector<aba> abas;
        for (const auto& sec : addr) {
            if (!sec.in_brackets) {
                //We can have multiple abas in a section.
                find_abas(sec.text, abas);
            }
        }
        for (const auto& sec : addr) {
            if (sec.in_brackets && std::any_of(abas.begin(), abas.end(), [&sec](const aba& a){ return has_bab(sec.text, a); })) {
                return true;
            }
        }
        return false;
    }

    /*
    --- Day 7: Internet Protocol Version 7 ---
    While snooping around the local network of EBHQ, you compile a list of IP addresses (they're IPv7, of course; IPv6 is much too limited). You'd like to figure out which IPs support TLS (transport-layer snooping).

    An IP supports TLS if it has an Autonomous Bridge Bypass Annotation, or ABBA. An ABBA is any four-character sequence which consists of a pair of two different characters followed by the reverse of that pair, such as xyyx or abba. However, the IP also must not have an ABBA within any hypernet sequences, which are contained by square brackets.

    For example:

    abba[mnop]qrst supports TLS (abba outside square brackets).
    abcd[bddb]xyyx does not support TLS (bddb is within square brackets, even though xyyx is outside square brackets).
    aaaa[qwer]tyui does not support TLS (aaaa is invalid; the interior characters must be different).
    ioxxoj[asdfgh]zxcvbn supports TLS (oxxo is outside square brackets, even though it's within a larger string).
    How many IPs in your puzzle input support TLS?
    */
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num = std::count_if(input.begin(), input.end(), &supports_tls);
        return std::to_string(num);
    }

    /*
    --- Part Two ---
    You would also like to know which IPs support SSL (super-secret listening).

    An IP supports SSL if it has an Area-Broadcast Accessor, or ABA, anywhere in the supernet sequences (outside any square bracketed sections), and a corresponding Byte Allocation Block, or BAB, anywhere in the hypernet sequences. An ABA is any three-character sequence which consists of the same character twice with a different character between them, such as xyx or aba. A corresponding BAB is the same characters but in reversed positions: yxy and bab, respectively.

    For example:

    aba[bab]xyz supports SSL (aba outside square brackets with corresponding bab within square brackets).
    xyx[xyx]xyx does not support SSL (xyx, but no corresponding yxy).
    aaa[kek]eke supports SSL (eke in supernet with corresponding kek in hypernet; the aaa sequence is not related, because the interior character must be different).
    zazbz[bzb]cdb supports SSL (zaz has no corresponding aza, but zbz has a corresponding bzb, even though zaz and zbz overlap).
    How many IPs in your puzzle input support SSL?
    */
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        const auto num = std::count_if(input.begin(), input.end(), &supports_ssl);
        return std::to_string(num);//193 too low
    }

    aoc::registration r{2016, 7, part_1, part_2};

//    TEST_SUITE("2016_day7") {
//        TEST_CASE("2016_day7:example") {
//
//        }
//    }

}