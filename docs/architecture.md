# FastLog 架构文档

## 1. 项目背景 & 动机

C++ 生态中 spdlog 是最流行的日志库，但内部实现复杂（10 年迭代）。本项目以「理解 spdlog 核心设计」为目标，从零实现一个高性能日志库，涵盖同步/异步、格式化、Sink 体系、回溯等关键特性。

技术栈：C++11、RAII、多态、模板、多线程、GTest。

## 2. 核心设计思想

**一切皆 Sink** — 日志输出到哪里由 Sink 决定（文件、控制台、彩色、切分、多路），Logger 不关心细节，只负责组装消息并交给 Sink。

**策略模式 + 装饰器模式** — Sink 是策略接口，DistSink / FlushPolicy 是装饰器，可以在运行时自由组合。

**异步 = BlockingQueue + ThreadPool** — 生产者（调用方）入队即返回，消费者（工作线程）从队列取消息写盘。

**回溯 = 环形缓冲区** — CircularQueue 固定大小，满时覆盖最旧元素。启用后在每条 log 时存入，ERROR 时一次性 dump 出来。

## 3. 系统架构

```
┌─────────────────────────────────────────────────────┐
│                    LoggerManager                     │
│                   (单例, CRUD, apply_all)             │
├─────────────────────────────────────────────────────┤
│                                                     │
│   ┌──────────────┐          ┌──────────────────┐    │
│   │    Logger    │          │  AsyncLogger     │    │
│   │  (同步基类)   │◄────────│  (异步, 继承)     │    │
│   │              │          │  BlockingQueue   │    │
│   │  Formatter   │          │  ThreadPool      │    │
│   │  Backtracer  │          │                  │    │
│   │  SinkPtr     │          │                  │    │
│   └──────┬───────┘          └──────────────────┘    │
│          │                                          │
│   ┌──────▼──────────────────────────────────────┐   │
│   │              Sink (策略接口)                  │   │
│   │         log() / flush()                     │   │
│   └────────────────┬────────────────────────────┘   │
│          ┌─────────┼─────────────┐                  │
│          │         │              │                  │
│   ┌──────▼──┐ ┌────▼────┐ ┌──────▼──────┐          │
│   │Stdout   │ │File     │ │Rotating    │          │
│   │Color    │ │         │ │File        │          │
│   └─────────┘ └─────────┘ └─────────────┘          │
│          ┌─────────┴──────────┐                     │
│   ┌──────▼──────┐    ┌────────▼───────┐            │
│   │  DistSink   │    │  FlushPolicy   │            │
│   │  (多路分发)  │    │  (刷盘策略)     │            │
│   └─────────────┘    └────────────────┘            │
└─────────────────────────────────────────────────────┘
```

**数据流**：

```
LOG_INFO(logger, msg)
  → Logger::log(level, msg, file, line)
    → level 过滤
    → buildLogMessage()        // 组装 LogMessage
    → backtracer_.push()        // 回溯缓存（若启用）
    → formatter_.format()       // 格式化
    → sink_->log(level, str)    // 策略模式 → 写盘
```

## 4. 核心业务流程

### 4.1 同步写日志

```
调用方线程
  │ LOG_INFO(logger, "msg")
  ├─ 等级过滤（原子变量 level_）
  ├─ 构造 LogMessage（时间/线程ID/文件/行号）
  ├─ Backtracer::push() —— 若启用
  ├─ Formatter::format() —— 根据 pattern 格式化
  └─ Sink::log() —— 运行时多态，写盘
```

### 4.2 异步入队

```
调用方线程（不阻塞）          后台工作线程
  │                              │
  ├─ level 过滤                   ├─ while(running || !empty)
  ├─ buildLogMessage()           │    queue_.pop(msg)  ← 阻塞等待
  ├─ backtracer_.push()           │    format(msg)
  └─ queue_.push(msg) →→→→→→→→→→├─ sink_->log()
       notify_one()              └─ 循环
```

### 4.3 Backtrace 回溯

```
启用:  logger.enableBacktrace(32)     // CircularQueue(32)

LOG_INFO → backtracer_.push(msg)     // 入环形缓冲区
LOG_INFO → backtracer_.push(msg)
  ...
ERROR   → logger.dumpBacktrace()     // forEach → format → sink 输出
           (输出缓存中最近 32 条)
```

### 4.4 文件切分

