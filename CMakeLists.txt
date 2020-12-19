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

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

add_executable(AdventOfCode
        main.cpp
        utilities.cpp
        utilities.h
        2020/day1.cpp
        2020/day1.h
        2020/day2.cpp
        2020/day2.h
        2020/day3.cpp
        2020/day3.h
        2020/day4.cpp
        2020/day4.h
        2020/day5.cpp
        2020/day5.h
        2020/day6.cpp
        2020/day6.h
        2020/day7.cpp
        2020/day7.h
        2020/day8.cpp
        2020/day8.h
        time_format.h
        2020/day9.cpp
        2020/day9.h
        2020/day10.cpp
        2020/day10.h
        2020/day11.cpp
        2020/day11.h
        grid.cpp
        grid.h
        2020/day12.cpp
        2020/day12.h
        2020/day13.cpp
        2020/day13.h
        2020/day14.cpp
        2020/day14.h
        2020/day15.cpp
        2020/day15.h span.h arguments.h 2020/day16.cpp 2020/day16.h 2020/day17.cpp 2020/day17.h point.h point_nd.h 2020/day18.cpp 2020/day18.h)
target_include_directories(AdventOfCode PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(AdventOfCode PRIVATE doctest)

enable_testing()
doctest_discover_tests(AdventOfCode)