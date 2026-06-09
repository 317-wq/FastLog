#pragma once

#include "sink.h"

#include <memory>
#include <mutex>
#include <vector>

namespace ljt
{
    /// 多输出分发 Sink（DistSink / MultiSink）
    ///
    /// 内部持有多个子 Sink，log/flush 时分发到所有子 Sink。
    /// 典型用法：同时输出到控制台和文件。
    ///
    /// 使用示例：
    ///   auto console = std::make_shared<ColorStdoutSink>();
    ///   auto file    = std::make_shared<FileSink>("app.log");
    ///   DistSink dist;
    ///   dist.addSink(console);
    ///   dist.addSink(file);
    ///   Logger logger("app", dist);

    class DistSink : public Sink
    {
    public:
        DistSink() = default;

        /// 用一个初始 Sink 列表构造
        explicit DistSink(std::vector<SinkPtr> sinks);

        // ---- Sink 接口 ----

        void log(Level level, const std::string &message) override;
        void log(const std::string &message) override;
        void flush() override;

        // ---- 管理子 Sink ----

        /// 添加子 Sink
        void addSink(SinkPtr sink);

        /// 移除子 Sink
        void removeSink(const SinkPtr &sink);

        /// 获取当前所有子 Sink（只读）
        std::vector<SinkPtr> sinks() const;

    private:
        std::vector<SinkPtr> sinks_;
        mutable std::mutex mutex_;
    };

    using DistSinkPtr = std::shared_ptr<DistSink>;
}