```
log("msg")
  │
  ├─ 当前文件大小 + 本条长度 > max_size ?
  │    YES → rotate()
  │         ├─ close 当前文件
  │         ├─ app.log → app.1.log
  │         ├─ app.1.log → app.2.log
  │         ├─ ...
  │         ├─ 删除 app.max_files.log
  │         └─ open 新 app.log
  │
  └─ ofs_ << msg << '\n'
```

## 5. 技术实现细节

### 5.1 线程安全

| 组件 | 同步机制 |
|------|----------|
| Logger::level_ | `std::atomic<Level>`，无锁读写 |
| BlockingQueue | `std::mutex` + `std::condition_variable` |
| ThreadPool | `std::mutex` + `std::condition_variable` |
| FileSink / StdoutSink | `std::mutex` 保护写操作 |
| LoggerManager | `std::mutex` 保护 map 操作 |
| Backtracer | `std::mutex` 保护环形队列 |
| CircularQueue | 非线程安全（由 Backtracer::mutex_ 保护） |

### 5.2 Sink 体系设计

```
Sink (抽象基类)
├── log(Level, string)  —— 带等级的 log，默认委托给旧接口
├── log(string)         —— 纯虚函数，子类必须实现
└── flush()             —— 纯虚函数

子类实现：
  StdoutSink        → std::cout << msg
  ColorStdoutSink   → ANSI颜色码 + std::cout + 重置
  FileSink          → std::ofstream << msg，自动 mkdir 父目录
  RotatingFileSink  → FileSink + 大小检查 + 文件轮转
  DistSink          → 持有 vector<SinkPtr>，转发给所有子 Sink
  FlushPolicy       → 持有 SinkPtr，等级/定时触发 flush
```

### 5.3 Formatter Pattern 语法

```
%Y %m %d %H %M %S  — 时间分量
%l                  — 等级小写 (trace, info)
%^l                 — 等级大写 (TRACE, INFO)
%t                  — 线程 ID
%g                  — Logger 名称
%v                  — 日志正文 payload
%@                  — [file:line]（无源文件时为空）
%%                  — 字面 %
```

默认 pattern：`[%Y-%m-%d %H:%M:%S][%^l][%t][%@] %v`

### 5.4 Backtrace 实现细节

- `CircularQueue<T>`：基于 `std::vector` 的环形队列，内部预留 1 个空位标记「满」
- 固定容量，满时覆盖最旧元素（head 前进）
- `Backtracer::push()`：先检查 `enabled_` 原子标志（快速路径无锁），再加锁写入
- `forEach()`：加锁取出所有元素，逐一调用回调，取出后队列变空

### 5.5 Logger ↔ AsyncLogger

- `Logger` 定义虚函数 `log(Level, string, file, line)`
- `AsyncLogger` 覆写 `log()`：不格式化、不写盘，直接入队
- 共享 `buildLogMessage()`（protected 方法，消除重复代码）
- 共享 `pushBacktrace()`（protected 方法，回溯缓存）

## 6. 性能表现

单线程文件落盘（详细数据见 `./bench_tps`）：

| 场景 | 吞吐 |
|------|------|
| 同步 Logger | ~16 万 TPS |
| 异步 Logger（总吞吐） | ~22 万 TPS |
| 异步入队（调用方 QPS） | ~100 万 QPS |
| 纯格式化（format only） | ~29 万 OPS |
| 纯写文件（write only） | ~530 万 OPS |
| RotatingFileSink | ~240 万 TPS |
| DistSink（双路文件） | ~150 万 TPS |
| Backtrace 开销 | ~1%（可忽略） |

核心瓶颈：格式化（`std::put_time` 和 `std::ostringstream`）占总耗时 ~90%。

## 7. 不足 & 优化方向

| 不足 | 优化方向 |
|------|----------|
| Formatter 每次格式化都调用 `localtime_r` | 缓存秒级时间戳，同秒内复用 |
| 日志消息没有参数化（仅 `string`） | 支持 `fmt` 风格：`LOG_INFO("x={}", 42)` |
| `BlockingQueue` 固定 `std::queue` | 可改为无锁队列（`lock-free SPSC`） |
| ThreadPool 线程数固定 | 可支持动态扩缩容 |
| 无 DailyFileSink | 按天/按小时切分 |
| 无 Syslog / UDP Sink | 远程日志输出 |
| 不支持宽字符 | wchar_t / UTF-8 BOM |
