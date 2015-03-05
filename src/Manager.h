#pragma once
#ifndef MANAGER_H
#define MANAGER_H

#include "Singleton.h"

namespace coro
{
    class ILog;
    class CContextManager;
    class CSchedulerManager;

    class CManager
    {
    public:
        CManager();
        ~CManager() = default;
    public:
        CManager(const CManager&) = delete;
        CManager& operator=(const CManager&) = delete;
    public:
        void Init(std::shared_ptr<ILog> log);
        std::shared_ptr<CSchedulerManager> ShedulerManager(void);
        std::shared_ptr<CContextManager> ContextManager(void);
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };

    using Get = coro::CSingleton<CManager>;
}

#endif
