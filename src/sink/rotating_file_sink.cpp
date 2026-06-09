#include "../../include/sink/rotating_file_sink.h"

#include <cstdio>
#include <fstream>
#include <stdexcept>

namespace ljt
{
    /// 判断文件是否存在
    static bool fileExists(const std::string &path)
    {
        std::ifstream ifs(path);
        return ifs.good();
    }

    RotatingFileSink::RotatingFileSink(const std::string &base_path,
                                       std::size_t max_size,
                                       std::size_t max_files)
        : base_path_(base_path),
          max_size_(max_size),
          max_files_(max_files)
    {
        if (max_size == 0)
        {
            throw std::invalid_argument("RotatingFileSink: max_size must be > 0");
        }

        openCurrentFile();
    }

    RotatingFileSink::~RotatingFileSink()
    {
        flush();
        ofs_.close();
    }

    std::string RotatingFileSink::rotatedPath(std::size_t index) const
    {
        return base_path_ + "." + std::to_string(index);
    }

    void RotatingFileSink::openCurrentFile()
    {
        ofs_.open(base_path_, std::ios::app);
        if (!ofs_.is_open())
        {
            throw std::runtime_error("RotatingFileSink: failed to open " + base_path_);
        }
        current_size_ = static_cast<std::size_t>(ofs_.tellp());
    }

    void RotatingFileSink::rotate()
    {
        ofs_.close();

        // 删除最旧的轮转文件 (index == max_files_)
        std::string oldest = rotatedPath(max_files_);
        if (fileExists(oldest))
        {
            std::remove(oldest.c_str());
        }

        // 从后往前重命名: app.(N-1).log -> app.N.log, ..., app.log -> app.1.log
        for (std::size_t i = max_files_; i > 1; --i)
        {
            std::string src = rotatedPath(i - 1);
            std::string dst = rotatedPath(i);
            if (fileExists(src))
            {
                std::rename(src.c_str(), dst.c_str());
            }
        }

        // 当前文件重命名为 app.1.log
        if (max_files_ >= 1)
        {
            std::rename(base_path_.c_str(), rotatedPath(1).c_str());
        }

        // 打开新的空日志文件
        openCurrentFile();
    }

    void RotatingFileSink::log(Level level, const std::string &message)
    {
        // 带等级信息时直接委托给无等级版本
        log(message);
    }

    void RotatingFileSink::log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // 先检查：如果本条消息会导致超出大小限制，则先切分再写入
        // 保证当前文件始终包含最新数据，避免切分后文件为空
        if (current_size_ > 0 && current_size_ + message.size() + 1 > max_size_)
        {
            rotate();
        }

        ofs_ << message << '\n';
        current_size_ += message.size() + 1; // +1 为换行符
    }

    void RotatingFileSink::flush()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ofs_.flush();
    }
}
