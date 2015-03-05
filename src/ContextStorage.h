#pragma once
#ifndef CONTEXT_STORAGE_H
#define CONTEXT_STORAGE_H

#include "Context.h"

namespace coro
{
    class CContextStorage
    {
    public:
        CContextStorage();
        ~CContextStorage();
    public:
        CContextStorage(const CContextStorage&) = delete;
        CContextStorage& operator=(const CContextStorage&) = delete;
    public:
        uint32_t Add(std::weak_ptr<CContext::impl> ctx);
        void Resume(uint32_t id);
        void Remove(uint32_t id);
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };
}

#endif
