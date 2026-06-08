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
        
        // 不同日志等级的输出 -> 快捷输出
        virtual void trace(const std::string &msg);
        virtual void debug(const std::string &msg);
        virtual void info(const std::string &msg);
        virtual void warn(const std::string &msg);
        virtual void error(const std::string &msg);
        virtual void critical(const std::string &msg);

    protected:
        // 子类可重写的核心日志接口 (NVI模式)
        virtual void log(Level level, const std::string &msg);

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