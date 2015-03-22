#pragma once
#ifndef THREAD_STORAGE_H
#define THREAD_STORAGE_H

#include <memory>
#include <stdint.h>


namespace coro
{
    class CContextImpl;
    class CThreadStorage
    {
    public:
        static std::shared_ptr<CContextImpl> SetContext(std::shared_ptr<CContextImpl> context);
        static std::shared_ptr<CContextImpl> GetContext(void);
        static uint32_t SetSchedulerId(uint32_t scheduler_id);
        static uint32_t GetSchedulerId(void);
    };
}

#endif
