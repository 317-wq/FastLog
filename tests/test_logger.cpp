#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../include/logger.h"
#include "../include/sink/file_sink.h"

using namespace ljt;

TEST(LoggerTest, WriteFile)
{
    const std::string filename =
        "logger_test.log";

    auto sink =
        std::make_shared<FileSink>(
            filename);

    Logger logger(
        "test_logger",
        sink);

    logger.info("hello logger");

    sink->flush();

    std::ifstream ifs(filename);

    ASSERT_TRUE(ifs.is_open());

    std::string content;

    std::getline(ifs, content);

    EXPECT_NE(
        content.find("hello logger"),
        std::string::npos);

    EXPECT_NE(
        content.find("INFO"),
        std::string::npos);

    ifs.close();

    std::filesystem::remove(
        filename);
}