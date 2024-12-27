//
// Created by Daniel Garcia on 12/4/20.
//

#include "registration.h"
#include "ranges.h"

#include <string_view>
#include <algorithm>
#include <numeric>
#include <regex>
#include <fstream>
#include <charconv>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        enum class field_type : uint16_t {
            birth_year = 0x0001,//byr
            issue_year = 0x0002,//iyr
            expiration_year = 0x0004,//eyr
            height = 0x0008,//hgt
            hair_color = 0x0010,//hcl
            eye_color = 0x0020,//ecl
            passport_id = 0x0040,//pid
            country_id = 0x0080,//cid
            all_credential = 0x007f,
            all_passport = 0x00ff,
            none = 0x0000,
            invalid = 0xff00
        };

        field_type operator|(field_type a, field_type b) noexcept {
            using utype = std::underlying_type_t<field_type>;
            return static_cast<field_type>( static_cast<utype>(a) | static_cast<utype>(b) );
        }

        field_type parse_type(std::string_view name) noexcept {
            using namespace std::string_view_literals;
            if (name == "byr"sv) {
                return field_type::birth_year;
            }
            else if (name == "iyr"sv) {
                return field_type::issue_year;
            }
            else if (name == "eyr"sv) {
                return field_type::expiration_year;
            }
            else if (name == "hgt"sv) {
                return field_type::height;
            }
            else if (name == "hcl"sv) {
                return field_type::hair_color;
            }
            else if (name == "ecl"sv) {
                return field_type::eye_color;
            }
            else if (name == "pid"sv) {
                return field_type::passport_id;
            }
            else if (name == "cid"sv) {
                return field_type::country_id;
            }
            else {
                return field_type::invalid;
            }
        }

        [[nodiscard]] bool is_year_valid(const std::string& s, int min, int max) noexcept {
            int year = 0;
            const auto end = s.data() + s.size();
            auto res = std::from_chars(s.data(), end, year);
            if (res.ec == std::errc{} && res.ptr == end) {
                return year >= min && year <= max;
            }
            return false;
        }

        [[nodiscard]] bool is_height_valid(const std::string& s) {
            using namespace std::string_view_literals;
            static std::regex re {"([0-9]{2,3})(in|cm)"};
            std::smatch match;
            if (std::regex_match(s, match, re)) {
                bool inches = match[2].str() == "in"sv;
                int val = 0;
                std::from_chars(&*match[1].first, &*match[1].second, val);
                if (inches) {
                    return val >= 59 && val <= 76;
                }
                else {
                    return val >= 150 && val <= 193;
                }
            }
            return false;
        }

        [[nodiscard]] bool is_hair_color_valid(const std::string& s) noexcept {
            if (s.size() == 7 && s.front() == '#') {
                return std::all_of(s.begin() + 1, s.end(),
                                   [](char c){ return (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'); });
            }
            return false;
        }

        [[nodiscard]] bool is_eye_color_valid(const std::string& s) noexcept {
            using namespace std::string_view_literals;
            return s == "amb"sv ||
                    s == "blu"sv ||
                    s == "brn"sv ||
                    s == "gry"sv ||
                    s == "grn"sv ||
                    s == "hzl"sv ||
                    s == "oth"sv;
        }

        [[nodiscard]] bool is_passport_id_valid(const std::string& s) noexcept {
            if (s.size() == 9) {
                return std::all_of(s.begin(), s.end(),
                                   [](char c){ return c >= '0' && c <= '9'; });
            }
            return false;
        }

        struct field {
            field_type type;
            std::string data;

            [[nodiscard]] bool is_valid() const {
                switch (type) {
                    case field_type::birth_year:
                        return is_year_valid(data, 1920, 2002);
                    case field_type::issue_year:
                        return is_year_valid(data, 2010, 2020);
                    case field_type::expiration_year:
                        return is_year_valid(data, 2020, 2030);
                    case field_type::height:
                        return is_height_valid(data);
                    case field_type::hair_color:
                        return is_hair_color_valid(data);
                    case field_type::eye_color:
                        return is_eye_color_valid(data);
                    case field_type::passport_id:
                        return is_passport_id_valid(data);
                    case field_type::country_id:
                        return true;
                    default:
                        return false;
                }
            }
        };

        struct id {
            std::vector<field> fields;

            [[nodiscard]] bool is_valid(bool allow_credentials, bool check_fields) const noexcept {
                auto types = std::accumulate(fields.begin(), fields.end(), field_type::none, [check_fields](field_type acc, const field& f){
                    if (acc == field_type::invalid || f.type == field_type::invalid || (check_fields && !f.is_valid())) {
                        return field_type::invalid;
                    }
                    return acc | f.type;
                });
                return types == field_type::all_passport || (allow_credentials && types == field_type::all_credential);
            }
        };

        std::vector<id> parse(const std::string& file) {
            static std::regex re {"([a-z]{3}):([A-Za-z0-9_#-]+)"};
            std::smatch matches;
            std::vector<id> ids;
            std::vector<field> fields;
            fields.reserve(8);
            auto start = file.begin();
            const auto end = file.end();
            while (std::regex_search(start, end, matches, re)) {
                fields.push_back({parse_type(matches[1].str()), matches[2].str()});
                start = matches[0].second;
                if (auto next = matches[0].second;
                        next == end ||
                        ((next + 1) != end && *(next + 1) == '\n'))
                {
                    ids.emplace_back();
                    ids.back().fields.reserve(8);
                    ids.back().fields.swap(fields);
                }
            }
            if (!fields.empty()) {
                ids.push_back({std::move(fields)});
            }
            return ids;
        }

        std::vector<id> get_input(const std::vector<std::string>& lines) {
            auto retval = parse(lines | std::views::join_with('\n') | std::ranges::to<std::string>());
            if (std::any_of(retval.begin(), retval.end(),
                            [](const id& i){ return std::any_of(i.fields.begin(), i.fields.end(),
                                                                [](const field& f){ return f.type == field_type::invalid; }); }))
            {
                throw std::runtime_error{"Invalid field name in data."};
            }
            return retval;
        }

    }


    /************************* Part 1 *************************/
    std::string solve_day_4_1(const std::vector<std::string>& lines) {
        auto ids = get_input(lines);
        auto num_valid = std::count_if(ids.begin(), ids.end(), [](const id& i){
            return i.is_valid(true, false);
        });
        return std::to_string(num_valid);
    }


    /************************* Part 2 *************************/
    std::string solve_day_4_2(const std::vector<std::string>& lines) {
        auto ids = get_input(lines);
        auto num_valid = std::count_if(ids.begin(), ids.end(), [](const id& i){
            return i.is_valid(true, true);
        });
        return std::to_string(num_valid);
    }

    static aoc::registration r {2020, 4, solve_day_4_1, solve_day_4_2};

} /* namespace aoc2020 */
