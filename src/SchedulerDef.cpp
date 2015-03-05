#include "stdafx.h"
#include "SchedulerDef.h"

#include <vector>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/asio/io_service.hpp>

#include "Log.h"

struct coro::CSchedulerDef::impl
{
    void MainLoop(uint32_t thread_number)
    {
        while (!m_service.stopped())
        {
            try
            {
                m_service.run_one();
            }
            catch (std::exception& e)
            {
                auto msg = "Task in thread number %1% in scheduler with id %2% and name '%3%' finish with exception %4%";
                m_log->Error(boost::str(boost::format(msg) % thread_number % m_id % m_name % e.what()));
            }
            catch (...)
            {
                auto msg = "Task in thread number %1% in scheduler with id %2% and name '%3%' finish with unknown exception";
                m_log->Error(boost::str(boost::format(msg) % thread_number % m_id % m_name));
            }
        }
    }

    boost::thread CreateThread(uint32_t thread_number)
    {
        return boost::thread([this, thread_number] {
            try
            {
                MainLoop(thread_number);
            }
            catch (std::exception& e)
            {
                auto msg = "Thread in scheduler with id %1% and name '%2%' finish with exception %3%";
                m_log->Error(boost::str(boost::format(msg) % m_id % m_name % e.what()));
            }
            catch (...)
            {
                auto msg = "Thread in scheduler with id %1% and name '%2%' finish with unknown exception";
                m_log->Error(boost::str(boost::format(msg) % m_id % m_name));
            }
        });
    }

    uint32_t m_id;
    std::string m_name;
    boost::mutex m_mutex;
    std::shared_ptr<ILog> m_log;
    std::vector<boost::thread> m_threads;
    boost::asio::io_service m_service;
    std::unique_ptr<boost::asio::io_service::work> m_work;
};

coro::CSchedulerDef::CSchedulerDef(std::shared_ptr<ILog> log,
                                   const uint32_t& id,
                                   const std::string& name,
                                   const uint32_t& thread_count)
    : pimpl(std::make_shared<impl>())
{
    pimpl->m_id = id;
    pimpl->m_name = name;
    pimpl->m_log = log;
    pimpl->m_work.reset(new boost::asio::io_service::work(pimpl->m_service));
    pimpl->m_threads.reserve(thread_count);

    for (uint32_t i = 0; i != thread_count; ++i)
        pimpl->m_threads.emplace_back(pimpl->CreateThread(i));

}

coro::CSchedulerDef::~CSchedulerDef()
{
    Stop();
    Join();  
}

bool coro::CSchedulerDef::IsInit(void) const
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    return (pimpl->m_threads.size() != 0);
}

void coro::CSchedulerDef::Add(tTask task)
{
    pimpl->m_service.post(std::move(task));
}

void coro::CSchedulerDef::Stop(void)
{
    if (IsInit())
    {
        auto msg = "Start send stopped signal to scheduler with id %1% and name '%2%'";
        pimpl->m_log->Info(boost::str(boost::format(msg) % pimpl->m_id % pimpl->m_name));
        pimpl->m_log->Info("Start send stopped signal to schedulers");
        pimpl->m_work.reset();
        pimpl->m_service.stop();
        msg = "Finish send stopped signal to scheduler with id %1% and name '%2%'";
        pimpl->m_log->Info(boost::str(boost::format(msg) % pimpl->m_id % pimpl->m_name));
    }
}

void coro::CSchedulerDef::Join(void)
{
    if (IsInit())
    {
        boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

        auto msg = "Start wait finished scheduler with id %1% and name '%2%'";
        pimpl->m_log->Info(boost::str(boost::format(msg) % pimpl->m_id % pimpl->m_name));
        for (auto& thread : pimpl->m_threads)
            thread.join();
        pimpl->m_threads.clear();
        msg = "Finish wait finished scheduler with id %1% and name '%2%'";
        pimpl->m_log->Info(boost::str(boost::format(msg) % pimpl->m_id % pimpl->m_name));
    }
}
