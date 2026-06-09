/**
 * 示例 02：异步 Logger — 高并发场景不阻塞调用线程
 *
 * 异步 Logger 用线程池在后台写盘，调用 log() 时仅入队就立即返回。
 * 日志将输出到 logs/02_async.log 文件。
 */

#include <iostream>
#include <thread>
#include <vector>

#include "../include/async_logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"

int main() {
    auto sink = std::make_shared<ljt::FileSink>("logs/02_async.log");

    // pool_size = 2: 用 2 个后台线程写盘
    ljt::AsyncLogger logger("async", sink, 2);

    std::cout << "异步 Logger 启动，8 个线程并发写入..." << std::endl;

    // 8 个线程同时写日志
    std::vector<std::thread> workers;
    for (int t = 0; t < 8; ++t) {
        workers.emplace_back([&logger, t]() {
            for (int i = 0; i < 1000; ++i) {
                LOG_INFO(logger, "线程[" + std::to_string(t)
                         + "] 第 " + std::to_string(i) + " 条");
            }
        });
    }

    for (auto &w : workers) w.join();

    std::cout << "全部入队完成，后台线程正在落盘..." << std::endl;

    // AsyncLogger 析构时会等待队列排空后再退出
    // logger 在此处超出作用域自动析构

    std::cout << "日志已写入 logs/02_async.log" << std::endl;
    return 0;
}
