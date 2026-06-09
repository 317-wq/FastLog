#include "../include/logger.h"

namespace ljt
{
    Logger::Logger(std::string name, SinkPtr sink)
        : name_(std::move(name)),
          sink_(std::move(sink))
    {
    }

    LogMessage Logger::buildLogMessage(Level level, const std::string &msg,
                                       const char* file, int line) const
    {
        LogMessage log_msg;
        log_msg.level = level;
        log_msg.logger_name = name_;
        log_msg.payload = msg;
        log_msg.time = std::chrono::system_clock::now();
        log_msg.tid = std::this_thread::get_id();
        log_msg.source_file = file;
        log_msg.source_line = line;
        return log_msg;
    }

    void Logger::log(Level level, const std::string &msg,
                     const char* file, int line)
    {
        // 过滤等级比较
        if (level < level_.load())
        {
            return;
        }

        auto log_msg = buildLogMessage(level, msg, file, line);

        // 回溯缓存（在格式化之前存入原始消息）
        backtracer_.push(log_msg);

        std::string formatted = formatter_.format(log_msg);

        sink_->log(level, formatted); // 运行时多态，传递等级供 Sink 差异化处理
    }

    void Logger::setLevel(Level level)
    {
        // 将参数赋值给原子变量
        level_.store(level /*, std::memory_order_relaxed*/);
    }

    Level Logger::getLevel() const
    {
        // 获取原子变量的值
        return level_.load(/*std::memory_order_relaxed*/);
    }

    const std::string& Logger::name() const
    {
        return name_;
    }

    void Logger::flush()
    {
        sink_->flush();
    }

    void Logger::setPattern(const std::string &pattern)
    {
        formatter_.setPattern(pattern);
    }

    void Logger::enableBacktrace(std::size_t size)
    {
        backtracer_.enable(size);
    }

    void Logger::disableBacktrace()
    {
        backtracer_.disable();
    }

    void Logger::dumpBacktrace()
    {
        backtracer_.forEach([this](const LogMessage &msg)
        {
            auto formatted = formatter_.format(msg);
            sink_->log(msg.level, formatted);
        });
    }

    void Logger::pushBacktrace(const LogMessage &msg)
    {
        backtracer_.push(msg);
    }
}
