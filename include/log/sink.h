#pragma once

#include <memory>
#include <string>

// 日志输出器

namespace ljt
{
    class Sink
    {
    public:
        virtual ~Sink() = default;
        // 输出日志
        virtual void log(const std::string &message) = 0;
        // 刷新输出缓冲区
        virtual void flush() = 0;
    };

    using SinkPtr = std::shared_ptr<Sink>;
}