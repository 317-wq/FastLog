#include "../include/formatter.h"

#include <iomanip>
#include <sstream>

namespace ljt
{
    std::string Formatter::levelToString(Level level)
    {
        switch (level)
        {
        case Level::TRACE:
            return "TRACE";
        case Level::DEBUG:
            return "DEBUG";
        case Level::INFO:
            return "INFO";
        case Level::WARN:
            return "WARN";
        case Level::ERROR:
            return "ERROR";
        case Level::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
        }
    }

    std::string Formatter::format(const LogMessage &msg)
    {
        // 时间戳转换为 年 月 日 时 分 秒
        auto tt = std::chrono::system_clock::to_time_t(msg.time);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        std::ostringstream oss;
        oss
            << "["
            << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << "]"
            << "[" << levelToString(msg.level) << "]"
            << "[" << msg.tid << "]"
            << " "
            << msg.payload;
        return oss.str();
    }
}