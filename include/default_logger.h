#pragma once

#include "logger.h"

namespace ljt
{
    /// 获取默认全局 Logger（首次调用时自动创建，默认彩色输出到 stdout）
    Logger& defaultLogger();

    /// 设置默认 Logger 的过滤等级
    inline void setDefaultLevel(Level level)
    {
        defaultLogger().setLevel(level);
    }
}

/// @file default_logger.h
/// 全局便捷宏：无需手动创建 Logger，自动注入 __FILE__ 和 __LINE__

#define FLOG_TRACE(msg)    ::ljt::defaultLogger().log(::ljt::Level::TRACE,    (msg), __FILE__, __LINE__)
#define FLOG_DEBUG(msg)    ::ljt::defaultLogger().log(::ljt::Level::DEBUG,    (msg), __FILE__, __LINE__)
#define FLOG_INFO(msg)     ::ljt::defaultLogger().log(::ljt::Level::INFO,     (msg), __FILE__, __LINE__)
#define FLOG_WARN(msg)     ::ljt::defaultLogger().log(::ljt::Level::WARN,     (msg), __FILE__, __LINE__)
#define FLOG_ERROR(msg)    ::ljt::defaultLogger().log(::ljt::Level::ERROR,    (msg), __FILE__, __LINE__)
#define FLOG_CRITICAL(msg) ::ljt::defaultLogger().log(::ljt::Level::CRITICAL, (msg), __FILE__, __LINE__)
