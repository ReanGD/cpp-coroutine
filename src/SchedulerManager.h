#pragma once
#ifndef SCHEDULER_MANAGER_H
#define SCHEDULER_MANAGER_H

#include <memory>
#include <string>
#include <stdint.h>


namespace coro
{
    class ILog;
    class IScheduler;
    class CSchedulerManager
    {
    public:
        CSchedulerManager() = delete;
        CSchedulerManager(std::shared_ptr<ILog> log);
        CSchedulerManager(const CSchedulerManager&) = delete;
        CSchedulerManager& operator=(const CSchedulerManager&) = delete;
        ~CSchedulerManager();
    public:
        void Add(const uint32_t& id, const std::string& name, const uint32_t thread_count);
        std::shared_ptr<IScheduler> Get(const uint32_t& id);
        void StopAll();
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };
}

#endif
