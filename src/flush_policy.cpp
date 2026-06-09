#include "../include/flush_policy.h"

namespace ljt
{
    FlushPolicy::FlushPolicy(SinkPtr sink,
                             Level flush_on,
                             std::chrono::milliseconds interval_ms)
        : sink_(std::move(sink)),
          flush_on_(flush_on),
          interval_ms_(interval_ms),
          last_flush_(Clock::now())
    {
    }

    void FlushPolicy::log(Level level, const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        sink_->log(level, message);

        // 等级触发：达到阈值自动 flush
        if (level >= flush_on_)
        {
            sink_->flush();
            last_flush_ = Clock::now(); // 重置定时器
            return;
        }

        // 定时触发检查
        checkPeriodicFlush();
    }

    void FlushPolicy::log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        sink_->log(message);

        checkPeriodicFlush();
    }

    void FlushPolicy::flush()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sink_->flush();
        last_flush_ = Clock::now();
    }

    void FlushPolicy::checkPeriodicFlush()
    {
        if (interval_ms_.count() == 0)
        {
            return;
        }

        auto now = Clock::now();
        if (now - last_flush_ >= interval_ms_)
        {
            sink_->flush();
            last_flush_ = now;
        }
    }
}
