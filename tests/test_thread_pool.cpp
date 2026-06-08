#include <atomic>

#include <gtest/gtest.h>

#include "../include/thread_pool.h"

using namespace ljt;

TEST(ThreadPoolTest, ExecuteTask)
{
    ThreadPool pool(2);

    pool.start();

    std::atomic<int> value{0};

    pool.submit(
        [&]()
        {
            value = 100;
        });

    std::this_thread::sleep_for(
        std::chrono::milliseconds(
            100));

    EXPECT_EQ(
        value.load(),
        100);

    pool.stop();
}

TEST(ThreadPoolTest, MultiTask)
{
    ThreadPool pool(4);

    pool.start();

    std::atomic<int> counter{0};

    for (int i = 0;
         i < 100;
         ++i)
    {
        pool.submit(
            [&]()
            {
                ++counter;
            });
    }

    pool.stop();

    EXPECT_EQ(
        counter.load(),
        100);
}

TEST(ThreadPoolTest, StopPool)
{
    ThreadPool pool(2);

    pool.start();

    EXPECT_NO_THROW(
        pool.stop());
}
