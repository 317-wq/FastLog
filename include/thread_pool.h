#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ljt
{
    class ThreadPool
    {
    public:
        using Task = std::function<void()>;

    public:
        // 单参数构造
        explicit ThreadPool(std::size_t thread_num);

        ~ThreadPool();

    public:
        // 运行线程池，就是执行任务
        void start();
        // 终止线程池
        void stop();
        // 提交任务
        void submit(Task task);

    private:
        // 工作线程绑定的函数
        void worker();

    private:
        std::size_t thread_num_;           // 线程数量
        std::vector<std::thread> workers_; // 工作线程
        std::queue<Task> tasks_;           // 任务队列
        std::mutex mutex_;                 // 保护存取任务的锁
        std::condition_variable cv_;       // 条件变量
        std::atomic<bool> running_{false}; // 是否运行
    };
}