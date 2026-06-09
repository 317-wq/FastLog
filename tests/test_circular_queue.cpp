#include <gtest/gtest.h>
#include <string>

#include "../include/circular_queue.h"

using namespace ljt;

// ============================================================
// 基础操作测试
// ============================================================

TEST(CircularQueueTest, NewQueueIsEmpty)
{
    CircularQueue<int> q(10);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(CircularQueueTest, PushAndSize)
{
    CircularQueue<int> q(5);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);

    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 3u);
}

TEST(CircularQueueTest, FrontAndPop)
{
    CircularQueue<int> q(5);
    q.push_back(10);
    q.push_back(20);
    q.push_back(30);

    EXPECT_EQ(q.front(), 10);
    q.pop_front();
    EXPECT_EQ(q.front(), 20);
    EXPECT_EQ(q.size(), 2u);
}

TEST(CircularQueueTest, PopAll)
{
    CircularQueue<int> q(3);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);

    q.pop_front();
    q.pop_front();
    q.pop_front();

    EXPECT_TRUE(q.empty());
}

// ============================================================
// 覆盖行为测试
// ============================================================

TEST(CircularQueueTest, OverwriteWhenFull)
{
    CircularQueue<int> q(3); // 容量 3

    q.push_back(1);
    q.push_back(2);
    q.push_back(3);
    EXPECT_EQ(q.size(), 3u);

    // 满了，下一个会覆盖最旧的 (1)
    q.push_back(4);
    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front(), 2); // 1 被覆盖
    EXPECT_EQ(q.at(0), 2);
    EXPECT_EQ(q.at(1), 3);
    EXPECT_EQ(q.at(2), 4);
}

TEST(CircularQueueTest, FullFlag)
{
    CircularQueue<int> q(2);
    EXPECT_FALSE(q.full());

    q.push_back(1);
    EXPECT_FALSE(q.full());

    q.push_back(2);
    EXPECT_TRUE(q.full());

    q.push_back(3); // 覆盖
    EXPECT_TRUE(q.full());
}

// ============================================================
// 随机访问测试
// ============================================================

TEST(CircularQueueTest, AtAccess)
{
    CircularQueue<std::string> q(4);
    q.push_back("a");
    q.push_back("b");
    q.push_back("c");
    q.push_back("d");
    q.push_back("e"); // 覆盖 "a"

    EXPECT_EQ(q.at(0), "b");
    EXPECT_EQ(q.at(1), "c");
    EXPECT_EQ(q.at(2), "d");
    EXPECT_EQ(q.at(3), "e");
    EXPECT_EQ(q.size(), 4u);
}

// ============================================================
// 边界测试
// ============================================================

TEST(CircularQueueTest, ZeroCapacity)
{
    CircularQueue<int> q(0);
    EXPECT_TRUE(q.empty());

    q.push_back(42); // 不应崩溃
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(CircularQueueTest, SingleElement)
{
    CircularQueue<int> q(1);

    EXPECT_FALSE(q.full());

    q.push_back(100);
    EXPECT_EQ(q.size(), 1u);
    EXPECT_EQ(q.front(), 100);
    EXPECT_TRUE(q.full());

    q.push_back(200); // 覆盖
    EXPECT_EQ(q.front(), 200);
    EXPECT_TRUE(q.full());
}

TEST(CircularQueueTest, MoveSemantics)
{
    CircularQueue<std::string> q(3);
    std::string s = "hello";

    q.push_back(std::move(s));
    EXPECT_EQ(q.front(), "hello");
    EXPECT_TRUE(s.empty()); // s 已被移动
}

// ============================================================
// 大量元素测试
// ============================================================

TEST(CircularQueueTest, LargeNumberOfElements)
{
    CircularQueue<int> q(5);
    for (int i = 0; i < 100; ++i)
    {
        q.push_back(i);
    }

    // 只保留最后 5 个
    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.at(0), 95);
    EXPECT_EQ(q.at(1), 96);
    EXPECT_EQ(q.at(2), 97);
    EXPECT_EQ(q.at(3), 98);
    EXPECT_EQ(q.at(4), 99);
}
