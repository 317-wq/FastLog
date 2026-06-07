#include "../include/sink/stdout_sink.h"

#include <iostream>

namespace ljt
{
    void StdoutSink::log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << message << '\n';
    }

    void StdoutSink::flush()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout.flush();
    }
}