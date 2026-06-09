#include "../include/default_logger.h"
#include "../include/sink/color_stdout_sink.h"

namespace ljt
{
    Logger& defaultLogger()
    {
        // C++11 保证静态局部变量初始化是线程安全的
        // 使用彩色输出到 stdout，更直观
        static Logger logger("default", std::make_shared<ColorStdoutSink>());
        return logger;
    }
}
