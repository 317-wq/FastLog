/**
 * FastLog 综合性能压测：TPS / QPS / OPS
 *
 * 测试维度：
 *   TPS (Transactions/sec)：完整日志写入吞吐（format → write → flush）
 *   QPS (Queries/sec)：    调用方吞吐（入队速率，异步场景不阻塞）
 *   OPS (Operations/sec)： 单项操作速率（纯格式化、纯写文件）
 *
 * 所有日志文件输出到 logs/ 目录，保留以供查看效果。
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include "../include/async_logger.h"
#include "../include/backtracer.h"
#include "../include/default_logger.h"
#include "../include/flush_policy.h"
#include "../include/formatter.h"
#include "../include/log_message.h"
#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/color_stdout_sink.h"
#include "../include/sink/dist_sink.h"
#include "../include/sink/file_sink.h"
#include "../include/sink/rotating_file_sink.h"

using namespace ljt;
using namespace std::chrono;
using Clock = high_resolution_clock;

static const std::string LOG_DIR = "logs";
static const std::string SEP(72, '=');

// ============================================================
// 辅助
// ============================================================

inline double elapsed_ms(Clock::time_point start) {
    return duration<double, std::milli>(Clock::now() - start).count();
}

inline double toRate(int total, double ms) {
    return ms > 0.0 ? (total / (ms / 1000.0)) : 0.0;
}

inline std::string formatRate(double rate) {
    if (rate >= 1e6) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << (rate / 1e6) << " M/s";
        return oss.str();
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << rate << " /s";
    return oss.str();
}

static void waitDrain(const std::string &path, int expected_lines,
                      int timeout_sec = 10) {
    auto deadline = Clock::now() + std::chrono::seconds(timeout_sec);
    while (Clock::now() < deadline) {
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            int lines = 0;
            std::string dummy;
            while (std::getline(ifs, dummy)) ++lines;
            if (lines >= expected_lines) return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

static void printHeader(const std::string &title) {
    std::cout << "\n\033[1;36m" << SEP << "\n  " << title << "\n" << SEP
              << "\033[0m\n\n";
}

static void printRow(const std::string &name, const std::string &field1,
                     const std::string &field2, const std::string &field3,
                     const std::string &field4) {
    std::cout << std::left
              << "  " << std::setw(26) << name
              << std::setw(16) << field1
              << std::setw(16) << field2
              << std::setw(16) << field3
              << std::setw(16) << field4 << "\n";
}

// ============================================================
// 1. TPS — 同步 vs 异步（完整落盘吞吐）
// ============================================================

static void benchSyncTPS(const std::string &path, int total, int thread_num) {
    auto sink = std::make_shared<FileSink>(path);
    Logger logger("sync", sink);
    int per = total / thread_num;

    auto t0 = Clock::now();
    {
        std::vector<std::thread> workers;
        for (int i = 0; i < thread_num; ++i)
            workers.emplace_back([&, per]() {
                for (int j = 0; j < per; ++j)
                    LOG_INFO(logger, "sync tps msg");
            });
        for (auto &t : workers) t.join();
    }
    sink->flush();
    double ms = elapsed_ms(t0);

    int actual = per * thread_num;
    printRow(std::string("Sync (") + std::to_string(thread_num) + "线程)",
             std::to_string(actual), std::to_string((int)ms) + "ms",
             formatRate(toRate(actual, ms)), "TPS");
}

static void benchAsyncTPS(const std::string &path, int total, int thread_num,
                          int pool_size) {
    int per = total / thread_num;
    int actual = per * thread_num;
    double enq_ms = 0, total_ms = 0;

    auto t0 = Clock::now();
    {
        auto sink = std::make_shared<FileSink>(path);
        AsyncLogger logger("async", sink, pool_size);

        // 入队阶段：测量纯调用耗时
        auto t_enq = Clock::now();
        {
            std::vector<std::thread> workers;
            for (int i = 0; i < thread_num; ++i)
                workers.emplace_back([&, per]() {
                    for (int j = 0; j < per; ++j)
                        LOG_INFO(logger, "async tps msg");
                });
            for (auto &t : workers) t.join();
        }
        enq_ms = elapsed_ms(t_enq);

        // logger 析构：停止队列 → 等待线程排空队列 → 写盘完成
    }
    total_ms = elapsed_ms(t0);

    printRow(std::string("Async(p") + std::to_string(pool_size) + " " +
                 std::to_string(thread_num) + "线程)",
             std::to_string(actual), std::to_string((int)total_ms) + "ms",
             formatRate(toRate(actual, total_ms)),
             "TPS(调用:" + formatRate(toRate(actual, enq_ms)) + ")");
}

// ============================================================
// 2. QPS — 纯调用吞吐（异步入队不阻塞）
// ============================================================

static void benchQPS(int total, int thread_num) {
    auto sink = std::make_shared<FileSink>(LOG_DIR + "/bench_qps.log");
    AsyncLogger logger("qps", sink, 2);
    int per = total / thread_num;

    // 纯内存操作：入队不落盘
    auto t0 = Clock::now();
    {
        std::vector<std::thread> workers;
        for (int i = 0; i < thread_num; ++i)
            workers.emplace_back([&, per]() {
                for (int j = 0; j < per; ++j)
                    LOG_INFO(logger, "qps msg");
            });
        for (auto &t : workers) t.join();
    }
    double ms = elapsed_ms(t0);

    int actual = per * thread_num;
    printRow(std::string("QPS Async(") + std::to_string(thread_num) + "线程)",
             std::to_string(actual), std::to_string((int)ms) + "ms",
             formatRate(toRate(actual, ms)), "QPS(入队)");
}

// ============================================================
// 3. OPS — 单项操作速率
// ============================================================

static void benchOPS() {
    const int N = 500000;

    // 3a. 纯格式化
    Formatter fmt;
    LogMessage msg;
    msg.level = Level::INFO;
    msg.logger_name = "test";
    msg.payload = "ops format test message with some padding";
    msg.time = system_clock::now();
    msg.tid = std::this_thread::get_id();
    msg.source_file = "main.cpp";
    msg.source_line = 100;

    auto t0 = Clock::now();
    std::string dummy;
    for (int i = 0; i < N; ++i) {
        dummy = fmt.format(msg);
    }
    double format_ms = elapsed_ms(t0);
    volatile auto prevent = dummy.size();
    (void)prevent;

    // 3b. 纯文件写入（无格式化开销）
    auto t1 = Clock::now();
    std::ofstream ofs(LOG_DIR + "/bench_ops_raw.log", std::ios::app);
    for (int i = 0; i < N; ++i) {
        ofs << "[INFO] ops raw write test message\n";
    }
    ofs.flush();
    ofs.close();
    double write_ms = elapsed_ms(t1);

    // 3c. 完整 Pipeline（format + write）
    auto sink = std::make_shared<FileSink>(LOG_DIR + "/bench_ops_full.log");
    Logger logger("ops", sink);
    auto t2 = Clock::now();
    for (int i = 0; i < N; ++i) {
        LOG_INFO(logger, "ops full pipeline message");
    }
    sink->flush();
    double full_ms = elapsed_ms(t2);

    printHeader("OPS — 单项操作速率 (" + std::to_string(N) + " 条)");
    printRow("  纯格式化 (format only)", std::to_string(N),
             std::to_string((int)format_ms) + "ms",
             formatRate(toRate(N, format_ms)), "OPS");
    printRow("  纯写文件 (write only)", std::to_string(N),
             std::to_string((int)write_ms) + "ms",
             formatRate(toRate(N, write_ms)), "OPS");
    printRow("  完整流水线 (format+write)", std::to_string(N),
             std::to_string((int)full_ms) + "ms",
             formatRate(toRate(N, full_ms)), "OPS");
}

// ============================================================
// 4. RotatingFileSink 压测
// ============================================================

static void benchRotating() {
    const std::string base = LOG_DIR + "/bench_rotate.log";
    const int N = 50000;

    auto t0 = Clock::now();
    {
        RotatingFileSink sink(base, 1024 * 200, 5); // 200KB 切一次
        for (int i = 0; i < N; ++i) {
            sink.log(Level::INFO, "rotating file sink benchmark message " + std::to_string(i));
        }
        sink.flush();
    }
    double ms = elapsed_ms(t0);

    // 统计生成了几个文件
    int file_count = 0;
    for (int i = 0; i <= 5; ++i) {
        std::string p = i == 0 ? base : base + "." + std::to_string(i);
        std::ifstream ifs(p);
        if (ifs.good()) {
            ++file_count;
            int lines = 0;
            std::string l;
            while (std::getline(ifs, l)) ++lines;
            std::cout << "    " << p << "  (" << lines << " 行)\n";
        }
    }

    printRow(std::string("RotatingFileSink(") + std::to_string(file_count) + "个文件)",
             std::to_string(N), std::to_string((int)ms) + "ms",
             formatRate(toRate(N, ms)), "TPS");
}

// ============================================================
// 5. DistSink 开销测试
// ============================================================

static void benchDistSink() {
    const int N = 100000;

    // 用两个文件 Sink 模拟多路分发（避免控制台刷屏）
    auto file1 = std::make_shared<FileSink>(LOG_DIR + "/bench_dist_1.log");
    auto file2 = std::make_shared<FileSink>(LOG_DIR + "/bench_dist_2.log");
    DistSink dist({file1, file2});

    auto t0 = Clock::now();
    for (int i = 0; i < N; ++i) {
        dist.log(Level::INFO, "dist sink benchmark msg " + std::to_string(i));
    }
    dist.flush();
    double ms = elapsed_ms(t0);

    printRow("DistSink(file×2)", std::to_string(N),
             std::to_string((int)ms) + "ms", formatRate(toRate(N, ms)), "TPS");
}

// ============================================================
// 6. Backtrace 开销测试
// ============================================================

static void benchBacktrace() {
    const int N = 200000;

    auto sink = std::make_shared<FileSink>(LOG_DIR + "/bench_backtrace.log");
    Logger logger("bt", sink);
    logger.enableBacktrace(64);

    auto t0 = Clock::now();
    for (int i = 0; i < N; ++i) {
        LOG_INFO(logger, "backtrace test message");
    }
    double ms_with = elapsed_ms(t0);

    // 关闭 backtrace 再测
    logger.disableBacktrace();
    auto t1 = Clock::now();
    for (int i = 0; i < N; ++i) {
        LOG_INFO(logger, "no backtrace test message");
    }
    double ms_without = elapsed_ms(t1);
    sink->flush();

    printRow("Logger(backtrace=64)", std::to_string(N),
             std::to_string((int)ms_with) + "ms",
             formatRate(toRate(N, ms_with)), "TPS");
    printRow("Logger(backtrace=off)", std::to_string(N),
             std::to_string((int)ms_without) + "ms",
             formatRate(toRate(N, ms_without)), "TPS");
    double overhead = (ms_with / ms_without - 1.0) * 100.0;
    std::cout << "    回溯开销: " << std::fixed << std::setprecision(1)
              << overhead << "%\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::filesystem::create_directories(LOG_DIR);

    std::cout << "\n\033[1;35m"
              << "╔══════════════════════════════════════════════════════════════════════╗\n"
              << "║           FastLog 综合性能压测 — TPS · QPS · OPS                    ║\n"
              << "╚══════════════════════════════════════════════════════════════════════╝\033[0m\n";
    std::cout << "日志目录: " << LOG_DIR << "/\n";
    std::cout << "说明: TPS=完整落盘吞吐, QPS=调用入队吞吐, OPS=单项操作速率\n";

    // ============ TPS ============
    printHeader("1. TPS — 同步 vs 异步吞吐");
    std::cout << "  " << std::left
              << std::setw(26) << "  场景"
              << std::setw(16) << "总量"
              << std::setw(16) << "耗时"
              << std::setw(16) << "吞吐"
              << std::setw(16) << "备注" << "\n"
              << "  " << std::string(90, '-') << "\n";

    // 单线程
    benchSyncTPS(LOG_DIR + "/bench_sync_1.log", 100000, 1);
    benchAsyncTPS(LOG_DIR + "/bench_async_1.log", 100000, 1, 1);
    benchAsyncTPS(LOG_DIR + "/bench_async_1p2.log", 100000, 1, 2);

    // 多线程
    benchSyncTPS(LOG_DIR + "/bench_sync_4.log", 400000, 4);
    benchAsyncTPS(LOG_DIR + "/bench_async_4.log", 400000, 4, 2);
    benchAsyncTPS(LOG_DIR + "/bench_async_4p4.log", 400000, 4, 4);

    benchSyncTPS(LOG_DIR + "/bench_sync_8.log", 400000, 8);
    benchAsyncTPS(LOG_DIR + "/bench_async_8.log", 400000, 8, 4);

    // ============ QPS ============
    printHeader("2. QPS — 纯调用吞吐（入队不落盘）");
    std::cout << "  " << std::left
              << std::setw(26) << "  场景"
              << std::setw(16) << "总量"
              << std::setw(16) << "耗时"
              << std::setw(16) << "吞吐"
              << std::setw(16) << "类型" << "\n"
              << "  " << std::string(90, '-') << "\n";

    benchQPS(500000, 1);
    benchQPS(500000, 4);
    benchQPS(500000, 8);

    // ============ OPS ============
    benchOPS();

    // ============ Rotating ============
    printHeader("3. RotatingFileSink 切分压测");
    std::cout << "  " << std::left
              << std::setw(26) << "  场景"
              << std::setw(16) << "总量"
              << std::setw(16) << "耗时"
              << std::setw(16) << "吞吐"
              << std::setw(16) << "类型" << "\n"
              << "  " << std::string(90, '-') << "\n";
    benchRotating();

    // ============ DistSink ============
    printHeader("4. DistSink 多输出开销");
    std::cout << "  " << std::left
              << std::setw(26) << "  场景"
              << std::setw(16) << "总量"
              << std::setw(16) << "耗时"
              << std::setw(16) << "吞吐"
              << std::setw(16) << "类型" << "\n"
              << "  " << std::string(90, '-') << "\n";
    benchDistSink();

    // ============ Backtrace ============
    printHeader("5. Backtrace 回溯开销");
    std::cout << "  " << std::left
              << std::setw(26) << "  场景"
              << std::setw(16) << "总量"
              << std::setw(16) << "耗时"
              << std::setw(16) << "吞吐"
              << std::setw(16) << "类型" << "\n"
              << "  " << std::string(90, '-') << "\n";
    benchBacktrace();

    // ============ 汇总 ============
    std::cout << "\n\033[1;32m" << SEP << "\n  压测完成！\n";
    std::cout << "  所有日志文件已保留在 " << LOG_DIR << "/ 目录下，可以查看效果。\n";
    std::cout << "  日志文件列表：\n";
    for (const auto &entry : std::filesystem::directory_iterator(LOG_DIR)) {
        if (entry.is_regular_file()) {
            auto sz = entry.file_size();
            std::cout << "    " << entry.path().filename().string()
                      << "  (" << sz << " bytes)\n";
        }
    }
    std::cout << SEP << "\033[0m\n";

    return 0;
}
