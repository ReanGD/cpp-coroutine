#pragma once
#ifndef SCHEDULER_IMPL_H
#define SCHEDULER_IMPL_H

#include <memory>
#include <vector>
#include <chrono>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include "Types.h"


namespace coro
{
    class ILog;
    class CSchedulerImpl
        : public std::enable_shared_from_this<CSchedulerImpl>
    {
    public:
        CSchedulerImpl(std::shared_ptr<ILog> log, const uint32_t& scheduler_id, const std::string& scheduler_name);
        ~CSchedulerImpl();
        CSchedulerImpl() = delete;
        CSchedulerImpl(const CSchedulerImpl&) = delete;
        CSchedulerImpl& operator=(const CSchedulerImpl&) = delete;
    protected:
        void MainLoop(uint32_t thread_number);
        boost::thread CreateThread(uint32_t thread_number, tTask init_task);
    public:
        void Start(const uint32_t& thread_count, tTask init_task);
        void Add(tTask task);
        void AddTimeout(tTask task, const std::chrono::milliseconds& duration);
        void Stop();
        void Join();
    public:
        uint32_t id;
        std::string name;
    private:
        boost::mutex m_mutex;
        std::shared_ptr<ILog> m_log;
        std::vector<boost::thread> m_threads;
        boost::asio::io_service m_service;
        std::unique_ptr<boost::asio::io_service::work> m_work;
    };
}

#endif
