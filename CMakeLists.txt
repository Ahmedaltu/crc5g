cmake_minimum_required(VERSION 3.14)
project(crc5g C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-O3 -march=native)

# CRC library
add_library(crc src/crc.c src/crc_table.c src/crc_ilp.c)
target_include_directories(crc PUBLIC src/)

# GTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Unit tests
enable_testing()
add_executable(test_crc
    tests/test_crc.cpp
    tests/test_crc_table.cpp
    tests/test_crc_ilp.cpp
)
target_link_libraries(test_crc crc GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_crc)

# Benchmark
add_executable(bench_crc tests/bench_crc.cpp)
target_link_libraries(bench_crc crc)
