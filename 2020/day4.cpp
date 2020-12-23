//
// Created by Daniel Garcia on 12/4/20.
//

#include "registration.h"

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
                if (auto next = matches[0].second + 1;
                        matches[0].second == end ||
                        (next != end && *next == '\n'))
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

        std::vector<id> get_input(const fs::path& input_dir) {
            std::ifstream in {input_dir / "2020" / "day_4_input.txt"};
            std::string contents {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
            auto retval = parse(contents);
            if (std::any_of(retval.begin(), retval.end(),
                            [](const id& i){ return std::any_of(i.fields.begin(), i.fields.end(),
                                                                [](const field& f){ return f.type == field_type::invalid; }); }))
            {
                throw std::runtime_error{"Invalid field name in data."};
            }
            return retval;
        }

    }

    /*
    You arrive at the airport only to realize that you grabbed your North Pole Credentials instead of your passport. While these documents are extremely similar, North Pole Credentials aren't issued by a country and therefore aren't actually valid documentation for travel in most of the world.

    It seems like you're not the only one having problems, though; a very long line has formed for the automatic passport scanners, and the delay could upset your travel itinerary.

    Due to some questionable network security, you realize you might be able to solve both of these problems at the same time.

    The automatic passport scanners are slow because they're having trouble detecting which passports have all required fields. The expected fields are as follows:

    byr (Birth Year)
    iyr (Issue Year)
    eyr (Expiration Year)
    hgt (Height)
    hcl (Hair Color)
    ecl (Eye Color)
    pid (Passport ID)
    cid (Country ID)
    Passport data is validated in batch files (your puzzle input). Each passport is represented as a sequence of key:value pairs separated by spaces or newlines. Passports are separated by blank lines.

    Here is an example batch file containing four passports:

    ecl:gry pid:860033327 eyr:2020 hcl:#fffffd
    byr:1937 iyr:2017 cid:147 hgt:183cm

    iyr:2013 ecl:amb cid:350 eyr:2023 pid:028048884
    hcl:#cfa07d byr:1929

    hcl:#ae17e1 iyr:2013
    eyr:2024
    ecl:brn pid:760753108 byr:1931
    hgt:179cm

    hcl:#cfa07d eyr:2025 pid:166559648
    iyr:2011 ecl:brn hgt:59in
    The first passport is valid - all eight fields are present. The second passport is invalid - it is missing hgt (the Height field).

    The third passport is interesting; the only missing field is cid, so it looks like data from North Pole Credentials, not a passport at all! Surely, nobody would mind if you made the system temporarily ignore missing cid fields. Treat this "passport" as valid.

    The fourth passport is missing two fields, cid and byr. Missing cid is fine, but missing any other field is not, so this passport is invalid.

    According to the above rules, your improved system would report 2 valid passports.

    Count the number of valid passports - those that have all required fields. Treat cid as optional. In your batch file, how many passports are valid?
     */
    std::string solve_day_4_1(const fs::path& input_dir) {
        auto ids = get_input(input_dir);
        auto num_valid = std::count_if(ids.begin(), ids.end(), [](const id& i){
            return i.is_valid(true, false);
        });
        return std::to_string(num_valid);
    }

    /*
    The line is moving more quickly now, but you overhear airport security talking about how passports with invalid data are getting through. Better add some data validation, quick!

    You can continue to ignore the cid field, but each other field has strict rules about what values are valid for automatic validation:

    byr (Birth Year) - four digits; at least 1920 and at most 2002.
    iyr (Issue Year) - four digits; at least 2010 and at most 2020.
    eyr (Expiration Year) - four digits; at least 2020 and at most 2030.
    hgt (Height) - a number followed by either cm or in:
    If cm, the number must be at least 150 and at most 193.
    If in, the number must be at least 59 and at most 76.
    hcl (Hair Color) - a # followed by exactly six characters 0-9 or a-f.
    ecl (Eye Color) - exactly one of: amb blu brn gry grn hzl oth.
    pid (Passport ID) - a nine-digit number, including leading zeroes.
    cid (Country ID) - ignored, missing or not.
    Your job is to count the passports where all required fields are both present and valid according to the above rules. Here are some example values:

    byr valid:   2002
    byr invalid: 2003

    hgt valid:   60in
    hgt valid:   190cm
    hgt invalid: 190in
    hgt invalid: 190

    hcl valid:   #123abc
    hcl invalid: #123abz
    hcl invalid: 123abc

    ecl valid:   brn
    ecl invalid: wat

    pid valid:   000000001
    pid invalid: 0123456789
    Here are some invalid passports:

    eyr:1972 cid:100
    hcl:#18171d ecl:amb hgt:170 pid:186cm iyr:2018 byr:1926

    iyr:2019
    hcl:#602927 eyr:1967 hgt:170cm
    ecl:grn pid:012533040 byr:1946

    hcl:dab227 iyr:2012
    ecl:brn hgt:182cm pid:021572410 eyr:2020 byr:1992 cid:277

    hgt:59cm ecl:zzz
    eyr:2038 hcl:74454a iyr:2023
    pid:3556412378 byr:2007
    Here are some valid passports:

    pid:087499704 hgt:74in ecl:grn iyr:2012 eyr:2030 byr:1980
    hcl:#623a2f

    eyr:2029 ecl:blu cid:129 byr:1989
    iyr:2014 pid:896056539 hcl:#a97842 hgt:165cm

    hcl:#888785
    hgt:164cm byr:2001 iyr:2015 cid:88
    pid:545766238 ecl:hzl
    eyr:2022

    iyr:2010 hgt:158cm hcl:#b6652a ecl:blu byr:1944 eyr:2021 pid:093154719
    Count the number of valid passports - those that have all required fields and valid values. Continue to treat cid as optional. In your batch file, how many passports are valid?
    */
    std::string solve_day_4_2(const fs::path& input_dir) {
        auto ids = get_input(input_dir);
        auto num_valid = std::count_if(ids.begin(), ids.end(), [](const id& i){
            return i.is_valid(true, true);
        });
        return std::to_string(num_valid);
    }

    static aoc::registration r {2020, 4, solve_day_4_1, solve_day_4_2};

} /* namespace aoc2020 */
