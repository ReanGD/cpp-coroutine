#pragma once
#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <memory>
#include <chrono>

namespace coro
{
    class CTimeout
    {
    public:
        CTimeout() = delete;
        CTimeout(std::chrono::milliseconds ms);
        ~CTimeout();
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };
}

#endif
