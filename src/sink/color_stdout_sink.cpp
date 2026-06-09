#include "../../include/sink/color_stdout_sink.h"

#include <iostream>

namespace ljt
{
    const char* ColorStdoutSink::colorCode(Level level)
    {
        switch (level)
        {
        case Level::TRACE:    return "\033[37m";        // 白色
        case Level::DEBUG:    return "\033[36m";        // 青色
        case Level::INFO:     return "\033[32m";        // 绿色
        case Level::WARN:     return "\033[33m";        // 黄色
        case Level::ERROR:    return "\033[31m";        // 红色
        case Level::CRITICAL: return "\033[41;37m";     // 红底白字
        default:              return "\033[0m";         // 默认重置
        }
    }

    void ColorStdoutSink::log(Level level, const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << colorCode(level) << message << "\033[0m" << '\n';
    }

    void ColorStdoutSink::log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << message << '\n';
    }

    void ColorStdoutSink::flush()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout.flush();
    }
}
