#pragma once

#include <chrono>
#include <string>
#include <thread>

#include "level.h"

namespace ljt
{
    struct LogMessage
    {
        Level level;                                // 日志等级
        std::string logger_name;                    // 日志名字
        std::string payload;                        // 有效载荷
        std::chrono::system_clock::time_point time; // 时刻[系统时间]
        std::thread::id tid;                        // 线程id
        const char* source_file{""};                // 源文件名
        int source_line{0};                         // 源文件行号
    };
}