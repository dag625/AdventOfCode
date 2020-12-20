//
// Created by Daniel Garcia on 12/8/20.
//

#ifndef ADVENTOFCODE_TIME_FORMAT_H
#define ADVENTOFCODE_TIME_FORMAT_H

#include <chrono>
#include <ostream>
#include <iomanip>

namespace std::chrono {

    template <typename Rep, typename Period>
    std::ostream& operator<<(std::ostream& os, std::chrono::duration<Rep, Period> v) {
        auto fill_char = os.fill();
        os << std::setfill('0');
        if (v > std::chrono::hours{24}) {
            auto days = std::chrono::duration_cast<std::chrono::duration<long, std::ratio<86400>>>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(days);
            os << days.count() << " days";
        }
        bool haveTime = false;
        if (v > std::chrono::hours{1}) {
            auto hours = std::chrono::duration_cast<std::chrono::hours>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(hours);
            os << hours.count() << ":";
            haveTime = true;
        }
        if (haveTime || v > std::chrono::minutes{1}) {
            auto mins = std::chrono::duration_cast<std::chrono::minutes>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(mins);
            os << mins.count() << ":";
            haveTime = true;
        }
        std::string units;
        bool havePeriod = false;
        if (haveTime || v > std::chrono::seconds{1}) {
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(v);
            v -= std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(secs);
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
                }
                os << std::setw(3) << ms.count();
            }
            auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(v).count();
            if (rem > 0) {
                os << std::setw(3) << rem;
            }
        }
        if (!haveTime) {
            os << units;
        }
        os.fill(fill_char);
        return os;
    }

} /* namespace aoc */

#endif //ADVENTOFCODE_TIME_FORMAT_H
