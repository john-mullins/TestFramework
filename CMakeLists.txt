cmake_minimum_required(VERSION 3.11)

project(testframework)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(Upstream_VERSION 1.0)
add_library(testframework INTERFACE)
add_library(testframework::testframework ALIAS testframework)

target_include_directories(testframework
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>
)

set(TARGETS_EXPORT_NAME "${PROJECT_NAME}_lib")

install(TARGETS testframework
    EXPORT ${TARGETS_EXPORT_NAME}
        INCLUDES DESTINATION .
)

install(EXPORT ${TARGETS_EXPORT_NAME}
    FILE ${TARGETS_EXPORT_NAME}.cmake
    NAMESPACE testframework::
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

set(HDR_FILES
        testframework/MiniTestFramework.h
        testframework/assertions.h
        testframework/stream_any.h
        testframework/testfailure.h
        testframework/streamfortestoutput.h
        testframework/TestHelpers.h
        testframework/testmain.inl
)

install(FILES
    ${HDR_FILES}
    DESTINATION testframework
)

configure_package_config_file(
    "cmake/testframeworkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/testframeworkConfig.cmake"
    INSTALL_DESTINATION lib/cmake/testframework
)

add_executable(testframework_tests)

target_link_libraries(testframework_tests
    PRIVATE
        testframework::testframework
)

target_sources(testframework_tests
    PRIVATE
        tests/main.cpp
        tests/stream_any_tests.cpp
        tests/testfailuretests.cpp
        tests/assertiontests.cpp

    ${HDR_FILES}
)
