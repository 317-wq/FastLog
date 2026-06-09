/**
 * 示例 03：自定义日志格式
 *
 * 演示 Formatter 的格式 pattern 自定义功能。
 * 同时输出到控制台和文件，对比不同格式效果。
 */

#include <iostream>

#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"
#include "../include/sink/stdout_sink.h"

int main() {
    // --- 格式 1：简洁格式 ---
    {
        auto sink = std::make_shared<ljt::FileSink>("logs/03_compact.log");
        ljt::Logger logger("compact", sink);
        logger.setPattern("[%H:%M:%S] [%^l] %v");

        LOG_INFO(logger,  "简洁格式：只有时间+等级+消息");
        LOG_WARN(logger,  "适合对可读性要求高的场景");
        LOG_ERROR(logger, "错误消息一目了然");
    }

    // --- 格式 2：详细格式（默认） ---
    {
        auto sink = std::make_shared<ljt::FileSink>("logs/03_verbose.log");
        ljt::Logger logger("verbose", sink);
        // 不设 pattern，使用默认： [时间][LEVEL][线程][FILE:LINE] msg

        LOG_INFO(logger,  "详细格式：包含完整上下文信息");
        LOG_ERROR(logger, "适合排查问题时使用");
    }

    // --- 格式 3：纯消息 ---
    {
        auto sink = std::make_shared<ljt::FileSink>("logs/03_plain.log");
        ljt::Logger logger("plain", sink);
        logger.setPattern("%v");

        LOG_INFO(logger,  "纯消息格式：只有正文");
        LOG_WARN(logger,  "没有任何元数据装饰");
    }

    std::cout << "三种格式的日志已分别写入：" << std::endl;
    std::cout << "  logs/03_compact.log — 简洁格式" << std::endl;
    std::cout << "  logs/03_verbose.log — 详细格式（默认）" << std::endl;
    std::cout << "  logs/03_plain.log   — 纯消息格式" << std::endl;
    return 0;
}
