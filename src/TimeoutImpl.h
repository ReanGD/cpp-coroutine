#pragma once
#ifndef TIMEOUT_IMPL_H
#define TIMEOUT_IMPL_H

#include <chrono>
#include "Types.h"


namespace coro
{
    class CTimeoutImpl
    {
    public:
        CTimeoutImpl() = default;
        CTimeoutImpl(const std::chrono::milliseconds& duration);
    public:
        bool IsExpired() const;
        void ThrowIfInit() const;
    public:
        std::chrono::milliseconds m_duration = std::chrono::milliseconds::max();
        std::chrono::time_point<std::chrono::system_clock> m_expiry_time;
    };
}

#endif
