cmake_minimum_required(VERSION 3.10)
project(FAST_LOG)

enable_testing()

find_package(GTest REQUIRED)

# add_executable(test_formatter
#     tests/test_formatter.cpp
#     src/formatter.cpp
# )

# target_include_directories(
#     test_formatter
#     PRIVATE include
# )

# target_link_libraries(
#     test_formatter
#     GTest::gtest
#     GTest::gtest_main
# )

# add_test(
#     NAME FormatterTest
#     COMMAND test_formatter
# )

# add_executable(test_sink
#     tests/test_sink.cpp
#     src/sink/file_sink.cpp
#     src/sink/stdout_sink.cpp
# )

# target_include_directories(
#     test_sink
#     PRIVATE include
# )

# target_link_libraries(
#     test_sink
#     GTest::gtest
#     GTest::gtest_main
# )

# add_test(
#     NAME SinkTest
#     COMMAND test_sink
# )

# add_executable(test_logger
#     tests/test_logger.cpp
#     src/logger.cpp
#     src/formatter.cpp
#     src/sink/file_sink.cpp
#     src/sink/stdout_sink.cpp
# )

# target_include_directories(
#     test_logger
#     PRIVATE include
# )

# target_link_libraries(
#     test_logger
#     GTest::gtest
#     GTest::gtest_main
# )

# add_test(
#     NAME LoggerTest
#     COMMAND test_logger
# )

# add_executable(test_logger_manager
#     tests/test_logger_manager.cpp
#     src/logger.cpp
#     src/formatter.cpp
#     src/sink/file_sink.cpp
#     src/sink/stdout_sink.cpp
#     src/logger_manager.cpp
# )

# target_include_directories(
#     test_logger_manager
#     PRIVATE include
# )

# target_link_libraries(
#     test_logger_manager
#     GTest::gtest
#     GTest::gtest_main
# )

# add_test(
#     NAME LoggerManagerTest
#     COMMAND test_logger_manager
# )

# add_executable(test_blocking_queue
#     tests/test_blocking_queue.cpp
#     src/logger.cpp
#     src/formatter.cpp
#     src/sink/file_sink.cpp
#     src/sink/stdout_sink.cpp
#     src/logger_manager.cpp
# )

# target_include_directories(
#     test_blocking_queue
#     PRIVATE include
# )

# target_link_libraries(
#     test_blocking_queue
#     GTest::gtest
#     GTest::gtest_main
#     pthread
# )

# add_test(
#     NAME LoggerBlockingQueueTest
#     COMMAND test_blocking_queue
# )

add_executable(test_thread_pool
    tests/test_thread_pool.cpp
    src/logger.cpp
    src/formatter.cpp
    src/sink/file_sink.cpp
    src/sink/stdout_sink.cpp
    src/logger_manager.cpp
    src/thread_pool.cpp
)

target_include_directories(
    test_thread_pool
    PRIVATE include
)

target_link_libraries(
    test_thread_pool
    GTest::gtest
    GTest::gtest_main
    pthread
)

add_test(
    NAME test_thread_pool_test
    COMMAND test_thread_pool
)