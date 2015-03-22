#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <chrono>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include "Types.h"
#include "Base.h"


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
        void Join();
    public:
        static uint32_t CurrentId(void);
        static bool IsInsideScheduler();
    public:
        const uint32_t id;
        const std::string name;
    private:
        boost::mutex m_mutex;
        std::vector<boost::thread> m_threads;
        boost::asio::io_service m_service;
        std::unique_ptr<boost::asio::io_service::work> m_work;
    };
}

#endif
