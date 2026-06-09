#include "../../include/sink/dist_sink.h"

#include <algorithm>

namespace ljt
{
    DistSink::DistSink(std::vector<SinkPtr> sinks)
        : sinks_(std::move(sinks))
    {
    }

    void DistSink::log(Level level, const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &s : sinks_)
        {
            s->log(level, message);
        }
    }

    void DistSink::log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &s : sinks_)
        {
            s->log(message);
        }
    }

    void DistSink::flush()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &s : sinks_)
        {
            s->flush();
        }
    }

    void DistSink::addSink(SinkPtr sink)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.push_back(std::move(sink));
    }

    void DistSink::removeSink(const SinkPtr &sink)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.erase(
            std::remove(sinks_.begin(), sinks_.end(), sink),
            sinks_.end());
    }

    std::vector<SinkPtr> DistSink::sinks() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return sinks_;
    }
}
