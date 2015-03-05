#pragma once
#ifndef SCHEDULER_DEF_H
#define SCHEDULER_DEF_H

#include <memory>
#include "Scheduler.h"

namespace coro
{
    class ILog;
    class CSchedulerDef :
        public IScheduler
    {
    public:
        CSchedulerDef() = delete;
        CSchedulerDef(std::shared_ptr<ILog> log, const uint32_t& id, const std::string& name, const uint32_t& thread_count);
        ~CSchedulerDef();
        CSchedulerDef(const CSchedulerDef&) = delete;
        CSchedulerDef& operator=(const CSchedulerDef&) = delete;
    protected:
        bool IsInit(void) const;
    public:
        void Add(tTask task) override;        
        void Stop() override;
        void Join() override;
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };
}

#endif
