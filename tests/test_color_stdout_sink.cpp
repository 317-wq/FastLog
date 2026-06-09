#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "../include/sink/color_stdout_sink.h"

using namespace ljt;

/// 重定向 std::cout 的辅助类
class CoutRedirect
{
public:
    CoutRedirect()
    {
        old_ = std::cout.rdbuf(oss_.rdbuf());
    }

    ~CoutRedirect()
    {
        std::cout.rdbuf(old_);
    }

    std::string str() const { return oss_.str(); }

private:
    std::ostringstream oss_;
    std::streambuf* old_;
};

// ============================================================
// 基础功能测试
// ============================================================

TEST(ColorStdoutSinkTest, OutputMessage)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log("hello");

    std::string output = redirect.str();
    EXPECT_NE(output.find("hello"), std::string::npos);
}

TEST(ColorStdoutSinkTest, OutputWithLevel)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::INFO, "colored info");

    std::string output = redirect.str();
    EXPECT_NE(output.find("colored info"), std::string::npos);
}

// ============================================================
// ANSI 颜色码测试
// ============================================================

TEST(ColorStdoutSinkTest, TraceHasWhiteColor)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::TRACE, "trace");

    std::string output = redirect.str();
    EXPECT_NE(output.find("\033[37m"), std::string::npos); // 白色
}

TEST(ColorStdoutSinkTest, DebugHasCyanColor)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::DEBUG, "debug");

    std::string output = redirect.str();
    EXPECT_NE(output.find("\033[36m"), std::string::npos); // 青色
}

TEST(ColorStdoutSinkTest, InfoHasGreenColor)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::INFO, "info");

    std::string output = redirect.str();
    EXPECT_NE(output.find("\033[32m"), std::string::npos); // 绿色
}

TEST(ColorStdoutSinkTest, WarnHasYellowColor)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::WARN, "warn");

    std::string output = redirect.str();
    EXPECT_NE(output.find("\033[33m"), std::string::npos); // 黄色
}

TEST(ColorStdoutSinkTest, ErrorHasRedColor)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::ERROR, "error");

    std::string output = redirect.str();
    EXPECT_NE(output.find("\033[31m"), std::string::npos); // 红色
}

TEST(ColorStdoutSinkTest, CriticalHasRedBgWhite)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::CRITICAL, "critical");

    std::string output = redirect.str();
    EXPECT_NE(output.find("\033[41;37m"), std::string::npos); // 红底白字
}

// ============================================================
// 颜色重置测试
// ============================================================

TEST(ColorStdoutSinkTest, HasResetAfterMessage)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log(Level::INFO, "msg");

    std::string output = redirect.str();
    // ANSI 颜色序列后应当有重置码
    EXPECT_NE(output.find("\033[0m"), std::string::npos);
}

// ============================================================
// 向后兼容测试（无等级调用与 StdoutSink 行为一致）
// ============================================================

TEST(ColorStdoutSinkTest, NoLevelCallOutputsPlainText)
{
    CoutRedirect redirect;
    ColorStdoutSink sink;

    sink.log("plain message");

    std::string output = redirect.str();
    // 无等级调用不应包含 ANSI 颜色码
    EXPECT_EQ(output.find("\033["), std::string::npos);
    EXPECT_NE(output.find("plain message"), std::string::npos);
}

// ============================================================
// Flush 测试
// ============================================================

TEST(ColorStdoutSinkTest, FlushDoesNotCrash)
{
    ColorStdoutSink sink;
    EXPECT_NO_THROW(sink.flush());
}

// ============================================================
// 全等级遍历测试
// ============================================================

TEST(ColorStdoutSinkTest, AllLevelsOutput)
{
    ColorStdoutSink sink;

    // 验证每个等级的彩色输出都包含 ANSI 颜色码
    struct TestCase
    {
        Level level;
        const char* expected_color;
    };

    TestCase cases[] = {
        {Level::TRACE,    "\033[37m"},
        {Level::DEBUG,    "\033[36m"},
        {Level::INFO,     "\033[32m"},
        {Level::WARN,     "\033[33m"},
        {Level::ERROR,    "\033[31m"},
        {Level::CRITICAL, "\033[41;37m"},
    };

    for (const auto &tc : cases)
    {
        CoutRedirect redirect;
        sink.log(tc.level, "test");
        std::string output = redirect.str();
        EXPECT_NE(output.find(tc.expected_color), std::string::npos);
    }
}
