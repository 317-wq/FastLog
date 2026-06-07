#include "../include/logger.h"

namespace ljt
{
    Logger::Logger(std::string name, SinkPtr sink)
        : name_(std::move(name)),
          sink_(std::move(sink))
    {
    }

    void Logger::log(Level level, const std::string &msg)
    {
        // 过滤等级比较
        if (level < level_.load())
        {
            return;
        }

        LogMessage log_msg;
        log_msg.level = level;                           // 日志等级
        log_msg.logger_name = name_;                     // 日志名字
        log_msg.payload = msg;                           // 有效载荷
        log_msg.time = std::chrono::system_clock::now(); // 系统时刻
        log_msg.tid = std::this_thread::get_id();        // 对应线程id
        std::string formatted = formatter_.format(log_msg);

        sink_->log(formatted); // 运行时多态
    }

    void Logger::trace(const std::string &msg)
    {
        log(Level::TRACE, msg);
    }

    void Logger::debug(const std::string &msg)
    {
        log(Level::DEBUG, msg);
    }

    void Logger::info(const std::string &msg)
    {
        log(Level::INFO, msg);
    }

    void Logger::warn(const std::string &msg)
    {
        log(Level::WARN, msg);
    }

    void Logger::error(const std::string &msg)
    {
        log(Level::ERROR, msg);
    }

    void Logger::critical(const std::string &msg)
    {
        log(Level::CRITICAL, msg);
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
}