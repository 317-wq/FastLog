#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "../include/formatter.h"

using namespace ljt;

/// 构造一个测试用的 LogMessage
static LogMessage makeMsg(Level level = Level::INFO,
                          const std::string &logger_name = "test",
                          const std::string &payload = "hello",
                          const char* file = "main.cpp",
                          int line = 42)
{
    LogMessage msg;
    msg.level = level;
    msg.logger_name = logger_name;
    msg.payload = payload;
    msg.time = std::chrono::system_clock::now();
    msg.tid = std::this_thread::get_id();
    msg.source_file = file;
    msg.source_line = line;
    return msg;
}

// ============================================================
// 默认格式测试
// ============================================================

TEST(FormatterTest, DefaultPattern)
{
    Formatter fmt;
    EXPECT_EQ(fmt.pattern(), Formatter::defaultPattern());
}

TEST(FormatterTest, DefaultFormatContainsPayload)
{
    Formatter fmt;
    auto msg = makeMsg(Level::INFO, "test", "hello world");
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("hello world"), std::string::npos);
}

TEST(FormatterTest, DefaultFormatContainsLevel)
{
    Formatter fmt;
    auto msg = makeMsg(Level::ERROR, "test", "error msg");
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("ERROR"), std::string::npos);
}

TEST(FormatterTest, DefaultFormatContainsTimeBrackets)
{
    Formatter fmt;
    auto msg = makeMsg();
    std::string result = fmt.format(msg);
    // 默认格式: [时间][LEVEL][线程ID][FILE:LINE] msg
    EXPECT_NE(result.find("["), std::string::npos);
    EXPECT_NE(result.find("]"), std::string::npos);
}

TEST(FormatterTest, DefaultFormatContainsFileLine)
{
    Formatter fmt;
    auto msg = makeMsg(Level::INFO, "test", "msg", "main.cpp", 42);
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("main.cpp:42"), std::string::npos);
}

// ============================================================
// 自定义格式测试
// ============================================================

TEST(FormatterTest, CustomPattern)
{
    Formatter fmt("%v"); // 仅输出正文
    auto msg = makeMsg(Level::INFO, "test", "pure message");
    std::string result = fmt.format(msg);
    EXPECT_EQ(result, "pure message");
}

TEST(FormatterTest, LevelUppercase)
{
    Formatter fmt("[%^l] %v");
    auto msg = makeMsg(Level::WARN, "test", "warning msg");
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("WARN"), std::string::npos);
}

TEST(FormatterTest, LevelLowercase)
{
    Formatter fmt("[%l] %v");
    auto msg = makeMsg(Level::ERROR, "test", "error msg");
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("error"), std::string::npos);
}

TEST(FormatterTest, LoggerName)
{
    Formatter fmt("[%g] %v");
    auto msg = makeMsg(Level::INFO, "my_logger", "hello");
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("my_logger"), std::string::npos);
}

TEST(FormatterTest, SourceLocation)
{
    Formatter fmt("%@ %v");
    auto msg = makeMsg(Level::INFO, "test", "msg", "app.cpp", 100);
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("app.cpp:100"), std::string::npos);
}

TEST(FormatterTest, SourceLocationEmpty)
{
    Formatter fmt("[%@] %v");
    auto msg = makeMsg(Level::INFO, "test", "msg", "", 0);
    std::string result = fmt.format(msg);
    // 无源文件信息时 %@ 为空，产生 "[] msg"
    EXPECT_NE(result.find("[]"), std::string::npos);
}

TEST(FormatterTest, LiteralPercent)
{
    Formatter fmt("%% %v");
    auto msg = makeMsg(Level::INFO, "test", "hello");
    std::string result = fmt.format(msg);
    EXPECT_EQ(result, "% hello");
}

TEST(FormatterTest, UnknownToken)
{
    Formatter fmt("%X %v");
    auto msg = makeMsg(Level::INFO, "test", "hello");
    std::string result = fmt.format(msg);
    // 无法识别的 token 原样输出
    EXPECT_EQ(result, "%X hello");
}

TEST(FormatterTest, SetAndGetPattern)
{
    Formatter fmt;
    fmt.setPattern("--> %v <--");
    EXPECT_EQ(fmt.pattern(), "--> %v <--");

    auto msg = makeMsg(Level::INFO, "test", "center");
    std::string result = fmt.format(msg);
    EXPECT_NE(result.find("--> center <--"), std::string::npos);
}

// ============================================================
// 复杂组合格式测试
// ============================================================

TEST(FormatterTest, ComplexPattern)
{
    Formatter fmt("%Y-%m-%d %H:%M:%S [%^l] [%g] %v");
    auto msg = makeMsg(Level::DEBUG, "complex", "combined test");
    std::string result = fmt.format(msg);

    EXPECT_NE(result.find("DEBUG"), std::string::npos);
    EXPECT_NE(result.find("complex"), std::string::npos);
    EXPECT_NE(result.find("combined test"), std::string::npos);
}

TEST(FormatterTest, AllLevels)
{
    Formatter fmt("[%^l] %v");

    auto trace = makeMsg(Level::TRACE, "t", "t1");
    auto debug = makeMsg(Level::DEBUG, "t", "t2");
    auto info  = makeMsg(Level::INFO,  "t", "t3");
    auto warn  = makeMsg(Level::WARN,  "t", "t4");
    auto error = makeMsg(Level::ERROR, "t", "t5");
    auto crit  = makeMsg(Level::CRITICAL, "t", "t6");

    EXPECT_NE(fmt.format(trace).find("TRACE"), std::string::npos);
    EXPECT_NE(fmt.format(debug).find("DEBUG"), std::string::npos);
    EXPECT_NE(fmt.format(info).find("INFO"),   std::string::npos);
    EXPECT_NE(fmt.format(warn).find("WARN"),   std::string::npos);
    EXPECT_NE(fmt.format(error).find("ERROR"), std::string::npos);
    EXPECT_NE(fmt.format(crit).find("CRITICAL"), std::string::npos);
}
