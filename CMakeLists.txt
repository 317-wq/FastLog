cmake_minimum_required(VERSION 3.10)
project(FAST_LOG)

enable_testing()

find_package(GTest REQUIRED)

add_executable(test_formatter
    tests/test_formatter.cpp
    src/formatter.cpp
)

target_include_directories(
    test_formatter
    PRIVATE include
)

target_link_libraries(
    test_formatter
    GTest::gtest
    GTest::gtest_main
)

add_test(
    NAME FormatterTest
    COMMAND test_formatter
)

add_executable(test_sink
    tests/test_sink.cpp
    src/file_sink.cpp
    src/stdout_sink.cpp
)

target_include_directories(
    test_sink
    PRIVATE include
)

target_link_libraries(
    test_sink
    GTest::gtest
    GTest::gtest_main
)

add_test(
    NAME SinkTest
    COMMAND test_sink
)