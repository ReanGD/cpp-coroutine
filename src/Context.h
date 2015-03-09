#pragma once
#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>
#include "Types.h"

namespace coro
{
    class ILog;
    class CContextImpl;
    class CContext
    {
    public:
        CContext(std::shared_ptr<ILog> log, const uint32_t& id);
        ~CContext() = default;
        CContext() = delete;
        CContext(const CContext&) = delete;
        CContext& operator=(const CContext&) = delete;
    public:
        bool Start(tTask task, const size_t stack_size);
        bool Resume();
    public:
        static uint32_t CurrentId();
        static void YieldImpl();
        static bool IsInsideCoroutine();
    private:
        std::shared_ptr<CContextImpl> pimpl;
    };

}

#endif
