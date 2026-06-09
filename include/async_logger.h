#pragma once

#include <memory>
#include <string>

#include "logger.h"
#include "thread_pool.h"
#include "blocking_queue.h"
#include "sink/sink.h"

namespace ljt
{
    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(std::string name, SinkPtr sink, std::size_t thread_num = 1);

        ~AsyncLogger() override;

        // 重写 Logger 核心日志接口，将消息入队而非同步写入
        void log(Level level, const std::string &msg,
                 const char* file, int line) override;

    private:
        // 工作线程循环：从队列取消息，格式化后写入 sink
        void workerLoop();

        BlockingQueue<LogMessage> queue_; // 阻塞队列
        ThreadPool pool_;                 // 线程池
        std::atomic<bool> running_{true}; // 运行标志
    };

    using AsyncLoggerPtr = std::shared_ptr<AsyncLogger>;
}
