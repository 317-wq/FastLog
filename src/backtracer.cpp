#include "../include/backtracer.h"

namespace ljt
{
    void Backtracer::enable(std::size_t size)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        messages_ = CircularQueue<LogMessage>(size);
        enabled_.store(true, std::memory_order_relaxed);
    }

    void Backtracer::disable()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_.store(false, std::memory_order_relaxed);
    }

    bool Backtracer::enabled() const
    {
        return enabled_.load(std::memory_order_relaxed);
    }

    void Backtracer::push(const LogMessage &msg)
    {
        if (!enabled_.load(std::memory_order_relaxed))
        {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        messages_.push_back(msg);
    }
}
