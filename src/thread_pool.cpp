#include "../include/thread_pool.h"

namespace ljt
{

    ThreadPool::ThreadPool(std::size_t thread_num)
        : thread_num_(thread_num)
    {
    }

    ThreadPool::~ThreadPool()
    {
        stop(); // RAII 自动释放
    }

    void ThreadPool::start()
    {
        if (running_)
        {
            return;
        }

        running_ = true;

        for (std::size_t i = 0; i < thread_num_; ++i)
        {
            // thread(&ThreadPool::worker, this)相当于
            workers_.emplace_back(&ThreadPool::worker, this);
        }
    }

    void ThreadPool::stop()
    {
        if (!running_)
        {
            return;
        }

        running_ = false;

        cv_.notify_all();

        for (auto &worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }

        workers_.clear();
    }

    void ThreadPool::worker()
    {
        while (true)
        {
            Task task;

            {
                std::unique_lock<std::mutex> lock(mutex_);

                cv_.wait(lock, [this]
                         { return !running_ || !tasks_.empty(); });

                if (!running_ && tasks_.empty())
                {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    void ThreadPool::submit(Task task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }

        cv_.notify_one();
    }
}