/**
 * 示例 07：Flush 策略
 *
 * FlushPolicy 包装 Sink，支持两种刷盘触发方式：
 *   1. 等级触发：ERROR/CRITICAL 时自动 flush
 *   2. 定时触发：每隔 N 秒自动 flush
 *
 * 在 crash 场景下，等级触发可以确保错误消息不丢失。
 */

#include <chrono>
#include <iostream>
#include <thread>

#include "../include/flush_policy.h"
#include "../include/sink/file_sink.h"

int main() {
    // FlushPolicy 包装 FileSink
    //   - ERROR 及以上自动 flush
    //   - 每 2 秒定时 flush
    auto file = std::make_shared<ljt::FileSink>("logs/07_flush.log");
    ljt::FlushPolicy policy(file, ljt::Level::ERROR,
                            std::chrono::seconds(2));

    std::cout << "Flush 策略演示：" << std::endl;
    std::cout << "  INFO 消息 — 不触发 flush（留在缓冲区）" << std::endl;
    std::cout << "  ERROR 消息 — 立即触发 flush（刷盘）\n" << std::endl;

    policy.log(ljt::Level::INFO,  "第1条 INFO：不会立即刷盘");
    policy.log(ljt::Level::INFO,  "第2条 INFO：仍在缓冲区");

    std::cout << "\n--- 现在写入 ERROR，会触发立即刷盘 ---" << std::endl;
    policy.log(ljt::Level::ERROR, "发生错误！这一条会触发 flush，把之前的也刷出去");

    std::cout << "--- 继续写入 ---" << std::endl;
    policy.log(ljt::Level::INFO,  "第3条 INFO：重新开始缓冲");
    policy.log(ljt::Level::WARN,  "警告：也不触发 flush（WARN < ERROR）");

    std::cout << "\n等待 2 秒让定时 flush 触发..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    policy.log(ljt::Level::INFO, "第4条 INFO：定时 flush 已触发过，这是新的");

    std::cout << "日志已写入 logs/07_flush.log" << std::endl;
    return 0;
}