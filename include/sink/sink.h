#pragma once

#include <memory>
#include <string>
#include <cstdint>

// 前置声明，避免循环依赖
namespace ljt
{
    enum class Level : std::uint8_t;
}

// 日志输出器基类

namespace ljt
{
    class Sink
    {
    public:
        virtual ~Sink() = default;

        /// 输出日志（带等级信息，子类可覆写以实现按等级差异化处理）
        virtual void log(Level level, const std::string &message)
        {
            log(message); // 默认委托给旧接口，保持向后兼容
        }

        /// 输出日志（无等级信息）
        virtual void log(const std::string &message) = 0;

        /// 刷新输出缓冲区
        virtual void flush() = 0;
    };

    using SinkPtr = std::shared_ptr<Sink>;
}
