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

        // 不同日志等级的输出 -> 快捷输出
        void trace(const std::string &msg);
        void debug(const std::string &msg);
        void info(const std::string &msg);
        void warn(const std::string &msg);
        void error(const std::string &msg);
        void critical(const std::string &msg);

    private:
        // 自己根据level决定输出
        void log(Level level, const std::string &msg);

    public:
        // 设置过滤等级
        void setLevel(Level level);

        // 获取过滤等级
        Level getLevel() const;

    private:
        std::string name_; // 日志文件名字
        SinkPtr sink_; // 输出到哪里[基类] -> 运行时多态
        Formatter formatter_; // 格式化器
        std::atomic<Level> level_{Level::TRACE}; // 日志过滤等级
    };
}