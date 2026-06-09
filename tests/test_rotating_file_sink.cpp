#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>

#include "../include/sink/rotating_file_sink.h"

using namespace ljt;

/// 判断文件是否存在
static bool fileExists(const std::string &path)
{
    std::ifstream ifs(path);
    return ifs.good();
}

/// 临时文件清理辅助类
struct TempFile
{
    std::string path;
    explicit TempFile(std::string p) : path(std::move(p)) {}
    ~TempFile()
    {
        std::remove(path.c_str());
        for (int i = 1; i <= 10; ++i)
        {
            std::string r = path + "." + std::to_string(i);
            std::remove(r.c_str());
        }
    }
};

/// 读取文件全部内容
static std::string readFile(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return "";
    std::string content;
    std::string line;
    while (std::getline(ifs, line))
        content += line + "\n";
    return content;
}

/// 统计文件行数
static int countLines(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return 0;
    int n = 0;
    std::string dummy;
    while (std::getline(ifs, dummy)) ++n;
    return n;
}

// ============================================================
// 基础功能测试
// ============================================================

TEST(RotatingFileSinkTest, BasicWrite)
{
    TempFile f("rotating_basic.log");

    RotatingFileSink sink(f.path, 1024 * 1024); // 1MB, 远大于写入量

    sink.log("hello rotating");
    sink.flush();

    std::string content = readFile(f.path);
    EXPECT_NE(content.find("hello rotating"), std::string::npos);
}

TEST(RotatingFileSinkTest, MultipleWrites)
{
    TempFile f("rotating_multi.log");

    RotatingFileSink sink(f.path, 1024 * 1024);

    for (int i = 0; i < 100; ++i)
    {
        sink.log("line " + std::to_string(i));
    }

    sink.flush();
    EXPECT_EQ(countLines(f.path), 100);
}

// ============================================================
// 切分功能测试
// ============================================================

TEST(RotatingFileSinkTest, RotationBySize)
{
    TempFile f("rotate_size.log");

    // 设置很小的 max_size（约 100 字节），确保写入几条就触发切分
    RotatingFileSink sink(f.path, 100, 5);

    // 每条消息约 20 字节，写 30 条总计远超 100 字节
    for (int i = 0; i < 30; ++i)
    {
        sink.log("message number " + std::to_string(i));
    }

    sink.flush();

    // 当前文件应该有数据
    int current_lines = countLines(f.path);
    EXPECT_GT(current_lines, 0);

    // 应该至少创建了 rotate_size.log.1 这个旧文件
    bool has_rotated = fileExists(f.path + ".1");
    EXPECT_TRUE(has_rotated);
}

TEST(RotatingFileSinkTest, OldFilesCreated)
{
    TempFile f("rotate_old.log");

    RotatingFileSink sink(f.path, 80, 5);

    for (int i = 0; i < 100; ++i)
    {
        sink.log("msg " + std::to_string(i) + " some padding text");
    }

    sink.flush();

    EXPECT_TRUE(fileExists(f.path + ".1"));
}

// ============================================================
// max_files 限制测试
// ============================================================

TEST(RotatingFileSinkTest, MaxFilesLimit)
{
    TempFile f("rotate_max.log");

    const std::size_t MAX = 3;
    RotatingFileSink sink(f.path, 60, MAX);

    // 写入足够多，触发远超过 MAX 次的切分
    for (int i = 0; i < 200; ++i)
    {
        sink.log("abcdefghijklmnopqrstuvwxyz");
    }

    sink.flush();

    // 旧文件索引不应超过 MAX
    EXPECT_FALSE(fileExists(f.path + "." + std::to_string(MAX + 1)));
    EXPECT_FALSE(fileExists(f.path + "." + std::to_string(MAX + 2)));
}

// ============================================================
// 参数校验测试
// ============================================================

TEST(RotatingFileSinkTest, ZeroMaxSizeThrows)
{
    EXPECT_THROW(
        RotatingFileSink sink("dummy.log", 0, 3),
        std::invalid_argument);
}

TEST(RotatingFileSinkTest, ValidConstruction)
{
    TempFile f("rotate_valid.log");
    EXPECT_NO_THROW(
        RotatingFileSink sink(f.path, 1024, 5));
}

// ============================================================
// 带等级的 log 接口测试
// ============================================================

TEST(RotatingFileSinkTest, LogWithLevel)
{
    TempFile f("rotate_level.log");

    RotatingFileSink sink(f.path, 1024 * 1024);

    sink.log(Level::ERROR, "error message");
    sink.log(Level::INFO, "info message");
    sink.flush();

    std::string content = readFile(f.path);
    EXPECT_NE(content.find("error message"), std::string::npos);
    EXPECT_NE(content.find("info message"), std::string::npos);
}

// ============================================================
// Flush 测试
// ============================================================

TEST(RotatingFileSinkTest, FlushDoesNotCrash)
{
    TempFile f("rotate_flush.log");

    RotatingFileSink sink(f.path, 1024 * 1024);
    sink.log("before flush");
    EXPECT_NO_THROW(sink.flush());
}

// ============================================================
// 析构测试
// ============================================================

TEST(RotatingFileSinkTest, DestructorClosesFile)
{
    TempFile f("rotate_dtor.log");

    {
        RotatingFileSink sink(f.path, 1024);
        sink.log("test dtor");
    }

    std::ifstream ifs(f.path);
    EXPECT_TRUE(ifs.is_open());
}
