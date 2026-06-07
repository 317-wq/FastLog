#pragma once

namespace ljt
{
    enum class Level : std::uint8_t
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };
    inline bool operator<(Level l, Level r) noexcept
    {
        return static_cast<int>(l) < static_cast<int>(r);
    }
}