#pragma once

#include <string>
#include "log_message.h"

namespace ljt
{
    class Formatter
    {
    public:
        // 格式化输出[LogMessage -> string]
        std::string format(const LogMessage &msg);

    private:
        // 日志等级 -> 字符串
        std::string levelToString(Level level);
    };
}