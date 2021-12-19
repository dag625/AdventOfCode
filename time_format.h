//
// Created by Daniel Garcia on 12/8/20.
//

#ifndef ADVENTOFCODE_TIME_FORMAT_H
#define ADVENTOFCODE_TIME_FORMAT_H

#include <chrono>
#include <sstream>
#include <iomanip>

namespace aoc {

    template <typename Rep, typename Period>
    std::string time_to_string(std::chrono::duration<Rep, Period> v) {
        std::stringstream os;
        os << std::setfill('0');
        bool haveDays = false;
        if (v > std::chrono::hours{24}) {
            auto days = std::chrono::duration_cast<std::chrono::duration<long, std::ratio<86400>>>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(days);
            os << days.count() << " days";
            haveDays = true;
        }
        bool haveTime = false;
        if (v > std::chrono::hours{1}) {
            auto hours = std::chrono::duration_cast<std::chrono::hours>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(hours);
            if (haveDays) {
                os << ' ';
            }
            os << hours.count() << ":";
            haveTime = true;
        }
        if (haveTime || v > std::chrono::minutes{1}) {
            auto mins = std::chrono::duration_cast<std::chrono::minutes>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(mins);
            if (haveTime) {
                os << std::setw(2) << std::setfill('0');
            }
            os << mins.count() << ":";
            haveTime = true;
        }
        std::string units;
        bool havePeriod = false;
        if (haveTime || v > std::chrono::seconds{1}) {
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(secs);
            if (haveTime) {
                os << std::setw(2) << std::setfill('0');
            }
            os << secs.count();
            units = "s";
        }
        if (v > std::chrono::milliseconds{1}) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(ms);
            if (units.empty()) {
                os << ms.count();
                units = "ms";
            }
            else if (ms.count() > 0) {
                os << '.' << std::setw(3) << ms.count();
                havePeriod = true;
            }
            else if (v.count() > 0) {
                os << ".000";
                havePeriod = true;
            }
        }
        if (v > std::chrono::microseconds{1}) {
            auto ms = std::chrono::duration_cast<std::chrono::microseconds>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(ms);
            if (units.empty()) {
                os << ms.count();
                units = "us";
            }
            else if (ms.count() > 0) {
                if (!havePeriod) {
                    os << '.';
                    havePeriod = true;
                }
                os << std::setw(3) << ms.count();
            }
            auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(v).count();
            if (rem > 0) {
                if (!havePeriod) {
                    os << ".";
                    havePeriod = true;
                }
                os << std::setw(3) << rem;
            }
        }
        if (!haveTime) {
            os << units;
        }
        return os.str();
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_TIME_FORMAT_H
