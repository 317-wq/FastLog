#include <gtest/gtest.h>

#include "../include/formatter.h"

using namespace ljt;

TEST(FormatterTest, FormatMessage)
{
    Formatter formatter;

    LogMessage msg;

    msg.level = Level::INFO;

    msg.payload = "hello";

    msg.time =
        std::chrono::system_clock::now();

    msg.tid =
        std::this_thread::get_id();

    auto result =
        formatter.format(msg);

    EXPECT_NE(
        result.find("INFO"),
        std::string::npos);

    EXPECT_NE(
        result.find("hello"),
        std::string::npos);
}