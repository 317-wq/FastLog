#pragma once

#include "sink.h"
#include "../level.h"
#include <mutex>

namespace ljt
{
    /// 带 ANSI 颜色的标准输出 Sink
    ///
    /// 颜色规则：
    ///   TRACE    — 白色
    ///   DEBUG    — 青色
    ///   INFO     — 绿色
    ///   WARN     — 黄色
    ///   ERROR    — 红色
    ///   CRITICAL — 红底白字

    class ColorStdoutSink : public Sink
    {
    public:
        // 带等级的彩色输出
        void log(Level level, const std::string &message) override;

        // 无等级时默认无色输出
        void log(const std::string &message) override;

        // 刷新输出缓冲区
        void flush() override;

    private:
        // 根据日志等级返回 ANSI 颜色码
        static const char* colorCode(Level level);

        std::mutex mutex_;
    };
}
