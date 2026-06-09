#include <gtest/gtest.h>
#include <atomic>
#include <cstdio>
#include <fstream>

#include "../include/level.h"
#include "../include/sink/dist_sink.h"
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

    int logCount() const { return log_count_.load(); }
    int flushCount() const { return flush_count_.load(); }
    Level lastLevel() const { return last_level_; }
    std::string lastMessage() const { return last_message_; }

private:
    std::atomic<int> log_count_{0};
    std::atomic<int> flush_count_{0};
    Level last_level_{Level::TRACE};
    std::string last_message_;
};

// ============================================================
// 基础分发测试
// ============================================================

TEST(DistSinkTest, DistributesLogToAllSubSinks)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();
    DistSink dist;
    dist.addSink(m1);
    dist.addSink(m2);

    dist.log("hello");

    EXPECT_EQ(m1->logCount(), 1);
    EXPECT_EQ(m2->logCount(), 1);
    EXPECT_EQ(m1->lastMessage(), "hello");
    EXPECT_EQ(m2->lastMessage(), "hello");
}

TEST(DistSinkTest, DistributesLogWithLevel)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();
    DistSink dist;
    dist.addSink(m1);
    dist.addSink(m2);

    dist.log(Level::ERROR, "error msg");

    EXPECT_EQ(m1->logCount(), 1);
    EXPECT_EQ(m2->logCount(), 1);
    EXPECT_EQ(m1->lastLevel(), Level::ERROR);
    EXPECT_EQ(m2->lastLevel(), Level::ERROR);
}

TEST(DistSinkTest, DistributesFlushToAll)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();
    DistSink dist;
    dist.addSink(m1);
    dist.addSink(m2);

    dist.flush();

    EXPECT_EQ(m1->flushCount(), 1);
    EXPECT_EQ(m2->flushCount(), 1);
}

// ============================================================
// 管理子 Sink 测试
// ============================================================

TEST(DistSinkTest, AddSink)
{
    auto m1 = std::make_shared<MockSink>();
    DistSink dist;

    dist.addSink(m1);
    auto sinks = dist.sinks();
    EXPECT_EQ(sinks.size(), 1u);
}

TEST(DistSinkTest, RemoveSink)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();
    DistSink dist;
    dist.addSink(m1);
    dist.addSink(m2);

    dist.removeSink(m1);

    auto sinks = dist.sinks();
    EXPECT_EQ(sinks.size(), 1u);

    // m2 仍在，m1 已移除
    dist.log("test");
    EXPECT_EQ(m1->logCount(), 0); // 已移除，不再收到
    EXPECT_EQ(m2->logCount(), 1); // 仍在
}

TEST(DistSinkTest, RemoveNonExistentSink)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();
    DistSink dist;
    dist.addSink(m1);

    // 移除不存在的 Sink 不应崩溃
    EXPECT_NO_THROW(dist.removeSink(m2));
    EXPECT_EQ(dist.sinks().size(), 1u);
}

TEST(DistSinkTest, ConstructorWithInitialList)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();

    DistSink dist({m1, m2});

    auto sinks = dist.sinks();
    EXPECT_EQ(sinks.size(), 2u);

    dist.log("test");
    EXPECT_EQ(m1->logCount(), 1);
    EXPECT_EQ(m2->logCount(), 1);
}

// ============================================================
// 空 Sink 列表的鲁棒性测试
// ============================================================

TEST(DistSinkTest, EmptySinkListDoesNotCrash)
{
    DistSink dist;

    EXPECT_NO_THROW(dist.log("msg"));
    EXPECT_NO_THROW(dist.log(Level::INFO, "msg"));
    EXPECT_NO_THROW(dist.flush());
    EXPECT_TRUE(dist.sinks().empty());
}

// ============================================================
// 三个 Sink 的分发测试
// ============================================================

TEST(DistSinkTest, ThreeSinks)
{
    auto m1 = std::make_shared<MockSink>();
    auto m2 = std::make_shared<MockSink>();
    auto m3 = std::make_shared<MockSink>();
    DistSink dist({m1, m2, m3});

    dist.log("broadcast");

    EXPECT_EQ(m1->logCount(), 1);
    EXPECT_EQ(m2->logCount(), 1);
    EXPECT_EQ(m3->logCount(), 1);
}

// ============================================================
// 集成测试：控制台 + 文件 同时输出
// ============================================================

TEST(DistSinkTest, ConsoleAndFile)
{
    const std::string path = "dist_sink_test.log";

    {
        auto console = std::make_shared<StdoutSink>();
        auto file = std::make_shared<FileSink>(path);

        DistSink dist({console, file});

        dist.log(Level::INFO, "to both console and file");
        dist.flush();
    }

    // 验证文件内容
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());
    std::string content;
    std::string line;
    while (std::getline(ifs, line))
        content += line + "\n";

    EXPECT_NE(content.find("to both console and file"), std::string::npos);

    ifs.close();
    std::remove(path.c_str());
}
