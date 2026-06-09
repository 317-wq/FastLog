/**
 * 示例 01：基础用法 — 同步 Logger + 文件输出
 *
 * 演示最基本的日志用法：创建 Logger，用 LOG_* 宏写日志。
 * 日志将输出到 logs/01_basic.log 文件。
 */

#include <iostream>

#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"

int main() {
    // 1. 创建一个文件 Sink，输出到 logs/ 目录
    auto sink = std::make_shared<ljt::FileSink>("logs/01_basic.log");

    // 2. 创建 Logger
    ljt::Logger logger("basic", sink);

    // 3. 设置过滤等级（只记录 WARN 及以上）
    logger.setLevel(ljt::Level::WARN);

    // 4. 使用宏写入日志（自动注入 __FILE__ 和 __LINE__）
    LOG_TRACE(logger,   "这条不会输出（等级太低）");
    LOG_DEBUG(logger,   "这条也不会输出");
    LOG_INFO(logger,    "这条也不会输出");
    LOG_WARN(logger,    "警告：磁盘空间不足");
    LOG_ERROR(logger,   "错误：连接超时");
    LOG_CRITICAL(logger,"严重：服务崩溃");

    // 5. 刷盘
    logger.flush();

    std::cout << "日志已写入 logs/01_basic.log" << std::endl;
    return 0;
}
