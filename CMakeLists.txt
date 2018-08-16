cmake_minimum_required(VERSION 3.11)

project(testframework)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECTS_ROOT $ENV{CMAKE_BUILD_ROOT})

add_library(testframework INTERFACE)

add_executable(testframework_tests)

target_include_directories(testframework
    INTERFACE
        .
)

target_link_libraries(testframework_tests
    PRIVATE
        testframework
)

target_sources(testframework_tests
    PRIVATE
        tests/main.cpp
        tests/stream_any_tests.cpp

        MiniTestFramework.h
        assertions.h
        stream_any.h
        testfailure.h
        streamfortestoutput.h
        TestHelpers.h
)
