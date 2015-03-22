#pragma once
#ifndef SCHEDULER_MANAGER_H
#define SCHEDULER_MANAGER_H

#include <memory>
#include <chrono>
#include <string>
#include <stdint.h>
#include "Types.h"

namespace coro
{
    class ILog;
    class CSchedulerManager
    {
    public:
        CSchedulerManager() = delete;
        CSchedulerManager(std::shared_ptr<ILog> log);
        CSchedulerManager(const CSchedulerManager&) = delete;
        CSchedulerManager& operator=(const CSchedulerManager&) = delete;
        ~CSchedulerManager();
    public:
        void Create(const uint32_t& id, const std::string& name, const uint32_t thread_count, tTask init_task);
        void Add(const uint32_t& id, tTask task);
        void AddTimeout(tTask task, const std::chrono::milliseconds& duration);
        void Stop();
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };
}

#endif
