#pragma once

#include <vector>
#include <cstddef>

// 固定大小的环形队列（基于 std::vector 实现）
// 满时覆盖最旧元素

namespace ljt
{
    template <typename T>
    class CircularQueue
    {
    public:
        CircularQueue() = default;

        /// 构造指定容量的环形队列（内部预留 1 个空位作为满标记）
        explicit CircularQueue(std::size_t max_items)
            : max_items_(max_items + 1), v_(max_items_)
        {
        }

        /// 入队 -> 万能引用 + 完美转发
        template<typename U>
        void push_back(U &&item)
        {
            if (max_items_ == 0) return;

            v_[tail_] = std::forward<U>(item);
            tail_ = (tail_ + 1) % max_items_;

            if (tail_ == head_)
            {
                head_ = (head_ + 1) % max_items_;
            }
        }

        /// 获取队头元素引用
        const T &front() const { return v_[head_]; }
        T &front() { return v_[head_]; }

        /// 弹出队头
        void pop_front()
        {
            head_ = (head_ + 1) % max_items_;
        }

        /// 当前元素个数
        std::size_t size() const
        {
            if (tail_ >= head_)
                return tail_ - head_;
            else
                return max_items_ - (head_ - tail_);
        }

        /// 是否为空
        bool empty() const { return tail_ == head_; }

        /// 是否已满
        bool full() const
        {
            if (max_items_ == 0) return false;
            return ((tail_ + 1) % max_items_) == head_;
        }

        /// 按索引访问（0 = 队头）
        const T &at(std::size_t i) const
        {
            return v_[(head_ + i) % max_items_];
        }

    private:
        std::size_t max_items_{0};
        std::size_t head_{0};
        std::size_t tail_{0};
        std::vector<T> v_;
    };
}
