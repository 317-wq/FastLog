#pragma once

#include "sink.h"
#include "../level.h"

#include <fstream>
#include <mutex>
#include <string>

namespace ljt
{
    /// 按文件大小自动切分的文件 Sink
    ///
    /// 切分规则：
    ///   当当前日志文件大小超过 max_size 时，自动切分：
    ///     app.log      -> app.1.log
    ///     app.1.log    -> app.2.log
    ///     ...
    ///     app.N-1.log  -> app.N.log
    ///     app.N.log    被丢弃
    ///   最多保留 max_files 个旧文件

    class RotatingFileSink : public Sink
    {
    public:
        /// @param base_path   日志文件基础路径（如 "app.log"）
        /// @param max_size    单个文件最大字节数
        /// @param max_files   最多保留的旧文件数（不含当前文件），默认 3
        RotatingFileSink(const std::string &base_path,
                         std::size_t max_size,
                         std::size_t max_files = 3);

        ~RotatingFileSink() override;

        // 带等级的写日志（内部调用无等级版本）
        void log(Level level, const std::string &message) override;

        // 写日志
        void log(const std::string &message) override;

        // 刷新文件缓冲区
        void flush() override;

    private:
        // 执行文件切分
        void rotate();

        // 生成第 index 个旧文件的路径 (index >= 1)
        std::string rotatedPath(std::size_t index) const;

        // 打开当前日志文件
        void openCurrentFile();

        std::string base_path_;
        std::size_t max_size_;
        std::size_t max_files_;
        std::ofstream ofs_;
        std::size_t current_size_{0}; // 当前文件已写入字节数
        std::mutex mutex_;
    };
}
