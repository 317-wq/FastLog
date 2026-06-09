#pragma once

#include <string>
#include "log_message.h"

namespace ljt
{
    /// 日志格式化器，支持自定义格式 pattern
    ///
    /// 格式说明：
    ///   %Y    - 年（4 位）
    ///   %m    - 月（01-12）
    ///   %d    - 日（01-31）
    ///   %H    - 时（00-23）
    ///   %M    - 分（00-59）
    ///   %S    - 秒（00-59）
    ///   %l    - 日志等级（小写）
    ///   %^l   - 日志等级（大写）
    ///   %t    - 线程 ID
    ///   %g    - Logger 名称
    ///   %v    - 日志正文（payload）
    ///   %@    - [源文件:行号]（无源文件信息时为空）
    ///   %%    - 百分号本身
    ///
    /// 默认格式： "[%Y-%m-%d %H:%M:%S][%^l][%t][%@] %v"

    class Formatter
    {
    public:
        /// 默认格式串
        static const char* defaultPattern();

        /// 使用默认格式构造
        Formatter();

        /// 使用自定义格式构造
        explicit Formatter(const std::string &pattern);

        /// 设置自定义格式
        void setPattern(const std::string &pattern);

        /// 获取当前格式串
        const std::string &pattern() const;

        /// 格式化输出 [LogMessage -> string]
        std::string format(const LogMessage &msg) const;

    private:
        // 日志等级 -> 字符串
        static std::string levelToString(Level level, bool upper = true);

        std::string pattern_;
    };
}
