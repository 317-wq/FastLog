#pragma once

#include "level.h"
#include "logger.h"

/// @file macros.h
/// 便捷宏：自动注入 __FILE__ 和 __LINE__，直接调用 Logger::log()

#define LOG_TRACE(logger, msg)   (logger).log(::ljt::Level::TRACE,   (msg), __FILE__, __LINE__)
#define LOG_DEBUG(logger, msg)   (logger).log(::ljt::Level::DEBUG,   (msg), __FILE__, __LINE__)
#define LOG_INFO(logger, msg)    (logger).log(::ljt::Level::INFO,    (msg), __FILE__, __LINE__)
#define LOG_WARN(logger, msg)    (logger).log(::ljt::Level::WARN,    (msg), __FILE__, __LINE__)
#define LOG_ERROR(logger, msg)   (logger).log(::ljt::Level::ERROR,   (msg), __FILE__, __LINE__)
#define LOG_CRITICAL(logger, msg) (logger).log(::ljt::Level::CRITICAL, (msg), __FILE__, __LINE__)
