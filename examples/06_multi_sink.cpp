/**
 * 示例 06：多路输出（DistSink）
 *
 * 一个 Logger 同时输出到控制台（带颜色）和文件。
 */

#include <iostream>

#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/dist_sink.h"
#include "../include/sink/color_stdout_sink.h"
#include "../include/sink/file_sink.h"

int main() {
    // 创建两个子 Sink
    auto console = std::make_shared<ljt::ColorStdoutSink>();
    auto file    = std::make_shared<ljt::FileSink>("logs/06_multi.log");

    // 用 DistSink 聚合
    ljt::DistSink dist({console, file});
    // ljt::DistSink dist;
    // dist.addSink(console);
    // dist.addSink(file);

    // 创建 Logger，传入 DistSink
    ljt::Logger logger("multi", std::make_shared<ljt::DistSink>(dist.sinks()));

    std::cout << "以下日志同时输出到控制台（彩色）和文件（logs/06_multi.log）：\n"
              << std::endl;

    LOG_INFO(logger,  "这条消息同时出现在控制台和文件");
    LOG_WARN(logger,  "警告：CPU 使用率超过 80%");
    LOG_ERROR(logger, "错误：数据库连接失败");
    LOG_CRITICAL(logger,"严重：内存不足，即将 OOM");

    logger.flush();
    std::cout << "\n日志已同时写入控制台和 logs/06_multi.log" << std::endl;
    return 0;
}
