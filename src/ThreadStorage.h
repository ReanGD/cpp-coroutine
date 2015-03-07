#pragma once
#ifndef THREAD_STORAGE_H
#define THREAD_STORAGE_H

#include <memory>


namespace coro
{
    class CContextImpl;
    class CThreadStorage
    {
    public:
        static std::shared_ptr<CContextImpl> SetContext(std::shared_ptr<CContextImpl> context);
        static std::shared_ptr<CContextImpl> GetContext(void);
    };
}

#endif
