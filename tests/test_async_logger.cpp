#include <filesystem>
#include <fstream>
#include <thread>

#include <gtest/gtest.h>

#include "../include/async_logger.h"
#include "../include/macros.h"
#include "../include/sink/file_sink.h"
#include "../include/sink/stdout_sink.h"

using namespace ljt;

TEST(AsyncLoggerTest, BasicWrite)
{
    const std::string file =
        "async_basic.log";

    {
        auto sink =
            std::make_shared<FileSink>(file);

        AsyncLogger logger(
            "async",
            sink,
            1);

        LOG_INFO(logger, "hello async");

        std::this_thread::sleep_for(
            std::chrono::milliseconds(200));

        sink->flush();
    }

    std::ifstream ifs(file);

    ASSERT_TRUE(ifs.is_open());

    std::string line;

    std::getline(ifs, line);

    EXPECT_NE(
        line.find("hello async"),
        std::string::npos);

    ifs.close();

    std::filesystem::remove(file);
}

TEST(AsyncLoggerTest, LevelFilter)
{
    const std::string file =
        "async_filter.log";

    {
        auto sink =
            std::make_shared<FileSink>(file);

        AsyncLogger logger(
            "async",
            sink);

        logger.setLevel(
            Level::WARN);

        LOG_INFO(logger, "info");

        LOG_ERROR(logger, "error");

        std::this_thread::sleep_for(
            std::chrono::milliseconds(200));

        sink->flush();
    }

    std::ifstream ifs(file);

    ASSERT_TRUE(ifs.is_open());

    std::string content;
    std::string line;

    while (std::getline(ifs, line))
    {
        content += line;
    }

    EXPECT_EQ(
        content.find("info"),
        std::string::npos);

    EXPECT_NE(
        content.find("error"),
        std::string::npos);

    ifs.close();

    std::filesystem::remove(file);
}

TEST(AsyncLoggerTest, MassiveLog)
{
    const std::string file =
        "async_massive.log";

    constexpr int N = 10000;

    {
        auto sink =
            std::make_shared<FileSink>(file);

        AsyncLogger logger(
            "async",
            sink,
            2);

        for (int i = 0; i < N; ++i)
        {
            LOG_INFO(logger, "message");
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(2));

        sink->flush();
    }

    std::ifstream ifs(file);

    ASSERT_TRUE(ifs.is_open());

    int lines = 0;

    std::string line;

    while (std::getline(ifs, line))
    {
        ++lines;
    }

    EXPECT_EQ(lines, N);

    ifs.close();

    std::filesystem::remove(file);
}

TEST(AsyncLoggerTest, MultiThreadWrite)
{
    const std::string file =
        "async_multi.log";

    constexpr int THREAD_NUM = 8;

    constexpr int LOG_NUM = 1000;

    {
        auto sink =
            std::make_shared<FileSink>(file);

        AsyncLogger logger(
            "async",
            sink,
            2);

        std::vector<std::thread> workers;

        for (int i = 0;
             i < THREAD_NUM;
             ++i)
        {
            workers.emplace_back(
                [&]()
                {
                    for (int j = 0;
                         j < LOG_NUM;
                         ++j)
                    {
                        LOG_INFO(logger, "multi");
                    }
                });
        }

        for (auto &t : workers)
        {
            t.join();
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(2));

        sink->flush();
    }

    std::ifstream ifs(file);

    ASSERT_TRUE(ifs.is_open());

    int count = 0;

    std::string line;

    while (std::getline(ifs, line))
    {
        ++count;
    }

    EXPECT_EQ(
        count,
        THREAD_NUM * LOG_NUM);

    ifs.close();

    std::filesystem::remove(file);
}

TEST(AsyncLoggerTest, DestroyLogger)
{
    auto sink =
        std::make_shared<StdoutSink>();

    EXPECT_NO_THROW(
        {
            AsyncLogger logger(
                "async",
                sink,
                2);

            LOG_INFO(logger, "hello");
        });
}