//
// Created by Daniel Garcia on 12/15/20.
//

#ifndef ADVENTOFCODE_ARGUMENTS_H
#define ADVENTOFCODE_ARGUMENTS_H

#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <variant>
#include <charconv>
#include <iostream>
#include <sstream>

namespace aoc {

    template <typename T, typename = void>
    struct argument_identity_parser {
        static T parse(std::string_view data) { return T(data); }
    };

    template <typename T>
    struct argument_identity_parser<T, std::enable_if_t<std::is_integral_v<T>, void>> {
        static T parse(std::string_view data) {
            T retval = 0;
            auto res = std::from_chars(data.data(), data.data() + data.size(), retval);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            return retval;
        }
    };

    struct argument_boolean_parser {
        static bool parse(std::string_view data) {
            using namespace std::string_view_literals;
            if (data.empty()) {
                return true;
            }
            else if (data == "true"sv) {
                return true;
            }
            else if (data == "TRUE"sv) {
                return true;
            }
            else if (data == "True"sv) {
                return true;
            }
            else if (data == "yes"sv) {
                return true;
            }
            else if (data == "YES"sv) {
                return true;
            }
            else if (data == "Yes"sv) {
                return true;
            }
            else if (data == "1"sv) {
                return true;
            }
            else if (data == "false"sv) {
                return false;
            }
            else if (data == "FALSE"sv) {
                return false;
            }
            else if (data == "False"sv) {
                return false;
            }
            else if (data == "no"sv) {
                return false;
            }
            else if (data == "NO"sv) {
                return false;
            }
            else if (data == "No"sv) {
                return false;
            }
            else if (data == "0"sv) {
                return false;
            }
            else {
                return true;
            }
        }
    };

    class argument_parser {
        struct arg {
            std::optional<std::string> long_name;
            std::optional<char> short_name;
            std::function<void(std::string_view)> parse_func;
            std::function<void()> not_present_func = [](){};
            bool is_required = false;
            bool was_argument = false;

            arg(std::string_view ln, std::function<void(std::string_view)> p, bool req = false) :
                    long_name{ln}, short_name{}, parse_func{std::move(p)}, is_required{req} {}
            arg(char sn, std::function<void(std::string_view)> p, bool req = false) :
                    long_name{}, short_name{sn}, parse_func{std::move(p)}, is_required{req} {}
            arg(char sn, std::string_view ln, std::function<void(std::string_view)> p, bool req = false) :
                    long_name{ln}, short_name{sn}, parse_func{std::move(p)}, is_required{req} {}

            bool operator()(char c) const { return short_name.has_value() && *short_name == c; }
            bool operator()(std::string_view s) const { return long_name.has_value() && *long_name == s; }

            arg& set_not_present_callback(std::function<void()> cb) { not_present_func = std::move(cb); return *this; }

            [[nodiscard]] bool check_not_present(bool do_throw) const {
                if (!was_argument) {
                    if (is_required) {
                        std::stringstream es;
                        es << "Missing required argument";
                        if (long_name) {
                            es << " '" << *long_name << "'.";
                        }
                        else if (short_name) {
                            es << " '" << *short_name << "'.";
                        }
                        if (do_throw) {
                            throw std::runtime_error{es.str()};
                        }
                    }
                    else {
                        not_present_func();
                    }
                    return true;
                }
                else {
                    return false;
                }
            }
        };
        std::vector<arg> m_arguments;
        int m_argc = 0;
        char** m_argv = nullptr;

        static std::pair<std::optional<std::variant<char, std::string_view>>, std::string_view> parse_arg(std::string_view arg) {
            using namespace std::string_view_literals;
            if (arg.find("--") == 0) {
                arg.remove_prefix(2);
                if (auto eq_pos = arg.find('='); eq_pos != std::string_view::npos) {
                    auto name = arg.substr(0, eq_pos), data = arg.substr(eq_pos + 1);
                    return {name, data};
                }
                else {
                    return {arg, {}};
                }
            }
            else if (arg.find('-') == 0) {
                arg.remove_prefix(1);
                if (arg.size() == 1) {
                    return {arg.front(), {}};
                }
                else if (arg.find('=') == 1) {
                    auto data = arg.substr(2);
                    return {arg.front(), data};
                }
                else {
                    return {std::nullopt, {}};
                }
            }
            else {
                return {std::nullopt, {}};
            }
        }

