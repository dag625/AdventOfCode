cmake_minimum_required(VERSION 3.17)
project(AdventOfCode)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/onqtam/doctest.git
        GIT_TAG 2.4.2
)
FetchContent_MakeAvailable(doctest)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.9.1
)
FetchContent_MakeAvailable(nlohmann_json)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

find_package(OpenSSL REQUIRED)

add_executable(AdventOfCode
        main.cpp
        utilities.cpp
        utilities.h
        2020/day1.cpp
        2020/day2.cpp
        2020/day3.cpp
        2020/day4.cpp
        2020/day5.cpp
        2020/day6.cpp
        2020/day7.cpp
        2020/day8.cpp
        time_format.h
        2020/day9.cpp
        2020/day10.cpp
        2020/day11.cpp
        grid.cpp
        grid.h
        2020/day12.cpp
        2020/day13.cpp
        2020/day14.cpp
        2020/day15.cpp
        span.h
        arguments.h
        2020/day16.cpp
        2020/day17.cpp
        point.h
        point_nd.h
        2020/day18.cpp
        2020/day19.cpp
        2020/day20.cpp
        stride_span.h
        2020/day21.cpp
        2020/day22.cpp
        2020/day23.cpp
        challenge.cpp
        challenge.h
        registration.cpp
        registration.h
        2020/day24.cpp
        2020/day25.cpp
        2015/day1.cpp
        2015/day2.cpp
        utility_templates.h
        2015/day3.cpp
        hash_functions.cpp
        hash_functions.h
        2015/day4.cpp
        2015/day5.cpp
        2015/day6.cpp
        parse.h
        2015/day7.cpp
        2015/day8.cpp
        2015/day9.cpp
        2015/day10.cpp
        2015/day11.cpp
        2015/day12.cpp
        2015/day13.cpp)
target_include_directories(AdventOfCode PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(AdventOfCode PRIVATE doctest nlohmann_json OpenSSL::SSL)

enable_testing()
doctest_discover_tests(AdventOfCode)