cmake_minimum_required(VERSION 3.16)
project(lab1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB1_BUILD_TESTS "Build unit tests" ON)
option(LAB1_ENABLE_COVERAGE "Instrument code with gcov" OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
if(LAB1_ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_library(lab1_domain STATIC
    src/set/element.cpp
    src/set/cantor_set.cpp
    src/set/set_parser.cpp
    src/fifteen/fifteen_puzzle.cpp)
target_include_directories(lab1_domain PUBLIC include)

add_library(lab1_ui STATIC
    src/ui/console_io.cpp
    src/ui/menu.cpp
    src/ui/set_cli.cpp)
target_link_libraries(lab1_ui PUBLIC lab1_domain)

add_executable(set_app apps/set_main.cpp)
target_link_libraries(set_app PRIVATE lab1_ui)

if(LAB1_BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(lab1_tests
        tests/test_element.cpp
        tests/test_cantor_set.cpp
        tests/test_fifteen_puzzle.cpp
        tests/test_ui_basics.cpp
        tests/test_set_cli.cpp)
    target_link_libraries(lab1_tests PRIVATE lab1_ui GTest::gtest GTest::gtest_main)
    add_test(NAME lab1_tests COMMAND lab1_tests)
endif()
