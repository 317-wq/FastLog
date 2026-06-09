#include "../include/async_logger.h"
#include <thread>

namespace ljt
{

    AsyncLogger::AsyncLogger(std::string name, SinkPtr sink, std::size_t thread_num)
        : Logger(std::move(name), sink),
          pool_(thread_num)
    {
        pool_.start();

        // 启动每个线程执行 workerLoop
        for (std::size_t i = 0; i < thread_num; ++i)
        {
            pool_.submit([this]()
                         { workerLoop(); });
        }
    }

    AsyncLogger::~AsyncLogger()
    {
        running_ = false;
        queue_.stop();
        pool_.stop();
    }

    void AsyncLogger::workerLoop()
    {
        LogMessage msg;

        while (running_ || !queue_.empty())
        {
            if (queue_.pop(msg))
            {
                auto formatted = formatter_.format(msg);
                sink_->log(msg.level, formatted);
            }
        }
    }

    void AsyncLogger::log(Level level, const std::string &msg, const char* file, int line)
    {
        if (level < getLevel())
            return;

        auto log_msg = buildLogMessage(level, msg, file, line);

        // 回溯缓存
        pushBacktrace(log_msg);

        queue_.push(std::move(log_msg));
    }
}
