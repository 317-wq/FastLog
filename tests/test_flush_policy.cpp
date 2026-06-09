#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <atomic>

#include "../include/flush_policy.h"
#include "../include/sink/file_sink.h"
#include "../include/sink/stdout_sink.h"

using namespace ljt;

/// 用于测试的 Mock Sink：记录 log 和 flush 调用次数
class MockSink : public Sink
{
public:
    void log(Level level, const std::string &message) override
    {
        log_count_++;
        last_level_ = level;
        last_message_ = message;
    }

    void log(const std::string &message) override
    {
        log_count_++;
        last_message_ = message;
    }

    void flush() override
    {
        flush_count_++;
    }

    int log_count() const { return log_count_.load(); }
    int flush_count() const { return flush_count_.load(); }
    Level last_level() const { return last_level_; }
    std::string last_message() const { return last_message_; }

private:
    std::atomic<int> log_count_{0};
    std::atomic<int> flush_count_{0};
    Level last_level_{Level::TRACE};
    std::string last_message_;
};

// ============================================================
// 等级触发 Flush 测试
// ============================================================

TEST(FlushPolicyTest, ErrorTriggersFlush)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR);

    policy.log(Level::ERROR, "an error occurred");

    EXPECT_EQ(mock->log_count(), 1);
    EXPECT_EQ(mock->flush_count(), 1); // ERROR 触发 flush
}

TEST(FlushPolicyTest, CriticalTriggersFlush)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR);

    policy.log(Level::CRITICAL, "critical!");

    EXPECT_EQ(mock->flush_count(), 1);
}

TEST(FlushPolicyTest, InfoDoesNotTriggerFlush)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR);

    policy.log(Level::INFO, "just info");

    EXPECT_EQ(mock->log_count(), 1);
    EXPECT_EQ(mock->flush_count(), 0); // INFO < ERROR, 不触发 flush
}

TEST(FlushPolicyTest, WarnWithDefaultFlushOn)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock); // 默认 Level::ERROR

    policy.log(Level::WARN, "warning");
    EXPECT_EQ(mock->flush_count(), 0); // WARN < ERROR

    policy.log(Level::ERROR, "error");
    EXPECT_EQ(mock->flush_count(), 1); // ERROR >= ERROR
}

TEST(FlushPolicyTest, CustomFlushLevel)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::WARN); // WARN 及以上就 flush

    policy.log(Level::INFO, "info");
    EXPECT_EQ(mock->flush_count(), 0);

    policy.log(Level::WARN, "warn");
    EXPECT_EQ(mock->flush_count(), 1);

    policy.log(Level::ERROR, "error");
    EXPECT_EQ(mock->flush_count(), 2);
}

// ============================================================
// 定时 Flush 测试
// ============================================================

TEST(FlushPolicyTest, PeriodicFlushTriggers)
{
    auto mock = std::make_shared<MockSink>();
    // 等级触发设得很高（CRITICAL），定时触发 50ms
    FlushPolicy policy(mock, Level::CRITICAL, std::chrono::milliseconds(50));

    // 第一次写入：记录时间
    policy.log(Level::INFO, "first");
    EXPECT_EQ(mock->flush_count(), 0);

    // 等待超过 50ms
    std::this_thread::sleep_for(std::chrono::milliseconds(80));

    // 第二次写入时检查：时间已过 50ms，触发定时 flush
    policy.log(Level::INFO, "second");

    EXPECT_EQ(mock->flush_count(), 1);
}

TEST(FlushPolicyTest, PeriodicFlushNotTriggeredTooSoon)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::CRITICAL, std::chrono::milliseconds(500));

    policy.log(Level::INFO, "first");
    EXPECT_EQ(mock->flush_count(), 0);

    // 不等待，立即再写
    policy.log(Level::INFO, "second");
    EXPECT_EQ(mock->flush_count(), 0); // 时间间隔太短，不触发
}

TEST(FlushPolicyTest, PeriodicFlushDisabledByDefault)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR); // interval_ms = 0，禁用定时

    policy.log(Level::INFO, "msg");
    EXPECT_EQ(mock->flush_count(), 0);

    // 等待一段时间后再写
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    policy.log(Level::INFO, "msg2");
    EXPECT_EQ(mock->flush_count(), 0); // 定时 flush 已禁用
}

// ============================================================
// 强制 Flush 测试
// ============================================================

TEST(FlushPolicyTest, ExplicitFlush)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR);

    policy.log(Level::INFO, "msg");
    EXPECT_EQ(mock->flush_count(), 0);

    policy.flush(); // 显式调用
    EXPECT_EQ(mock->flush_count(), 1);
}

// ============================================================
// 委托测试
// ============================================================

TEST(FlushPolicyTest, DelegatesLogToWrappedSink)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR);

    policy.log(Level::DEBUG, "debug msg");

    EXPECT_EQ(mock->log_count(), 1);
    EXPECT_EQ(mock->last_message(), "debug msg");
    EXPECT_EQ(mock->last_level(), Level::DEBUG);
}

TEST(FlushPolicyTest, OneArgLogDelegates)
{
    auto mock = std::make_shared<MockSink>();
    FlushPolicy policy(mock, Level::ERROR);

    policy.log("plain message");

    EXPECT_EQ(mock->log_count(), 1);
    EXPECT_EQ(mock->last_message(), "plain message");
}

// ============================================================
// 集成测试：FlushPolicy 包装 FileSink
// ============================================================

TEST(FlushPolicyTest, WrapsFileSink)
{
    const std::string path = "flush_policy_test.log";

    {
        auto file_sink = std::make_shared<FileSink>(path);
        FlushPolicy policy(file_sink, Level::ERROR);

        policy.log(Level::INFO, "buffered message");
        // INFO 不触发 flush，数据在缓冲区

        policy.log(Level::ERROR, "error triggers flush");
        // ERROR 触发 flush，数据已刷盘
    }

    // 验证文件内容
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());

    std::string content;
    std::string line;
    while (std::getline(ifs, line))
        content += line + "\n";

    EXPECT_NE(content.find("buffered message"), std::string::npos);
    EXPECT_NE(content.find("error triggers flush"), std::string::npos);

    ifs.close();
    std::remove(path.c_str());
}
