//
// Created by Daniel Garcia on 12/1/20.
//

#include "utilities.h"

namespace aoc {

    namespace {

        const char *whitespace = " \t\n";

    }

    std::string ltrim(const std::string& s) {
        auto pos = s.find_first_not_of(whitespace);
        return s.substr(pos);
    }

    std::string rtrim(const std::string& s) {
        auto pos = s.find_last_not_of(whitespace);
        if (pos == std::string::npos) {
            pos = 0;
        }
        else {
            ++pos;
        }
        return s.substr(0, pos);
    }

    std::string trim(const std::string& s) {
        return ltrim(rtrim(s));
    }

} /* namespace aoc */