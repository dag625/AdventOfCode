cmake_minimum_required(VERSION 3.17)
project(AdventOfCode)

set(CMAKE_CXX_STANDARD 17)

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
        time_format.h 2020/day9.cpp 2020/day9.h 2020/day10.cpp 2020/day10.h 2020/day11.cpp 2020/day11.h grid.cpp grid.h 2020/day12.cpp 2020/day12.h 2020/day13.cpp 2020/day13.h)
target_include_directories(AdventOfCode PRIVATE ${CMAKE_CURRENT_LIST_DIR})