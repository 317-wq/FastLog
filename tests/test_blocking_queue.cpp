#include <gtest/gtest.h>
#include <thread>

#include "../include/blocking_queue.h"

using namespace ljt;

TEST(BlockingQueueTest, PushPop)
{
    BlockingQueue<int> queue;

    queue.push(100);

    int value = 0;

    bool result =
        queue.pop(value);

    EXPECT_TRUE(result);

    EXPECT_EQ(value, 100);
}

TEST(BlockingQueueTest, Size)
{
    BlockingQueue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(
        queue.size(),
        3);
}

TEST(BlockingQueueTest, Empty)
{
    BlockingQueue<int> queue;

    EXPECT_TRUE(
        queue.empty());

    queue.push(1);

    EXPECT_FALSE(
        queue.empty());
}

TEST(BlockingQueueTest, Stop)
{
    BlockingQueue<int> queue;

    queue.stop();

    int value;

    bool result =
        queue.pop(value);

    EXPECT_FALSE(result);
}

TEST(BlockingQueueTest, ProducerConsumer)
{
    BlockingQueue<int> queue;

    std::thread producer(
        [&]()
        {
            for (int i = 0;
                 i < 100;
                 ++i)
            {
                queue.push(i);
            }
        });

    int count = 0;

    std::thread consumer(
        [&]()
        {
            int value;

            while (count < 100)
            {
                if (queue.pop(value))
                {
                    ++count;
                }
            }
        });

    producer.join();

    consumer.join();

    EXPECT_EQ(count, 100);
}