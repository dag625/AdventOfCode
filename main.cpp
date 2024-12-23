#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "arguments.h"
#include "challenge.h"

namespace fs = std::filesystem;

int non_test_main(int argc, char** argv) {
    using namespace std::string_view_literals;
    fs::path input_dir;
    bool have_required = false, run_challenges = true, use_markdown_output_fmt = false, hide_answers = false;
    std::optional<int> year, day, chal;
    parse_arguments(argc, argv)
            .add('i', "input_dir"sv, input_dir)
            .add_flag( "run"sv, run_challenges)
            .add_flag( "md"sv, use_markdown_output_fmt)
            .add_flag('x', "hide"sv, hide_answers)
            .add_opt("year"sv, year)
            .add_opt("day"sv, day)
            .add_opt("challenge"sv, chal)
            .run(have_required);

    if (!have_required) {
        std::cerr << "Missing required arguments.\n";
        return 1;
    }
    if (!run_challenges) {
        return 0;
    }

    aoc::challenges::list().run_all(year, day, chal, input_dir, use_markdown_output_fmt, hide_answers);
    return 0;
}

int main(int argc, char** argv) {
    try {
        doctest::Context ctxt {argc, argv};
        int retval = ctxt.run();

        if(ctxt.shouldExit()) {
            return retval;
        }

        return retval + non_test_main(argc, argv); // combine the 2 results
    }
    catch (const std::exception& e) {
        std::cerr << "Exception:  " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception." << std::endl;
        return 1;
    }
}
