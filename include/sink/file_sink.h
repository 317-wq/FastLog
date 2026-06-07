#pragma once

#include "sink.h"

#include <fstream>
#include <mutex>
#include <string>

namespace ljt
{
    class FileSink : public Sink
    {
    public:
        // 单参数构造，避免隐式类型转换
        explicit FileSink(const std::string &filename);

        ~FileSink() override;

        // 输出日志
        void log(const std::string &message) override;

        // 刷新文件输出缓冲区
        void flush() override;

    private:
        std::ofstream ofs_; // 输出到哪里[文件]
        std::mutex mutex_; // 保护文件输出锁
    };
}