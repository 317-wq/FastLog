#include "../include/sink/file_sink.h"
#include <stdexcept>

namespace ljt
{
    FileSink::FileSink(const std::string &filename)
    {
        // 构造时候直接打开文件
        ofs_.open(filename, std::ios::app);

        if (!ofs_.is_open())
        {
            throw std::runtime_error("open log file failed");
        }
    }

    FileSink::~FileSink()
    {
        // 先将文件里面的日志全部刷新出去
        flush();
        ofs_.close();
    }

    // 输出日志
    void FileSink::log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ofs_ << message << '\n';
    }

    // 刷新文件缓冲区
    void FileSink::flush()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ofs_.flush();
    }
}