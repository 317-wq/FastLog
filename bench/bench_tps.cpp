/**
 * FastLog 性能压测：对比同步 Logger vs 异步 AsyncLogger 的 TPS
 *
 * 测试维度：
 *   - 同步 Logger：单线程 / 多线程
 *   - 异步 AsyncLogger：单线程 / 多线程，不同后台线程数 (1/2/4)
 *
 * 每个测试输出两个指标：
 *   1. 调用方 TPS  — 调用 log 接口本身的吞吐（异步不阻塞，优势明显）
 *   2. 总吞吐 TPS  — 包含落盘的总耗时
 *
 * 使用便捷宏 LOG_INFO() 自动注入 __FILE__ / __LINE__
 * 日志文件输出到项目 logs/ 目录
 */

#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "../include/async_logger.h"
#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"

using namespace ljt;
using namespace std::chrono;
using Clock = high_resolution_clock;

// ============================================================
// 辅助工具
// ============================================================

/// 日志输出目录
static const std::string LOG_DIR = "logs";

struct TempFile {
    std::string path;
    explicit TempFile(std::string p) : path(std::move(p)) {}
    ~TempFile() { std::filesystem::remove(path); }
};

inline double elapsed_ms(Clock::time_point start) {
    return duration<double, std::milli>(Clock::now() - start).count();
}

inline double to_tps(int total, double ms) {
    return ms > 0.0 ? (total / (ms / 1000.0)) : 0.0;
}

/// 等待异步队列排空（通过轮询目标文件行数验证）
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

// ============================================================
// 打印分隔线
// ============================================================

static void printHeader(const std::string &title) {
    std::cout << "\n" << std::string(72, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(72, '=') << "\n\n";
}

// ============================================================
// 同步 Logger
// ============================================================

static void benchSync(int total, int thread_num, const std::string &path) {
    auto sink = std::make_shared<FileSink>(path);
    Logger logger("sync", sink);
    int per_thread = total / thread_num;

    auto t0 = Clock::now();
    {
        std::vector<std::thread> workers;
        for (int i = 0; i < thread_num; ++i) {
            workers.emplace_back([&, per_thread]() {
                for (int j = 0; j < per_thread; ++j)
                    LOG_INFO(logger, "bench sync msg");
            });
        }
        for (auto &t : workers) t.join();
    }
    double call_ms = elapsed_ms(t0);   // 同步下调用 = 落盘

    sink->flush();
    double total_ms = elapsed_ms(t0);

    int actual = per_thread * thread_num;
    std::cout << std::left
              << "  " << std::setw(22) << "SyncLogger"
              << " 线程=" << std::setw(2) << thread_num
              << "  总量=" << std::setw(9) << actual
              << "  耗时=" << std::fixed << std::setprecision(2)
              << std::setw(8) << total_ms << " ms"
              << "  TPS=" << std::setw(10) << std::setprecision(0)
              << to_tps(actual, total_ms) << "\n";
}

// ============================================================
// 异步 AsyncLogger
// ============================================================

static void benchAsync(int total, int thread_num, int pool_size,
                       const std::string &path) {
    auto sink = std::make_shared<FileSink>(path);
    AsyncLogger logger("async", sink, pool_size);
    int per_thread = total / thread_num;

    // === 阶段1：测量纯入队耗时（调用方视角） ===
    auto t0 = Clock::now();
    {
        std::vector<std::thread> workers;
        for (int i = 0; i < thread_num; ++i) {
            workers.emplace_back([&, per_thread]() {
                for (int j = 0; j < per_thread; ++j)
                    LOG_INFO(logger, "bench async msg");
            });
        }
        for (auto &t : workers) t.join();
    }
    double enqueue_ms = elapsed_ms(t0);

    // === 阶段2：等待队列排空 + 落盘 ===
    int actual = per_thread * thread_num;
    waitDrain(path, actual);
    sink->flush();
    double total_ms = elapsed_ms(t0);

    // 输出
    std::cout << std::left
              << "  " << std::setw(22)
              << ("AsyncLogger(pool=" + std::to_string(pool_size) + ")")
              << " 线程=" << std::setw(2) << thread_num
              << "  总量=" << std::setw(9) << actual
              << "  入队=" << std::fixed << std::setprecision(2)
              << std::setw(8) << enqueue_ms << " ms"
              << "  总耗时=" << std::setw(8) << total_ms << " ms"
              << "  调用TPS=" << std::setw(10) << std::setprecision(0)
              << to_tps(actual, enqueue_ms)
              << "  总TPS=" << to_tps(actual, total_ms) << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    // 确保 logs 目录存在
    std::filesystem::create_directories(LOG_DIR);

    std::cout << "FastLog TPS 压测 — 同步 vs 异步\n";
    std::cout << "日志格式: [时间][LEVEL][线程ID][FILE:LINE] payload\n";
    std::cout << "使用宏:   LOG_INFO(logger, msg)  自动注入 __FILE__/__LINE__\n";
    std::cout << "日志目录: " << LOG_DIR << "/\n";
    std::cout << "说明: 同步下调用耗时 = 总耗时(直接落盘);\n";
    std::cout << "      异步下调用耗时 = 入队耗时(不阻塞), 总耗时含排水落盘.\n";

    // ---------- 单线程场景 ----------
    printHeader("单线程压测");

    {
        TempFile f(LOG_DIR + "/bench_sync_1.log");
        benchSync(100000, 1, f.path);
    }
    {
        TempFile f(LOG_DIR + "/bench_async_p1.log");
        benchAsync(100000, 1, 1, f.path);
    }
    {
        TempFile f(LOG_DIR + "/bench_async_p2.log");
        benchAsync(100000, 1, 2, f.path);
    }

    // ---------- 大量单线程 ----------
    printHeader("大量单线程压测 (500,000 条)");

    {
        TempFile f(LOG_DIR + "/bench_sync_large.log");
        benchSync(500000, 1, f.path);
    }
    {
        TempFile f(LOG_DIR + "/bench_async_large.log");
        benchAsync(500000, 1, 4, f.path);
    }

    // ---------- 多线程场景 ----------
    printHeader("4 线程并发压测 (4 × 100,000 = 400,000 条)");

    {
        TempFile f(LOG_DIR + "/bench_sync_4t.log");
        benchSync(400000, 4, f.path);
    }
    {
        TempFile f(LOG_DIR + "/bench_async_p2_4t.log");
        benchAsync(400000, 4, 2, f.path);
    }
    {
        TempFile f(LOG_DIR + "/bench_async_p4_4t.log");
        benchAsync(400000, 4, 4, f.path);
    }

    // ---------- 高并发场景 ----------
    printHeader("8 线程高并发压测 (8 × 50,000 = 400,000 条)");

    {
        TempFile f(LOG_DIR + "/bench_sync_8t.log");
        benchSync(400000, 8, f.path);
    }
    {
        TempFile f(LOG_DIR + "/bench_async_p4_8t.log");
        benchAsync(400000, 8, 4, f.path);
    }

    std::cout << "\n压测完成，日志文件已自动清理.\n";
    return 0;
}
