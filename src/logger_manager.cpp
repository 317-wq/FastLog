#include "../include/logger_manager.h"

namespace ljt
{
    LoggerManager &LoggerManager::instance()
    {
        // 静态局部变量的初始化[创建]是线程安全的
        static LoggerManager manager;
        return manager;
    }

    void LoggerManager::registerLogger(const LoggerPtr &logger)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loggers_[logger->name()] = logger;
    }

    LoggerPtr LoggerManager::getLogger(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(name);
        if (it == loggers_.end())
        {
            return nullptr;
        }

        return it->second;
    }

    bool LoggerManager::exists(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return loggers_.find(name) != loggers_.end();
    }

    void LoggerManager::dropLogger(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(loggers_.find(name) == loggers_.end())
            return;
        loggers_.erase(name);
    }
}