// #include <gtest/gtest.h>
// #include <filesystem>
// #include <fstream>
// #include "../include/logger.h"
// #include "../include/sink/file_sink.h"

// using namespace ljt;

// TEST(LoggerTest, WriteFile)
// {
//     const std::string filename =
//         "logger_test.log";

//     auto sink =
//         std::make_shared<FileSink>(
//             filename);

//     Logger logger(
//         "test_logger",
//         sink);

//     logger.info("hello logger");

//     sink->flush();

//     std::ifstream ifs(filename);

//     ASSERT_TRUE(ifs.is_open());

//     std::string content;

//     std::getline(ifs, content);

//     EXPECT_NE(
//         content.find("hello logger"),
//         std::string::npos);

//     EXPECT_NE(
//         content.find("INFO"),
//         std::string::npos);

//     ifs.close();

//     std::filesystem::remove(
//         filename);
// }

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "../include/logger.h"
#include "../include/sink/file_sink.h"
#include "../include/sink/stdout_sink.h"

using namespace ljt;

TEST(LevelFilterTest, FilterInfo)
{
    const std::string filename =
        "level_filter.log";

    auto sink =
        std::make_shared<FileSink>(
            filename);

    Logger logger(
        "test",
        sink);

    logger.setLevel(
        Level::WARN);

    logger.info("info message");

    logger.warn("warn message");

    sink->flush();

    std::ifstream ifs(filename);

    ASSERT_TRUE(
        ifs.is_open());

    std::string line;

    std::getline(ifs, line);

    EXPECT_EQ(
        line.find("info message"),
        std::string::npos);

    EXPECT_NE(
        line.find("warn message"),
        std::string::npos);

    ifs.close();

    std::filesystem::remove(
        filename);
}

TEST(LevelFilterTest, SetGetLevel)
{
    auto sink =
        std::make_shared<StdoutSink>();

    Logger logger(
        "test",
        sink);

    logger.setLevel(
        Level::ERROR);

    EXPECT_EQ(
        logger.getLevel(),
        Level::ERROR);
}