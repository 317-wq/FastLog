#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "formatter.h"
#include "log_message.h"
#include "sink/sink.h"

namespace ljt
{
    class Logger
    {
    public:
        Logger(std::string name, SinkPtr sink);

        virtual ~Logger() = default;

        // ---- 唯一日志入口：必须带 file/line，由宏 LOG_*() 自动注入 ----
        // 使用方式: LOG_TRACE / LOG_DEBUG / LOG_INFO / LOG_WARN / LOG_ERROR / LOG_CRITICAL
        virtual void log(Level level, const std::string &msg, const char* file, int line);

    public:
        // 设置过滤等级
        void setLevel(Level level);

        // 获取过滤等级
        Level getLevel() const;

        // 获取日志名字
        const std::string &name() const;

    protected:
        std::string name_;                       // 日志文件名字
        SinkPtr sink_;                           // 输出到哪里[基类] -> 运行时多态
        Formatter formatter_;                    // 格式化器
        std::atomic<Level> level_{Level::TRACE}; // 日志过滤等级
    };

    using LoggerPtr = std::shared_ptr<Logger>;
}