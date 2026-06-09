# FastLog

一个轻量级 C++ 高性能日志库，仿 spdlog 架构设计，支持同步/异步、自定义格式、彩色终端、文件切分、多路输出、日志回溯等。

## 快速开始

```cpp
#include "default_logger.h"

int main() {
    FLOG_INFO("服务启动成功");
    FLOG_ERROR("连接超时");
    FLOG_CRITICAL("服务崩溃");
}
```

编译：

```bash
cd build && cmake .. && make -j$(nproc)
./examples/example_01   # 运行示例
./bench_tps             # 性能压测
```

## 模块概览

| 模块 | 说明 |
|------|------|
| `Logger` | 同步日志器，等级过滤，多态 Sink |
| `AsyncLogger` | 异步日志器，BlockingQueue + ThreadPool |
| `Formatter` | 自定义 pattern：`"%Y-%m-%d %H:%M:%S [%^l] %v"` |
| `ColorStdoutSink` | ANSI 彩色终端（TRACE白 DEBUG青 INFO绿 WARN黄 ERROR红） |
| `FileSink` | 文件输出 |
| `RotatingFileSink` | 按文件大小自动切分 |
| `DistSink` | 多路分发（同时输出控制台 + 文件） |
| `FlushPolicy` | Flush 策略（等级触发 + 定时触发） |
| `Backtracer` | 日志回溯（环形缓冲区，出错时 dump 上下文） |
| `LoggerManager` | 单例管理器，注册/获取/批量操作 |
| `DefaultLogger` | 零配置全局 Logger，`FLOG_*` 宏直接使用 |

## 使用示例

```cpp
// 1. 同步写文件
auto sink = std::make_shared<FileSink>("logs/app.log");
Logger logger("app", sink);
LOG_INFO(logger, "hello");

// 2. 异步高并发
auto sink = std::make_shared<FileSink>("logs/async.log");
AsyncLogger logger("async", sink, 4);  // 4 个后台线程
LOG_INFO(logger, "不会阻塞调用线程");

// 3. 自定义格式
logger.setPattern("[%H:%M:%S] [%^l] %v");

// 4. 多路输出（控制台彩色 + 文件）
auto dist = std::make_shared<DistSink>(
    std::vector<SinkPtr>{console, file});
Logger logger("multi", dist);

// 5. 日志回溯
logger.enableBacktrace(32);
// ... 大量日志 ...
LOG_ERROR(logger, "出错！");
logger.dumpBacktrace();  // 输出错误前 32 条日志

// 6. 文件切分（10MB 切一次，保留 5 个）
RotatingFileSink sink("logs/app.log", 10*1024*1024, 5);
```

## 性能（单线程，文件落盘）

| 场景 | 吞吐 |
|------|------|
| 同步 Logger | ~16 万 TPS |
| 异步 Logger（总吞吐） | ~22 万 TPS |
| 异步入队（调用方视角） | ~100 万 QPS |
| RotatingFileSink | ~240 万 TPS |
| DistSink（双路分发） | ~150 万 TPS |

> 更多数据见 `./bench_tps`。

## 目录结构

```
FastLog/
├── include/        # 头文件
│   └── sink/       # Sink 子类
├── src/            # 源文件
│   └── sink/
├── tests/          # 单元测试（14 套，GTest）
├── bench/          # 性能压测
├── examples/       # 使用示例（9 个）
├── docs/           # 架构文档
└── logs/           # 日志输出目录
```

## 构建 & 测试

```bash
cd build && cmake .. && make -j$(nproc)
ctest                  # 14 套测试
./bench_tps            # 性能压测
./examples/example_01  # 运行示例
```

## License

MIT
