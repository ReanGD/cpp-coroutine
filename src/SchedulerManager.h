#pragma once
#ifndef SCHEDULER_MANAGER_H
#define SCHEDULER_MANAGER_H

#include <chrono>
#include "Base.h"
#include "Types.h"

namespace coro
{
    class CSchedulerManager : public CBase
    {
    public:
        CSchedulerManager(std::shared_ptr<ILog> log);
        ~CSchedulerManager();
        CSchedulerManager() = delete;
        CSchedulerManager(const CSchedulerManager&) = delete;
        CSchedulerManager& operator=(const CSchedulerManager&) = delete;
    public:
        void Create(const uint32_t& id, const std::string& name, const uint32_t thread_count, tTask init_task);
        void Add(const uint32_t& id, tTask task);
        void AddTimeout(tTask task, const std::chrono::milliseconds& duration);
        void Stop();
    public:
        static uint32_t CurrentId(void);
    private:
        struct impl; std::unique_ptr<impl> pimpl;
    };
}

#endif
