# ============================================================
# FastLog 示例程序
#
# 编译所有示例：
#   cd build && cmake .. && make example_01 example_02 ...
# ============================================================

set(EXAMPLE_COMMON_SRC
    ../src/logger.cpp
    ../src/formatter.cpp
    ../src/sink/file_sink.cpp
    ../src/sink/stdout_sink.cpp
    ../src/sink/color_stdout_sink.cpp
    ../src/sink/rotating_file_sink.cpp
    ../src/sink/dist_sink.cpp
    ../src/flush_policy.cpp
    ../src/backtracer.cpp
    ../src/logger_manager.cpp
    ../src/default_logger.cpp
    ../src/async_logger.cpp
    ../src/thread_pool.cpp
)

add_executable(example_01 01_basic_usage.cpp       ${EXAMPLE_COMMON_SRC})
add_executable(example_02 02_async_logger.cpp      ${EXAMPLE_COMMON_SRC})
add_executable(example_03 03_custom_format.cpp     ${EXAMPLE_COMMON_SRC})
add_executable(example_04 04_colored_console.cpp   ${EXAMPLE_COMMON_SRC})
add_executable(example_05 05_rotating_file.cpp     ${EXAMPLE_COMMON_SRC})
add_executable(example_06 06_multi_sink.cpp        ${EXAMPLE_COMMON_SRC})
add_executable(example_07 07_flush_policy.cpp      ${EXAMPLE_COMMON_SRC})
add_executable(example_08 08_backtrace.cpp         ${EXAMPLE_COMMON_SRC})
add_executable(example_09 09_default_logger.cpp    ${EXAMPLE_COMMON_SRC})

foreach(exe
    example_01 example_02 example_03
    example_04 example_05 example_06
    example_07 example_08 example_09
)
    target_include_directories(${exe} PRIVATE ../include)
    target_link_libraries(${exe} pthread stdc++fs)
endforeach()
