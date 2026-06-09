#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../include/backtracer.h"
#include "../include/logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"
#include "../include/sink/stdout_sink.h"

using namespace ljt;

// ============================================================
// Backtracer 基础功能测试
// ============================================================

TEST(BacktracerTest, InitiallyDisabled)
{
    Backtracer bt;
    EXPECT_FALSE(bt.enabled());
}

TEST(BacktracerTest, EnableAndDisable)
{
    Backtracer bt;
    bt.enable(10);
    EXPECT_TRUE(bt.enabled());

    bt.disable();
    EXPECT_FALSE(bt.enabled());
}

TEST(BacktracerTest, PushWhenDisabledDoesNotStore)
{
    Backtracer bt;
    LogMessage msg;
    msg.payload = "should not be stored";

    bt.push(msg);

    int count = 0;
    bt.forEach([&](const LogMessage &) { ++count; });
    EXPECT_EQ(count, 0);
}

TEST(BacktracerTest, PushAndDump)
{
    Backtracer bt;
    bt.enable(5);

    for (int i = 0; i < 5; ++i)
    {
        LogMessage msg;
        msg.payload = "msg " + std::to_string(i);
        bt.push(msg);
    }

    int count = 0;
    bt.forEach([&](const LogMessage &m)
    {
        EXPECT_NE(m.payload.find("msg "), std::string::npos);
        ++count;
    });
    EXPECT_EQ(count, 5);
}

TEST(BacktracerTest, OverwriteOldestWhenFull)
{
    Backtracer bt;
    bt.enable(3);

    for (int i = 0; i < 5; ++i)
    {
        LogMessage msg;
        msg.payload = "msg " + std::to_string(i);
        bt.push(msg);
    }

    // 只保留最后 3 条
    int count = 0;
    std::vector<std::string> payloads;
    bt.forEach([&](const LogMessage &m)
    {
        payloads.push_back(m.payload);
        ++count;
    });

    EXPECT_EQ(count, 3);
    EXPECT_EQ(payloads[0], "msg 2");
    EXPECT_EQ(payloads[1], "msg 3");
    EXPECT_EQ(payloads[2], "msg 4");
}

TEST(BacktracerTest, DumpClearsBuffer)
{
    Backtracer bt;
    bt.enable(10);

    LogMessage msg;
    msg.payload = "hello";
    bt.push(msg);

    // 第一次 dump
    int count = 0;
    bt.forEach([&](const LogMessage &) { ++count; });
    EXPECT_EQ(count, 1);

    // 第二次 dump 应该为空
    count = 0;
    bt.forEach([&](const LogMessage &) { ++count; });
    EXPECT_EQ(count, 0);
}

// ============================================================
// Logger 集成测试
// ============================================================

TEST(BacktracerTest, LoggerEnableAndDump)
{
    const std::string path = "backtrace_test.log";

    {
        auto sink = std::make_shared<FileSink>(path);
        Logger logger("bt", sink);

        logger.enableBacktrace(5);

        LOG_INFO(logger, "message 1");
        LOG_INFO(logger, "message 2");
        LOG_INFO(logger, "message 3");
        LOG_WARN(logger, "warning");

        // dump 回溯日志
        logger.dumpBacktrace();
        sink->flush();
    }

    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());

    std::string content;
    std::string line;
    while (std::getline(ifs, line))
        content += line + "\n";

    // 正常日志
    EXPECT_NE(content.find("message 1"), std::string::npos);
    EXPECT_NE(content.find("message 2"), std::string::npos);
    EXPECT_NE(content.find("warning"), std::string::npos);

    // 回溯 dump 应该也有这些日志
    // 注意：正常输出已经有一次，dump 时又输出一次
    std::size_t first  = content.find("message 1");
    std::size_t second = content.find("message 1", first + 1);
    EXPECT_NE(second, std::string::npos); // message 1 出现两次

    ifs.close();
    std::remove(path.c_str());
}

TEST(BacktracerTest, LoggerBacktraceOverwrite)
{
    const std::string path = "backtrace_overwrite.log";

    {
        auto sink = std::make_shared<FileSink>(path);
        Logger logger("bt", sink);

        // 只保留最近 3 条
        logger.enableBacktrace(3);

        LOG_INFO(logger, "old 1");
        LOG_INFO(logger, "old 2");
        LOG_INFO(logger, "old 3");
        LOG_INFO(logger, "old 4");
        LOG_INFO(logger, "new 1");
        LOG_INFO(logger, "new 2");
        LOG_INFO(logger, "new 3");

        logger.dumpBacktrace();
        sink->flush();
    }

    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());

    std::string content;
    std::string line;
    while (std::getline(ifs, line))
        content += line + "\n";

    // 回溯 dump 的应该是最后 3 条
    // old 1, old 2 应该只在正常输出中出现一次（不在 dump 中再出现）
    EXPECT_NE(content.find("new 1"), std::string::npos);
    EXPECT_NE(content.find("new 2"), std::string::npos);
    EXPECT_NE(content.find("new 3"), std::string::npos);

    ifs.close();
    std::remove(path.c_str());
}

TEST(BacktracerTest, LoggerDisabledBacktrace)
{
    const std::string path = "backtrace_disabled.log";

    {
        auto sink = std::make_shared<FileSink>(path);
        Logger logger("bt", sink);

        // 不启用 backtrace

        LOG_INFO(logger, "info only");

        logger.dumpBacktrace(); // 不应崩溃
        sink->flush();
    }

    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());

    int count = 0;
    std::string line;
    while (std::getline(ifs, line))
        ++count;

    // 只有一条正常输出 + 一条空行（dump 无内容）
    EXPECT_EQ(count, 1);

    ifs.close();
    std::remove(path.c_str());
}
