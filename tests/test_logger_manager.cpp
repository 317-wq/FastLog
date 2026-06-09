#include <gtest/gtest.h>

#include "../include/logger_manager.h"
#include "../include/sink/stdout_sink.h"

using namespace ljt;

TEST(LoggerManagerTest, RegisterAndGet)
{
    auto sink =
        std::make_shared<StdoutSink>();

    auto logger =
        std::make_shared<Logger>(
            "main",
            sink);

    LoggerManager::instance()
        .registerLogger(logger);

    auto ptr =
        LoggerManager::instance()
            .getLogger("main");

    EXPECT_NE(ptr, nullptr);

    EXPECT_EQ(
        ptr->name(),
        "main");
}

TEST(LoggerManagerTest, DropLogger)
{
    auto sink =
        std::make_shared<StdoutSink>();

    auto logger =
        std::make_shared<Logger>(
            "drop_test",
            sink);

    auto &manager =
        LoggerManager::instance();

    manager.registerLogger(
        logger);

    EXPECT_TRUE(
        manager.exists(
            "drop_test"));

    manager.dropLogger(
        "drop_test");

    EXPECT_FALSE(
        manager.exists(
            "drop_test"));
}

// ============================================================
// 批量操作测试
// ============================================================

TEST(LoggerManagerTest, SetLevelAll)
{
    auto sink1 = std::make_shared<StdoutSink>();
    auto sink2 = std::make_shared<StdoutSink>();

    auto logger1 = std::make_shared<Logger>("all_lv1", sink1);
    auto logger2 = std::make_shared<Logger>("all_lv2", sink2);

    auto &mgr = LoggerManager::instance();
    mgr.registerLogger(logger1);
    mgr.registerLogger(logger2);

    mgr.setLevelAll(Level::ERROR);

    EXPECT_EQ(logger1->getLevel(), Level::ERROR);
    EXPECT_EQ(logger2->getLevel(), Level::ERROR);

    // 恢复
    mgr.setLevelAll(Level::TRACE);
    mgr.dropLogger("all_lv1");
    mgr.dropLogger("all_lv2");
}

TEST(LoggerManagerTest, FlushAllDoesNotCrash)
{
    auto sink = std::make_shared<StdoutSink>();
    auto logger = std::make_shared<Logger>("flush_all_test", sink);

    auto &mgr = LoggerManager::instance();
    mgr.registerLogger(logger);

    EXPECT_NO_THROW(mgr.flushAll());

    mgr.dropLogger("flush_all_test");
}

TEST(LoggerManagerTest, ApplyAll)
{
    auto sink = std::make_shared<StdoutSink>();
    auto logger = std::make_shared<Logger>("apply_test", sink);

    auto &mgr = LoggerManager::instance();
    mgr.registerLogger(logger);

    // 用 applyAll 设置等级
    mgr.applyAll([](const LoggerPtr &l)
    {
        l->setLevel(Level::WARN);
    });

    EXPECT_EQ(logger->getLevel(), Level::WARN);

    // 恢复
    mgr.dropLogger("apply_test");
}