#pragma once

#include "sink.h"
#include <mutex>

namespace ljt
{
    class StdoutSink : public Sink
    {
    public:
        // 输出日志
        void log(const std::string &message) override;
        // 刷新输出缓冲区
        void flush() override;

    private:
        std::mutex mutex_; // 保护输出锁
    };

}