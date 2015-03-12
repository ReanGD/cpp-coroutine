#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <memory>
#include <string>
#include <stdint.h>
#include "Types.h"


namespace coro
{
    class ILog;
    class CSchedulerImpl;
    class CScheduler
    {
    public:
        CScheduler(std::shared_ptr<ILog> log, const uint32_t& id, const std::string& name);
        ~CScheduler() = default;
        CScheduler() = delete;
        CScheduler(const CScheduler&) = delete;
        CScheduler& operator=(const CScheduler&) = delete;
    public:
        void Start(const uint32_t& thread_count, tTask init_task);
        void Add(tTask task);
        void Stop();
        void Join();
    public:
        static uint32_t CurrentId(void);
        static bool IsInsideScheduler();
    private:
        std::shared_ptr<CSchedulerImpl> pimpl;
    };
}

#endif