        void run(int argc, char** argv, bool do_throw, bool& error) {
            if (argv == nullptr) {
                return;
            }
            for (int i = 1; i < argc; ++i) {
                auto [name, value] = parse_arg(argv[i]);
                if (name) {
                    for (auto &arg : m_arguments) {
                        if (std::visit(arg, *name)) {
                            arg.was_argument = true;
                            try {
                                arg.parse_func(value);
                            }
                            catch (...) {
                                std::cerr << "Failed to parse command line argument:  " << argv[i] << '\n';
                            }
                        }
                    }
                }
            }
            bool tmp = false;
            for (const auto& arg : m_arguments) {
                auto check = arg.check_not_present(do_throw);
                tmp = tmp || check;
            }
            error = tmp;
        }

    public:
        argument_parser() = default;
        argument_parser(int argc, char** argv) : m_argc{argc}, m_argv{argv} {}

        argument_parser& add_flag(std::string_view arg, bool& value) {
            m_arguments.emplace_back(arg, [&value](std::string_view s){ value = argument_boolean_parser::parse(s); })
                .set_not_present_callback([&value](){ value = false; });
            return *this;
        }
        argument_parser& add_flag(char arg, bool& value) {
            m_arguments.emplace_back(arg, [&value](std::string_view s){ value = argument_boolean_parser::parse(s); })
                .set_not_present_callback([&value](){ value = false; });
            return *this;
        }
        argument_parser& add_flag(char arg1, std::string_view arg2, bool& value) {
            m_arguments.emplace_back(arg1, arg2, [&value](std::string_view s){ value = argument_boolean_parser::parse(s); })
                .set_not_present_callback([&value](){ value = false; });
            return *this;
        }

        template<typename T, typename Parser = argument_identity_parser<T>>
        argument_parser& add(std::string_view arg, T& value) {
            m_arguments.emplace_back(arg, [&value](std::string_view s){ value = Parser::parse(s); }, true);
            return *this;
        }
        template<typename T, typename Parser = argument_identity_parser<T>>
        argument_parser& add(char arg, T& value) {
            m_arguments.emplace_back(arg, [&value](std::string_view s){ value = Parser::parse(s); }, true);
            return *this;
        }
        template<typename T, typename Parser = argument_identity_parser<T>>
        argument_parser& add(char arg1, std::string_view arg2, T& value) {
            m_arguments.emplace_back(arg1, arg2, [&value](std::string_view s){ value = Parser::parse(s); }, true);
            return *this;
        }

        template<typename T, typename Parser = argument_identity_parser<T>>
        argument_parser& add_opt(std::string_view arg, std::optional<T>& value) {
            m_arguments.emplace_back(arg, [&value](std::string_view s){ value = Parser::parse(s); });
            return *this;
        }
        template<typename T, typename Parser = argument_identity_parser<T>>
        argument_parser& add_opt(char arg, std::optional<T>& value) {
            m_arguments.emplace_back(arg, [&value](std::string_view s){ value = Parser::parse(s); });
            return *this;
        }
        template<typename T, typename Parser = argument_identity_parser<T>>
        argument_parser& add_opt(char arg1, std::string_view arg2, std::optional<T>& value) {
            m_arguments.emplace_back(arg1, arg2, [&value](std::string_view s){ value = Parser::parse(s); });
            return *this;
        }

        void run(int argc, char** argv) {
            bool dummy;
            run(argc, argv, true, dummy);
        }

        void run() {
            bool dummy;
            run(m_argc, m_argv, true, dummy);
        }

        void run(bool& missing_required) {
            run(m_argc, m_argv, false, missing_required);
        }
    };

} /* namespace aoc */

aoc::argument_parser parse_arguments(int argc, char** argv) {
    return {argc, argv};
}

#endif //ADVENTOFCODE_ARGUMENTS_H
