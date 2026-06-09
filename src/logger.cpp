#include "../include/logger.h"

namespace ljt
{
    Logger::Logger(std::string name, SinkPtr sink)
        : name_(std::move(name)),
          sink_(std::move(sink))
    {
    }

    void Logger::log(Level level, const std::string &msg,
                     const char* file, int line)
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
        log_msg.source_file = file;                      // 源文件名
        log_msg.source_line = line;                      // 源文件行号
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
}