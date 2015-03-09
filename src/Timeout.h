#pragma once
#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdint.h>
#include <chrono>

namespace coro
{
    class CTimeout
    {
    public:
        CTimeout() = delete;
        CTimeout(const std::chrono::milliseconds& duration);
        ~CTimeout();
    private:
        uint32_t m_id;
    };
}

#endif
