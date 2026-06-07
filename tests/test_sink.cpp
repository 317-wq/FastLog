#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "../include/log/file_sink.h"
#include "../include/log/stdout_sink.h"

using namespace ljt;

TEST(SinkTest, StdoutSinkCreate)
{
    StdoutSink sink;

    EXPECT_NO_THROW(
        sink.log("hello"));
}

TEST(SinkTest, FileSinkWrite)
{
    const std::string file =
        "test_log.txt";

    {
        FileSink sink(file);

        sink.log("hello");
        sink.flush();
    }

    std::ifstream ifs(file);

    ASSERT_TRUE(ifs.is_open());

    std::string line;

    std::getline(ifs, line);

    EXPECT_EQ(line, "hello");

    ifs.close();

    std::filesystem::remove(file);
}