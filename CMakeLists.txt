cmake_minimum_required(VERSION 3.17)
project(AdventOfCode)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenSSL REQUIRED)

include(doctest/cmake/doctest.cmake)

add_subdirectory("fmt-8.0.1")

add_executable(AdventOfCode
        main.cpp
        challenge.cpp
        challenge.h
        registration.cpp
        registration.h
        utilities.cpp
        utilities.h
        grid.cpp
        grid.h
        grid_algs.h
        hash_functions.cpp
        hash_functions.h
        utility_templates.h
        time_format.h
        ranges.h 
        sum.h
        span.h
        stride_span.h
        arguments.h 
        point.h
        point_nd.h
        parse.h
        2020/day1.cpp
        2020/day2.cpp
        2020/day3.cpp
        2020/day4.cpp
        2020/day5.cpp
        2020/day6.cpp
        2020/day7.cpp
        2020/day8.cpp
        2020/day9.cpp
        2020/day10.cpp
        2020/day11.cpp
        2020/day12.cpp
        2020/day13.cpp
        2020/day14.cpp
        2020/day15.cpp
        2020/day16.cpp
        2020/day17.cpp
        2020/day18.cpp
        2020/day19.cpp
        2020/day20.cpp
        2020/day21.cpp
        2020/day22.cpp
        2020/day23.cpp
        2020/day24.cpp
        2020/day25.cpp
        2015/day1.cpp
        2015/day2.cpp
        2015/day3.cpp
        2015/day4.cpp
        2015/day5.cpp
        2015/day6.cpp
        2015/day7.cpp
        2015/day8.cpp
        2015/day9.cpp
        2015/day10.cpp
        2015/day11.cpp
        2015/day12.cpp
        2015/day13.cpp 
        2015/day14.cpp 
        2021/day1.cpp 
        2021/day2.cpp
        2021/day3.cpp 
        2021/day4.cpp
        2021/day5.cpp
        2021/day6.cpp 
        2021/day7.cpp 
        2021/day8.cpp 
        2021/day9.cpp 
        2021/day10.cpp
        2021/day11.cpp
        2021/day12.cpp
        2021/day13.cpp
        2021/day14.cpp
        2021/day15.cpp
        2021/day16.cpp
        2021/day17.cpp
        2021/day18.cpp
        2021/day19.cpp
        2021/day20.cpp
        2021/day21.cpp
        2021/day22.cpp
        2021/day23.cpp
        2021/day24.cpp
        2021/day25.cpp
        2015/day15.cpp
        2015/day16.cpp
        2015/day17.cpp
        2015/day18.cpp
        2015/day19.cpp
        2015/day20.cpp
        2015/day21.cpp)
target_include_directories(AdventOfCode PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/boost/include")
target_compile_definitions(AdventOfCode PRIVATE BOOST_IOSTREAMS_NO_LIB)
target_link_libraries(AdventOfCode PRIVATE OpenSSL::SSL fmt)

#This is a fix for the 2021 day 19 compile time matrix generation.
if (MSVC)
    target_compile_options(AdventOfCode PRIVATE "/constexpr:steps67108864")
elseif(GCC)
    target_compile_options(AdventOfCode PRIVATE "-fconstexpr-ops-limit=67108864")
elseif(CLANG)
    target_compile_options(AdventOfCode PRIVATE "-fconstexpr-steps=67108864")
endif()

enable_testing()
doctest_discover_tests(AdventOfCode)