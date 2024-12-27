//
// Created by Dan on 11/11/2023.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>

#include <openssl/evp.h>

#include "utilities.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr auto DOOR_ID_FMT = "cxdnnyjw{}"sv;

    std::vector<unsigned char> hash(const std::string& s, EVP_MD_CTX* ctx, const EVP_MD* md) {
        const auto res1 = EVP_DigestInit_ex2(ctx, md, nullptr);
        const auto res2 = EVP_DigestUpdate(ctx, s.data(), s.size());
        std::vector<unsigned char> buf;
        buf.resize(EVP_MAX_MD_SIZE);
        unsigned int size = 0;
        const auto res3 = EVP_DigestFinal_ex(ctx, buf.data(), &size);
        buf.resize(size);
        return buf;
    }

    /*
    --- Day 5: How About a Nice Game of Chess? ---
    You are faced with a security door designed by Easter Bunny engineers that seem to have acquired most of their security knowledge by watching hacking movies.

    The eight-character password for the door is generated one character at a time by finding the MD5 hash of some Door ID (your puzzle input) and an increasing integer index (starting with 0).

    A hash indicates the next character in the password if its hexadecimal representation starts with five zeroes. If it does, the sixth character in the hash is the next character of the password.

    For example, if the Door ID is abc:

    The first index which produces a hash that starts with five zeroes is 3231929, which we find by hashing abc3231929; the sixth character of the hash, and thus the first character of the password, is 1.
    5017308 produces the next interesting hash, which starts with 000008f82..., so the second character of the password is 8.
    The third time a hash starts with five zeroes is for abc5278568, discovering the character f.
    In this example, after continuing this search a total of eight times, the password is 18f47a30.

    Given the actual Door ID, what is the password?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx {EVP_MD_CTX_new(), &EVP_MD_CTX_free};
        std::string pass;
        for (int i = 0; i < std::numeric_limits<int>::max(); ++i) {
            const auto buf = hash(fmt::format(DOOR_ID_FMT, i), ctx.get(), EVP_md5());
            if (buf.size() > 2 && buf[0] == 0 && buf[1] == 0 && (buf[2] & 0xf0) == 0) {
                const auto val = buf[2] & 0x0f;
                if (val < 10) {
                    pass.push_back(static_cast<char>(val + '0'));
                }
                else {
                    pass.push_back(static_cast<char>(val - 10 + 'a'));
                }
            }
            if (pass.size() >= 8) {
                break;
            }
        }
        return pass;
    }

    /*
    --- Part Two ---
    As the door slides open, you are presented with a second door that uses a slightly more inspired security mechanism. Clearly unimpressed by the last version (in what movie is the password decrypted in order?!), the Easter Bunny engineers have worked out a better solution.

    Instead of simply filling in the password from left to right, the hash now also indicates the position within the password to fill. You still look for hashes that begin with five zeroes; however, now, the sixth character represents the position (0-7), and the seventh character is the character to put in that position.

    A hash result of 000001f means that f is the second character in the password. Use only the first result for each position, and ignore invalid positions.

    For example, if the Door ID is abc:

    The first interesting hash is from abc3231929, which produces 0000015...; so, 5 goes in position 1: _5______.
    In the previous method, 5017308 produced an interesting hash; however, it is ignored, because it specifies an invalid position (8).
    The second interesting hash is at index 5357525, which produces 000004e...; so, e goes in position 4: _5__e___.
    You almost choke on your popcorn as the final character falls into place, producing the password 05ace8e3.

    Given the actual Door ID and this new method, what is the password? Be extra proud of your solution if it uses a cinematic "decrypting" animation.
    */
    std::string part_2(const std::vector<std::string>& lines) {
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx {EVP_MD_CTX_new(), &EVP_MD_CTX_free};
        std::array<std::optional<char>, 8> pass{};
        for (int i = 0; i < std::numeric_limits<int>::max(); ++i) {
            const auto buf = hash(fmt::format(DOOR_ID_FMT, i), ctx.get(), EVP_md5());
            if (buf.size() > 3 && buf[0] == 0 && buf[1] == 0 && (buf[2] & 0xf0) == 0) {
                const unsigned char idx = buf[2] & 0x0f;
                const auto val = (buf[3] & 0xf0) >> 4;
                if (idx < pass.size() && !pass[idx].has_value()) {
                    if (val < 10) {
                        pass[idx] = static_cast<char>(val + '0');
                    }
                    else {
                        pass[idx] = static_cast<char>(val - 10 + 'a');
                    }
                }

            }
            if (std::all_of(pass.begin(), pass.end(), [](const std::optional<char>& c){ return c.has_value(); })) {
                break;
            }
        }
        std::string passStr;
        for (const auto& c : pass) {
            passStr.push_back(*c);
        }
        return passStr;
    }

    aoc::registration r{2016, 5, part_1, part_2};

//    TEST_SUITE("2016_day5") {
//        TEST_CASE("2016_day5:example") {
//
//        }
//    }

}