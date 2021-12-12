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
    fs::path input_dir{"inputs"};//Leave this be to avoid having to change code everywhere, and to provide the required prefix.
    bool have_required = false, run_challenges = true;
    std::optional<int> year, day, chal;
    parse_arguments(argc, argv)
            .add_flag( "run"sv, run_challenges)
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

    aoc::challenges::list().run_all(year, day, chal, input_dir);
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
