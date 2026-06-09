#pragma once

#include "sink/sink.h"
#include "level.h"

#include <chrono>
#include <memory>
#include <mutex>

namespace ljt
{
    /// Flush 策略包装器（装饰器模式）
    ///
    /// 两种触发方式：
    ///   1. 等级触发 — 当日志等级 >= flush_on 时自动 flush
    ///   2. 定时触发 — 每隔 interval_ms 毫秒自动 flush（在 log() 调用时检查）
    ///
    /// 使用示例：
    ///   auto sink = std::make_shared<FileSink>("app.log");
    ///   FlushPolicy policy(sink, Level::ERROR, std::chrono::seconds(3));

    class FlushPolicy : public Sink
    {
    public:
        /// @param sink         被包装的实际 Sink
        /// @param flush_on     达到此等级及以上自动触发 flush（默认 ERROR）
        /// @param interval_ms  定时 flush 间隔，0 表示禁用（默认禁用）
        explicit FlushPolicy(SinkPtr sink,
                             Level flush_on = Level::ERROR,
                             std::chrono::milliseconds interval_ms = std::chrono::milliseconds::zero());

        // 带等级的输出（根据等级判断是否 flush）
        void log(Level level, const std::string &message) override;

        // 无等级的输出（直接委托，不触发等级 flush）
        void log(const std::string &message) override;

        // 强制刷新
        void flush() override;

    private:
        // 检查是否需要定时 flush
        void checkPeriodicFlush();

        SinkPtr sink_;
        Level flush_on_;
        std::chrono::milliseconds interval_ms_;

        using Clock = std::chrono::steady_clock;
        Clock::time_point last_flush_;
        std::mutex mutex_;
    };
}
