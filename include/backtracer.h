#pragma once

#include "circular_queue.h"
#include "log_message.h"

#include <atomic>
#include <memory>
#include <mutex>

namespace ljt
{
    /// 日志回溯器
    ///
    /// 启用后用一个环形缓冲区缓存最近 N 条日志消息。
    /// 当发生 ERROR 时，调用 dump() 将历史日志全部输出，
    /// 解决 crash 前日志来不及落盘的问题。
    ///
    /// 使用示例：
    ///   logger.enableBacktrace(32);   // 缓存最近 32 条
    ///   // ... 正常日志 ...
    ///   LOG_ERROR(logger, "something wrong");
    ///   logger.dumpBacktrace();       // 输出 ERROR 前的上下文日志

    class Backtracer
    {
    public:
        Backtracer() = default;

        /// 启用回溯，设置缓冲区容量
        void enable(std::size_t size);

        /// 禁用回溯
        void disable();

        /// 是否已启用
        bool enabled() const;

        /// 存入一条日志消息
        void push(const LogMessage &msg);

        /// 将缓冲区中所有日志取出，逐一交给回调处理（不会清空队列）
        template <typename Func>
        void forEach(Func &&func)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            while (!messages_.empty())
            {
                func(messages_.front());
                messages_.pop_front();
            }
        }

    private:
        mutable std::mutex mutex_;
        std::atomic<bool> enabled_{false};
        CircularQueue<LogMessage> messages_;
    };

    using BacktracerPtr = std::shared_ptr<Backtracer>;
}
