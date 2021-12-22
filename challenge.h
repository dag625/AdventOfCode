//
// Created by Daniel Garcia on 12/23/20.
//

#ifndef ADVENTOFCODE_CHALLENGE_H
#define ADVENTOFCODE_CHALLENGE_H

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace aoc {

    struct challenge {
        using function = std::string (*)(const std::filesystem::path& input_dir);
        function func = nullptr;
        int year = 0;
        int day = 0;
        int num = 0;
        challenge() = default;
        challenge(int y, int d, int n, function f) noexcept :
            year{y}, day{d}, num{n}, func{f} {}
    };

    class challenges {
        std::vector<challenge> m_challenges;

        challenges() = default;
    public:
        static challenges& list();

        void add(int y, int d, int n, challenge::function f) noexcept;
        void run_all(std::optional<int> y, std::optional<int> d, std::optional<int> n, const std::filesystem::path& input_dir, bool use_markdown_output_fmt = false) const noexcept;
    };

} /* namespace aoc */

#endif //ADVENTOFCODE_CHALLENGE_H
