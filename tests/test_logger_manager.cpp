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