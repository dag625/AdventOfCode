//
// Created by Daniel Garcia on 12/16/20.
//

#include "registration.h"
#include "utilities.h"
#include "grid.h"
#include "span.h"

#include <iostream>
#include <vector>
#include <charconv>
#include <numeric>
#include <array>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        struct field_range {
            int min;
            int max;

            [[nodiscard]] bool matches(int val) const noexcept { return val >= min && val <= max; }
        };

        struct field_constraints {
            std::string name;
            std::array<field_range, 2> ranges;

            [[nodiscard]] bool matches(int val) const noexcept {
                return ranges[0].matches(val) || ranges[1].matches(val);
            }
        };

        struct input {
            std::vector<field_constraints> fields;
            aoc::grid<int> tickets;

            [[nodiscard]] std::vector<field_constraints>::const_iterator matching_fields(int val, std::size_t pos = 0) const noexcept {
                return std::find_if(fields.begin() + pos, fields.end(), [val](const field_constraints& f){ return f.matches(val); });
            }

            [[nodiscard]] bool any_matching_fields(int val, std::size_t pos = 0) const noexcept {
                return matching_fields(val) != fields.end();
            }

            [[nodiscard]] bool all_fields_valid(const aoc::span<int>& ticket) const noexcept {
                return std::all_of(ticket.begin(), ticket.end(), [this](int t){ return any_matching_fields(t); });
            }
        };

        int parse_int(std::string_view s) {
            int val = 0;
            auto res = std::from_chars(s.data(), s.data() + s.size(), val);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            return val;
        }

        std::vector<int> parse_ticket(std::string_view line) {
            auto els = aoc::split(line, ',');
            std::vector<int> retval;
            retval.reserve(els.size());
            std::transform(els.begin(), els.end(), std::back_inserter(retval), parse_int);
            return retval;
        }

        field_range parse_range(std::string_view r) {
            auto parts = aoc::split(r, '-');
            if (parts.size() != 2) {
                throw std::runtime_error{"Range must have minimum and maximum."};
            }
            return {parse_int(parts[0]), parse_int(parts[1])};
        }

        field_constraints parse_field(std::string_view line) {
            auto parts = aoc::split(line, ':');
            if (parts.size() != 2) {
                throw std::runtime_error{"Field must have name and ranges list."};
            }
            auto rs = aoc::split(aoc::trim(parts[1]), " or ");
            if (rs.size() != 2) {
                throw std::runtime_error{"Field must have two valid ranges."};
            }
            return {std::string{parts[0]}, {parse_range(rs[0]), parse_range(rs[1])}};
        }

        input get_input(const fs::path &input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_16_input.txt");
            const auto fields_end = std::find_if(lines.begin(), lines.end(), [](const std::string& s){ return s.empty(); });
            if (fields_end == lines.end()) {
                throw std::runtime_error{"No break between the fields list and tickets list found."};
            }
            std::vector<field_constraints> fields;
            fields.reserve(lines.size());
            std::transform(lines.begin(), fields_end, std::back_inserter(fields), parse_field);

            lines.erase(std::remove_if(fields_end + 1, lines.end(),
                                       [](const std::string& s){ return !s.empty() && !isdigit(s.front()); }),
                        lines.end());

            std::vector<std::vector<int>> tickets;
            std::transform(fields_end, lines.end(), std::back_inserter(tickets), parse_ticket);
            tickets.erase(std::remove_if(tickets.begin(), tickets.end(),
                                         [](const std::vector<int>& v){ return v.empty(); }),
                          tickets.end());
            if (tickets.empty() || std::any_of(tickets.begin(), tickets.end(), [
                    &tickets](const std::vector<int>& t){ return t.size() != tickets.front().size(); }))
            {
                throw std::runtime_error{"Either there are no tickets or tickets have different numbers of fields."};
            }
            std::vector<int> all_tickets;
            all_tickets.reserve(tickets.size() * tickets.front().size());
            for (const auto& t : tickets) {
                all_tickets.insert(all_tickets.end(), t.begin(), t.end());
            }
            return {fields, {std::move(all_tickets), tickets.front().size()}};
        }

        std::vector<std::string_view> flatten_columns(const std::vector<std::vector<std::string_view>>& names) noexcept {
            try {
                using pair = std::pair<std::vector<std::string_view>, std::size_t>;
                std::vector<pair> with_indexes;
                with_indexes.reserve(names.size());
                for (std::size_t idx = 0; idx < names.size(); ++idx) {
                    with_indexes.emplace_back(names[idx], idx);
                }
                auto sort_by_size = [](const pair& a, const pair& b){ return a.first.size() < b.first.size(); };
                for (int i = 0; i < with_indexes.size(); ++i) {
                    std::stable_sort(with_indexes.begin() + i, with_indexes.end(), sort_by_size);
                    for (int j = i + 1; j < with_indexes.size(); ++j) {
                        with_indexes[j].first.erase(
                                std::remove(with_indexes[j].first.begin(), with_indexes[j].first.end(), with_indexes[i].first.front()),
                                with_indexes[j].first.end());
                    }
                }
                std::sort(with_indexes.begin(), with_indexes.end(), [](const pair& a, const pair& b){ return a.second < b.second; });
                std::vector<std::string_view> retval;
                std::transform(with_indexes.begin(), with_indexes.end(), std::back_inserter(retval), [](const pair& v){ return v.first.front(); });
                return retval;
            }
            catch (...) {
                return {};
            }
        }

        void print(const std::vector<std::vector<std::string_view>>& names) {
            for (int i = 0; i < names.size(); ++i) {
                std::cout << "Col " << std::setw(4) << std::setfill(' ') << i <<
                        " (" << std::setw(4) << std::setfill(' ') << names[i].size() << "):  ";
                bool first = true;
                for (const auto n : names[i]) {
                    if (!first) {
                        std::cout << ", ";
                    }
                    first = false;
                    std::cout << n;
                }
                std::cout << '\n';
            }
        }

    }

    /*
    As you're walking to yet another connecting flight, you realize that one of the legs of your re-routed trip coming up is on a high-speed train. However, the train ticket you were given is in a language you don't understand. You should probably figure out what it says before you get to the train station after the next flight.

    Unfortunately, you can't actually read the words on the ticket. You can, however, read the numbers, and so you figure out the fields these tickets must have and the valid ranges for values in those fields.

    You collect the rules for ticket fields, the numbers on your ticket, and the numbers on other nearby tickets for the same train service (via the airport security cameras) together into a single document you can reference (your puzzle input).

    The rules for ticket fields specify a list of fields that exist somewhere on the ticket and the valid ranges of values for each field. For example, a rule like class: 1-3 or 5-7 means that one of the fields in every ticket is named class and can be any value in the ranges 1-3 or 5-7 (inclusive, such that 3 and 5 are both valid in this field, but 4 is not).

    Each ticket is represented by a single line of comma-separated values. The values are the numbers on the ticket in the order they appear; every ticket has the same format. For example, consider this ticket:

    .--------------------------------------------------------.
    | ????: 101    ?????: 102   ??????????: 103     ???: 104 |
    |                                                        |
    | ??: 301  ??: 302             ???????: 303      ??????? |
    | ??: 401  ??: 402           ???? ????: 403    ????????? |
    '--------------------------------------------------------'
    Here, ? represents text in a language you don't understand. This ticket might be represented as 101,102,103,104,301,302,303,401,402,403; of course, the actual train tickets you're looking at are much more complicated. In any case, you've extracted just the numbers in such a way that the first number is always the same specific field, the second number is always a different specific field, and so on - you just don't know what each position actually means!

    Start by determining which tickets are completely invalid; these are tickets that contain values which aren't valid for any field. Ignore your ticket for now.

    For example, suppose you have the following notes:

    class: 1-3 or 5-7
    row: 6-11 or 33-44
    seat: 13-40 or 45-50

    your ticket:
    7,1,14

    nearby tickets:
    7,3,47
    40,4,50
    55,2,20
    38,6,12
    It doesn't matter which position corresponds to which field; you can identify invalid nearby tickets by considering only whether tickets contain values that are not valid for any field. In this example, the values on the first nearby ticket are all valid for at least one field. This is not true of the other three nearby tickets: the values 4, 55, and 12 are are not valid for any field. Adding together all of the invalid values produces your ticket scanning error rate: 4 + 55 + 12 = 71.

    Consider the validity of the nearby tickets you scanned. What is your ticket scanning error rate?
    */
    std::string solve_day_16_1(const std::filesystem::path& input_dir) {
        auto in = get_input(input_dir);
        int64_t sum = 0;
        for (auto tf : in.tickets) {
            auto a = tf;
            for (const auto& f : in.fields) {
                if (f.matches(tf)) {
                    a = 0;
                    break;
                }
            }
            sum += a;
        }
        return std::to_string(sum);
    }

    /*
    Now that you've identified which tickets contain invalid values, discard those tickets entirely. Use the remaining valid tickets to determine which field is which.

    Using the valid ranges for each field, determine what order the fields appear on the tickets. The order is consistent between all tickets: if seat is the third field, it is the third field on every ticket, including your ticket.

    For example, suppose you have the following notes:

    class: 0-1 or 4-19
    row: 0-5 or 8-19
    seat: 0-13 or 16-19

    your ticket:
    11,12,13

    nearby tickets:
    3,9,18
    15,1,5
    5,14,9
    Based on the nearby tickets in the above example, the first position must be row, the second position must be class, and the third position must be seat; you can conclude that in your ticket, class is 12, row is 11, and seat is 13.

    Once you work out which field is which, look for the six fields on your ticket that start with the word departure. What do you get if you multiply those six values together?
    */
    std::string solve_day_16_2(const std::filesystem::path& input_dir) {
        using namespace std::string_view_literals;
        auto in = get_input(input_dir);
        for (std::size_t row = 0; row < in.tickets.num_rows();) {
            if (!in.all_fields_valid(in.tickets[row])) {
                in.tickets.erase_row(row);
            }
            else {
                ++row;
            }
        }
        std::vector<std::vector<std::string_view>> names;
        for (std::size_t c = 0; c < in.tickets.num_cols(); ++c) {
            auto col = in.tickets.column(c);
            std::vector<std::string_view> valid;
            for (const auto& f : in.fields) {
                if (std::all_of(col.begin(), col.end(), [&f](int v){ return f.matches(v); })) {
                    valid.emplace_back(f.name);
                }
            }
            names.push_back(std::move(valid));
        }
        auto col_names = flatten_columns(names);
        int64_t acc = 1;
        for (int i = 0; i < col_names.size(); ++i) {
            if (col_names[i].find("departure"sv) == 0) {
                acc *= in.tickets[0][i];
            }
        }
        return std::to_string(acc);
    }

    static aoc::registration r {2020, 16, solve_day_16_1, solve_day_16_2};

} /* namespace aoc2020 */