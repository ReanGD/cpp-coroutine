#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <boost/chrono.hpp>
#include "Base.h"
#include "Types.h"


namespace coro
{
    class CScheduler : public CBase
    {
    public:
        CScheduler(std::shared_ptr<ILog> log, const uint32_t& scheduler_id, const std::string& scheduler_name);
        ~CScheduler();
        CScheduler() = delete;
        CScheduler(const CScheduler&) = delete;
        CScheduler& operator=(const CScheduler&) = delete;
    private:
        void MainLoop(const uint32_t& thread_number, const tTask& init_task);
    public:
        void Start(const uint32_t& thread_count, const tTask& init_task);
        void Add(tTask task);
        void AddTimeout(tTask task, const std::chrono::milliseconds& duration);
        void Stop();
        void JoinUntil(const boost::chrono::steady_clock::time_point& until_time);
    public:
        const uint32_t id;
        const std::string name;
    private:
        struct impl; std::unique_ptr<impl> pimpl;
    };
}

#endif
