cmake_minimum_required(VERSION 3.11)

project(testframework)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(Upstream_VERSION 1.0)

add_library(testframework INTERFACE)

target_include_directories(testframework
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>
)

install(TARGETS testframework
    EXPORT testframework_lib
        INCLUDES DESTINATION .
)

install(EXPORT testframework_lib
    FILE testframework_lib.cmake
    DESTINATION lib/cmake/testframework
)


include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/testframeworkConfigVersion.cmake"
    VERSION ${Upstream_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/testframeworkConfigVersion.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/testframeworkConfig.cmake"
        DESTINATION lib/cmake/testframework
)

install(FILES
        ${PROJECT_SOURCE_DIR}/testframework/MiniTestFramework.h
        ${PROJECT_SOURCE_DIR}/testframework/assertions.h
        ${PROJECT_SOURCE_DIR}/testframework/stream_any.h
        ${PROJECT_SOURCE_DIR}/testframework/testfailure.h
        ${PROJECT_SOURCE_DIR}/testframework/streamfortestoutput.h
        ${PROJECT_SOURCE_DIR}/testframework/TestHelpers.h
    DESTINATION testframework
)

configure_package_config_file(
    "${CMAKE_CURRENT_BINARY_DIR}/testframeworkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/testframeworkConfig.cmake"
    INSTALL_DESTINATION lib/cmake/testframework
)

add_executable(testframework_tests)

target_link_libraries(testframework_tests
    PRIVATE
        testframework
)

target_sources(testframework_tests
    PRIVATE
        tests/main.cpp
        tests/stream_any_tests.cpp
        tests/testfailuretests.cpp
        tests/assertiontests.cpp

        testframework/MiniTestFramework.h
        testframework/assertions.h
        testframework/stream_any.h
        testframework/testfailure.h
        testframework/streamfortestoutput.h
        testframework/TestHelpers.h
)
