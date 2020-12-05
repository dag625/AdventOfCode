cmake_minimum_required(VERSION 3.17)
project(AdventOfCode2020)

set(CMAKE_CXX_STANDARD 17)

add_executable(AdventOfCode2020
        main.cpp
        day1.cpp
        day1.h
        utilities.cpp
        utilities.h
        day2.cpp
        day2.h
        day3.cpp
        day3.h day4.cpp day4.h day5.cpp day5.h)