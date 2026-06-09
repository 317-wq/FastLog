/**
 * 示例 08：日志回溯（Backtrace）
 *
 * 启用回溯后，Logger 会缓存最近 N 条日志。
 * 当发生错误时，调用 dumpBacktrace() 将错误前的上下文日志全部输出，
 * 帮助排查「crash 前发生了什么」。
 */

#include <iostream>

#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"

int main() {
    auto sink = std::make_shared<ljt::FileSink>("logs/08_backtrace.log");
    ljt::Logger logger("bt", sink);

    // 启用回溯：缓存最近 5 条日志
    logger.enableBacktrace(5);

    std::cout << "模拟正常处理流程..." << std::endl;

    LOG_INFO(logger, "用户登录成功，uid=10001");
    LOG_INFO(logger, "查询订单列表，count=42");
    LOG_DEBUG(logger,"缓存命中率 95%");
    LOG_INFO(logger, "计算运费：¥15.00");
    LOG_INFO(logger, "创建支付订单 #20240001");
    LOG_DEBUG(logger,"调用支付网关...");
    LOG_INFO(logger, "支付回调：status=success");

    std::cout << "一切正常，但突然出现问题..." << std::endl;
    LOG_ERROR(logger, "数据库写入失败：connection lost!");

    std::cout << "\n=== 现在 dump 回溯日志（ERROR 发生前的上下文） ===\n"
              << std::endl;

    // 将缓存的最近 5 条日志 dump 出来
    logger.dumpBacktrace();

    logger.flush();

    std::cout << "\n日志已写入 logs/08_backtrace.log" << std::endl;
    std::cout << "注意：backtrace dump 的日志会在正常日志后面，可以看到 ERROR 前的上下文"
              << std::endl;
    return 0;
}
