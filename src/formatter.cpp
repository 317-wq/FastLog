#include "../include/formatter.h"

#include <iomanip>
#include <sstream>
#include <ctime>

namespace ljt
{
    const char* Formatter::defaultPattern()
    {
        return "[%Y-%m-%d %H:%M:%S][%^l][%t][%@] %v";
    }

    Formatter::Formatter()
        : pattern_(defaultPattern())
    {
    }

    Formatter::Formatter(const std::string &pattern)
        : pattern_(pattern)
    {
    }

    void Formatter::setPattern(const std::string &pattern)
    {
        pattern_ = pattern;
    }

    const std::string &Formatter::pattern() const
    {
        return pattern_;
    }

    std::string Formatter::levelToString(Level level, bool upper)
    {
        switch (level)
        {
        case Level::TRACE:    return upper ? "TRACE"    : "trace";
        case Level::DEBUG:    return upper ? "DEBUG"    : "debug";
        case Level::INFO:     return upper ? "INFO"     : "info";
        case Level::WARN:     return upper ? "WARN"     : "warn";
        case Level::ERROR:    return upper ? "ERROR"    : "error";
        case Level::CRITICAL: return upper ? "CRITICAL" : "critical";
        default:              return upper ? "UNKNOWN"  : "unknown";
        }
    }

    std::string Formatter::format(const LogMessage &msg) const
    {
        // 预转换时间戳，避免每个时间 token 重复转换
        auto tt = std::chrono::system_clock::to_time_t(msg.time);
        std::tm tm{};
        localtime_r(&tt, &tm);

        std::ostringstream oss;
        const auto &p = pattern_;

        for (std::size_t i = 0; i < p.size(); ++i)
        {
            if (p[i] == '%' && i + 1 < p.size())
            {
                char c = p[i + 1];

                // 处理 %^l（大写等级）
                if (c == '^' && i + 2 < p.size() && p[i + 2] == 'l')
                {
                    oss << levelToString(msg.level, true);
                    i += 2; // 跳过 ^l
                    continue;
                }

                // 单字符 token
                switch (c)
                {
                case 'Y': oss << std::put_time(&tm, "%Y"); break;
                case 'm': oss << std::put_time(&tm, "%m"); break;
                case 'd': oss << std::put_time(&tm, "%d"); break;
                case 'H': oss << std::put_time(&tm, "%H"); break;
                case 'M': oss << std::put_time(&tm, "%M"); break;
                case 'S': oss << std::put_time(&tm, "%S"); break;
                case 'l': oss << levelToString(msg.level, false); break;
                case 't': oss << msg.tid; break;
                case 'g': oss << msg.logger_name; break;
                case 'v': oss << msg.payload; break;
                case '@': // 源文件:行号
                    if (msg.source_file && msg.source_file[0] != '\0')
                    {
                        oss << "[" << msg.source_file << ":" << msg.source_line << "]";
                    }
                    break;
                case '%': oss << '%'; break;
                default:
                    // 无法识别的 token，原样输出
                    oss << '%' << c;
                    break;
                }

                ++i; // 跳过 token 字符
            }
            else
            {
                oss << p[i];
            }
        }

        return oss.str();
    }
}
