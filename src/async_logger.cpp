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
                sink_->log(formatted);
            }
        }
    }

    void AsyncLogger::log(Level level, const std::string &msg, const char* file, int line)
    {
        if (level < getLevel())
            return;

        LogMessage log_msg;
        log_msg.level = level;
        log_msg.logger_name = name();
        log_msg.payload = msg;
        log_msg.time = std::chrono::system_clock::now();
        log_msg.tid = std::this_thread::get_id();
        log_msg.source_file = file;
        log_msg.source_line = line;

        queue_.push(log_msg);
    }
}
