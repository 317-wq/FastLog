#include "../../include/sink/file_sink.h"

#include <stdexcept>
#include <sys/stat.h>

namespace ljt
{
    /// 确保路径的父目录存在（如 logs/app.log → 创建 logs/）
    static void ensureParentDir(const std::string &path)
    {
        std::size_t pos = path.find_last_of('/');
        if (pos != std::string::npos)
        {
            std::string dir = path.substr(0, pos);
            mkdir(dir.c_str(), 0755); // 忽略已存在错误
        }
    }

    FileSink::FileSink(const std::string &filename)
    {
        ensureParentDir(filename);

        // 构造时候直接打开文件
        ofs_.open(filename, std::ios::app);

        if (!ofs_.is_open())
        {
            throw std::runtime_error("open log file failed: " + filename);
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