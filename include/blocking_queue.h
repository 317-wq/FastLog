#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

// 阻塞队列[生产消费模型]

namespace ljt
{
    template <typename T>
    class BlockingQueue
    {
    public:
        BlockingQueue() = default;

        ~BlockingQueue()
        {
            stop();
        }

    public:
        // 存数据
        void push(const T &value)
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push(value);
            }

            cv_.notify_one();
        }

        // 弹数据
        bool pop(T &value)
        {
            // 与wait函数搭配，手动释放锁，重获锁
            std::unique_lock<std::mutex> lock(mutex_);
            // 避免错误唤醒
            cv_.wait(lock, [this]
                     { return stopped_ || !queue_.empty(); });

            if (stopped_ && queue_.empty())
            {
                return false;
            }

            value = std::move(queue_.front());
            queue_.pop();
            return true;
        }

        // 是否停止
        void stop()
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopped_ = true;
            }

            cv_.notify_all(); // 唤醒所有线程
        }

        // queue是否为空
        bool empty()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        // queue中的数据个数
        size_t size()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }

    private:
        std::queue<T> queue_;        // 存储数据
        mutable std::mutex mutex_;   // 保护queue中取/弹数据
        std::condition_variable cv_; // 条件变量唤醒
        bool stopped_{false};        // 是否结束
    };
}