#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "backtracer.h"
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

        // 刷新 Sink 缓冲区
        void flush();

        // 设置日志格式 pattern（透传给 Formatter）
        void setPattern(const std::string &pattern);

        // ---- 回溯功能 ----

        /// 启用回溯，缓存最近 N 条日志消息
        void enableBacktrace(std::size_t size);

        /// 禁用回溯
        void disableBacktrace();

        /// 将缓存的回溯日志全部输出到当前 Sink 并清空
        void dumpBacktrace();

    protected:
        // 构建 LogMessage（供子类复用，消除重复代码）
        LogMessage buildLogMessage(Level level, const std::string &msg,
                                   const char* file, int line) const;

        // 推送至 backtracer（供子类复用）
        void pushBacktrace(const LogMessage &msg);

        std::string name_;                       // logger_name
        SinkPtr sink_;                           // 输出到哪里[基类] -> 运行时多态
        Formatter formatter_;                    // 格式化器
        std::atomic<Level> level_{Level::TRACE}; // 日志过滤等级
        Backtracer backtracer_;                  // 回溯缓存
    };

    using LoggerPtr = std::shared_ptr<Logger>;
}
