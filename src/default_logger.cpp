#include "../include/default_logger.h"

namespace ljt
{
    Logger& defaultLogger()
    {
        // C++11 保证静态局部变量初始化是线程安全的
        static Logger logger("default", std::make_shared<StdoutSink>());
        return logger;
    }
}
