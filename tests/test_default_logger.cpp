#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "../include/default_logger.h"
#include "../include/sink/stdout_sink.h"

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

TEST(DefaultLoggerTest, DefaultLoggerNotNull)
{
    Logger& logger = defaultLogger();
    EXPECT_EQ(logger.name(), "default");
}

TEST(DefaultLoggerTest, DefaultLoggerIsSingleton)
{
    Logger& a = defaultLogger();
    Logger& b = defaultLogger();
    EXPECT_EQ(&a, &b);
}

TEST(DefaultLoggerTest, SetAndGetLevel)
{
    setDefaultLevel(Level::ERROR);
    EXPECT_EQ(defaultLogger().getLevel(), Level::ERROR);

    // 恢复默认等级
    setDefaultLevel(Level::TRACE);
}

// ============================================================
// 宏功能测试
// ============================================================

TEST(DefaultLoggerTest, FLogInfoOutput)
{
    CoutRedirect redirect;

    FLOG_INFO("hello default logger");

    std::string output = redirect.str();
    EXPECT_NE(output.find("hello default logger"), std::string::npos);
    EXPECT_NE(output.find("INFO"), std::string::npos);
}

TEST(DefaultLoggerTest, FLogAllLevels)
{
    CoutRedirect redirect;

    FLOG_TRACE("trace msg");
    FLOG_DEBUG("debug msg");
    FLOG_INFO("info msg");
    FLOG_WARN("warn msg");
    FLOG_ERROR("error msg");
    FLOG_CRITICAL("critical msg");

    std::string output = redirect.str();

    EXPECT_NE(output.find("TRACE"), std::string::npos);
    EXPECT_NE(output.find("DEBUG"), std::string::npos);
    EXPECT_NE(output.find("INFO"), std::string::npos);
    EXPECT_NE(output.find("WARN"), std::string::npos);
    EXPECT_NE(output.find("ERROR"), std::string::npos);
    EXPECT_NE(output.find("CRITICAL"), std::string::npos);
}

// ============================================================
// 等级过滤测试
// ============================================================

TEST(DefaultLoggerTest, FLogLevelFilter)
{
    setDefaultLevel(Level::WARN);

    {
        CoutRedirect redirect;
        FLOG_INFO("should be filtered");
        FLOG_DEBUG("should be filtered");
        FLOG_TRACE("should be filtered");

        std::string output = redirect.str();
        EXPECT_EQ(output.find("should be filtered"), std::string::npos);
    }

    {
        CoutRedirect redirect;
        FLOG_WARN("warn msg");
        FLOG_ERROR("error msg");
        FLOG_CRITICAL("critical msg");

        std::string output = redirect.str();
        EXPECT_NE(output.find("warn msg"), std::string::npos);
        EXPECT_NE(output.find("error msg"), std::string::npos);
        EXPECT_NE(output.find("critical msg"), std::string::npos);
    }

    // 恢复默认等级
    setDefaultLevel(Level::TRACE);
}
