//
// Created by Daniel Garcia on 12/23/20.
//

#ifndef ADVENTOFCODE_REGISTRATION_H
#define ADVENTOFCODE_REGISTRATION_H

#include <string>
#include <filesystem>

namespace aoc {

    using challenge_function = std::string (*)(const std::filesystem::path& input_dir);

    class registration {
    public:
        //This may throw, but if it does the program's dead anyway.
        registration(int y, int d, challenge_function c1, challenge_function c2 = nullptr) noexcept;
    };

} /* namespace aoc */

#endif //ADVENTOFCODE_REGISTRATION_H
