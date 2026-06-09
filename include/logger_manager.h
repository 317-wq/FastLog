#pragma once

// 管理多个logger对象

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "logger.h"

namespace ljt
{
    class LoggerManager
    {
    public:
        // 单例模式
        static LoggerManager &instance();

    public:
        // 注册logger对象
        void registerLogger(const LoggerPtr &logger);

        // 获取对应name的logger对象
        LoggerPtr getLogger(const std::string &name);

        // 判断name的logger对象是否存在
        bool exists(const std::string &name);

        // 删除对应的logger对象
        void dropLogger(const std::string &name);

        // ---- 批量操作 ----

        /// 设置所有已注册 Logger 的过滤等级
        void setLevelAll(Level level);

        /// 刷新所有已注册 Logger 的 Sink
        void flushAll();

        /// 对所有已注册 Logger 执行自定义操作
        template <typename Func>
        void applyAll(Func &&func)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto &[name, logger] : loggers_)
            {
                func(logger);
            }
        }

    private:
        LoggerManager() = default;
        // 禁止拷贝构造，赋值构造
        LoggerManager(const LoggerManager &) = delete;
        LoggerManager &operator=(const LoggerManager &) = delete;

    private:
        std::unordered_map<std::string, LoggerPtr> loggers_; // 文件名 -> 管理这个文件日志的logger对象
        std::mutex mutex_;                                   // 保护logger对象的一系列相关操作
    };
}