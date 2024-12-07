//
// Created by Dan on 12/5/2024.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <openssl/evp.h>

#include <iostream>
#include <array>
#include <vector>

#include "utilities.h"
#include "openssl_hash.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    /*
     See https://adventofcode.com/2016/day/14
     */

    std::string get_input(const fs::path &input_dir) {
        const auto lines = read_file_lines(input_dir / "2016" / "day_14_input.txt");
        return lines.front();
    }

    std::string calc_hash(openssl_hash& h, const std::string& salt, const int64_t index) {
        const auto secret = fmt::format("{}{}", salt, index);
        h.reinit();
        h.add(secret);
        return h.finalizeHex();
    }

    std::string calc_stretched_hash(openssl_hash& h, const std::string& salt, const int64_t index, const int repititions = 2016) {
        auto retval = calc_hash(h, salt, index);
        for (int i = 0; i < repititions; ++i) {
            h.reinit();
            h.add(retval);
            retval = h.finalizeHex();
        }
        return retval;
    }

    using set_state = std::array<std::vector<int64_t>, 16>;

    std::vector<int64_t> find_sets(const std::string& s, const int64_t index, set_state& state) {
        char last = '\0';
        int len = 0;
        std::vector<int64_t> keys;
        bool have_triplet = false;
        for (const char c : s) {
            if (c == last) {
                ++len;
                int stateIndex = isalpha(c) ? (c - 'a' + 10) : (c - '0');
                if (len == 3 && !have_triplet) {
                    state[stateIndex].push_back(index);
                    have_triplet = true;
                }
                else if (len == 5) {
                    keys.insert(keys.end(), state[stateIndex].begin(), state[stateIndex].end() - 1);
                    state[stateIndex].erase(state[stateIndex].begin(), state[stateIndex].end() - 1);
                }
            }
            else {
                last = c;
                len = 1;
            }
        }
        return keys;
    }

    void clear_state(const int64_t index, set_state& state) {
        for (auto& idxs : state) {
            idxs.erase(std::remove_if(idxs.begin(), idxs.end(),
                                      [index](const int64_t i){ return index - i >= 1000; }),
                       idxs.end());
        }
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        openssl_hash h {EVP_md5()};
        std::vector<int64_t> key_indicies;
        int64_t index = 0, last_key_index = 0;
        set_state state{};
        constexpr int MAX_NUM_KEYS = 64;
        while (key_indicies.size() < MAX_NUM_KEYS) {
            const auto candidate = calc_hash(h, input, index);
            auto found = find_sets(candidate, index, state);
            key_indicies.insert(key_indicies.end(), found.begin(), found.end());
            ++index;
            clear_state(index, state);
        }
        std::sort(key_indicies.begin(), key_indicies.end());
        return std::to_string(key_indicies[MAX_NUM_KEYS - 1]);
    }

    /************************* Part 2 *************************/
    std::string part_2(const std::filesystem::path &input_dir) {
        const auto input = get_input(input_dir);
        openssl_hash h {EVP_md5()};
        std::vector<int64_t> key_indicies;
        int64_t index = 0, last_key_index = 0;
        set_state state{};
        constexpr int MAX_NUM_KEYS = 64;
        while (key_indicies.size() < MAX_NUM_KEYS) {
            const auto candidate = calc_stretched_hash(h, input, index);
            auto found = find_sets(candidate, index, state);
            key_indicies.insert(key_indicies.end(), found.begin(), found.end());
            ++index;
            clear_state(index, state);
        }
        std::sort(key_indicies.begin(), key_indicies.end());
        return std::to_string(key_indicies[MAX_NUM_KEYS - 1]);
        //Takes about ~35s on release build on my computer.
    }

    aoc::registration r{2016, 14, part_1, part_2};

    TEST_SUITE("2016_day14") {
        TEST_CASE("2016_day14:test_hash") {
            openssl_hash h {EVP_md5()};
            const auto h1 = calc_hash(h, "abc", 18);
            CHECK(h1.contains("888"));
        }
        TEST_CASE("2016_day14:example") {
            const std::string input = "abc";
            openssl_hash h {EVP_md5()};
            std::vector<int64_t> key_indicies;
            int64_t index = 0, last_key_index = 0;
            set_state state{};
            constexpr int MAX_NUM_KEYS = 64;
            while (key_indicies.size() < MAX_NUM_KEYS) {
                const auto candidate = calc_hash(h, input, index);
                auto found = find_sets(candidate, index, state);
                key_indicies.insert(key_indicies.end(), found.begin(), found.end());
                ++index;
                clear_state(index, state);
            }
            std::sort(key_indicies.begin(), key_indicies.end());
            CHECK_EQ(key_indicies[MAX_NUM_KEYS - 1], 22728);
        }
    }

} /* namespace <anon> */