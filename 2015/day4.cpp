//
// Created by Daniel Garcia on 4/27/21.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <openssl/evp.h>

#include <string>
#include <vector>
#include <cstdint>
#include <string_view>
#include <array>
#include <thread>
#include <atomic>
#include <algorithm>

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    const auto INPUT = "bgvyzdsv"sv;

    std::vector<uint8_t> calc_md5(const std::string& s) {
        std::vector<uint8_t> retval (EVP_MAX_MD_SIZE);
        unsigned int size = 0;
        const EVP_MD *md = EVP_md5();
        EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(md_ctx, md, nullptr);
        EVP_DigestUpdate(md_ctx, s.data(), s.size());
        EVP_DigestFinal_ex(md_ctx, retval.data(), &size);
        EVP_MD_CTX_free(md_ctx);
        retval.resize(size);
        return retval;
    }

    std::vector<uint8_t> calculate_value(uint64_t val) {
        return calc_md5(std::string{INPUT} + std::to_string(val));
    }

    std::string to_hex(const std::vector<uint8_t>& data) {
        constexpr auto digits = "0123456789abcdef"sv;
        std::string hex;
        hex.resize(data.size() * 2);
        char* buf = hex.data();
        for (uint8_t v : data) {
            *buf++ = digits[(v >> 4) & 0x0f];
            *buf++ = digits[ v       & 0x0f];
        }
        return hex;
    }

    bool starts_with_5_0s(const std::vector<uint8_t>& data) {
        return std::string_view{to_hex(data)}.substr(0, 5) == "00000"sv;
    }

    bool starts_with_6_0s(const std::vector<uint8_t>& data) {
        return std::string_view{to_hex(data)}.substr(0, 6) == "000000"sv;
    }

    std::string check_for_criteria(bool (*crit)(const std::vector<uint8_t>&)) {
        std::vector<std::thread> threads;
        constexpr int num_threads = 8;
        std::atomic<bool> found = false;
        std::atomic<uint64_t> value = 0;
        for (int i = 1; i <= num_threads; ++i) {
            threads.emplace_back([i, crit, &value, &found, num_threads](){
                for (uint64_t val = i; !found.load() || value.load() > val; val += num_threads) {
                    if (crit(calculate_value(val))) {
                        found.store(true);
                        uint64_t expected = 0;
                        if (value.compare_exchange_strong(expected, val)) {
                            return;
                        }
                        else {
                            while (val < expected && value.compare_exchange_strong(expected, val)) {}
                            return;
                        }
                    }
                }
            });
        }
        std::for_each(threads.begin(), threads.end(), [](std::thread& t){ t.join(); });
        return std::to_string(value.load());
    }

    /*
    --- Day 4: The Ideal Stocking Stuffer ---

    Santa needs help mining some AdventCoins (very similar to bitcoins) to use as gifts for all the economically forward-thinking little girls and boys.

    To do this, he needs to find MD5 hashes which, in hexadecimal, start with at least five zeroes. The input to the MD5 hash is some secret key (your puzzle input, given below) followed by a number in decimal. To mine AdventCoins, you must find Santa the lowest positive number (no leading zeroes: 1, 2, 3, ...) that produces such a hash.

    For example:

    If your secret key is abcdef, the answer is 609043, because the MD5 hash of abcdef609043 starts with five zeroes (000001dbbfa...), and it is the lowest such number to do so.
    If your secret key is pqrstuv, the lowest number it combines with to make an MD5 hash starting with five zeroes is 1048970; that is, the MD5 hash of pqrstuv1048970 looks like 000006136ef....
     */
    std::string part_1(const std::vector<std::string>& lines) {
        return check_for_criteria(starts_with_5_0s);
    }

    /*
    Now find one that starts with six zeroes.
     */
    std::string part_2(const std::vector<std::string>& lines) {
        return check_for_criteria(starts_with_6_0s);
    }

    aoc::registration r {2015, 4, part_1, part_2};

}