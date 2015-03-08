#include "stdafx.h"
#include "SchedulerImpl.h"

#include <stdexcept>
#include <boost/format.hpp>
#include <boost/thread/lock_guard.hpp>
#include "ThreadStorage.h"
#include "Log.h"


coro::CSchedulerImpl::CSchedulerImpl(std::shared_ptr<ILog> log, const uint32_t& scheduler_id, const std::string& scheduler_name)
    : id(scheduler_id)
    , name(scheduler_name)
    , m_log(log)
{
}

coro::CSchedulerImpl::~CSchedulerImpl()
{
    Stop();
    Join();  
}

void coro::CSchedulerImpl::MainLoop(uint32_t thread_number)
{
    auto tmp = CThreadStorage::SetScheduler(shared_from_this());
    while (!m_service.stopped())
    {
        try
        {
            m_service.run_one();
        }
        catch (std::exception& e)
        {
            auto msg = "Task in thread number %1% in scheduler with id %2% and name '%3%' finish with exception %4%";
            m_log->Error(boost::str(boost::format(msg) % thread_number % id % name % e.what()));
        }
        catch (...)
        {
            auto msg = "Task in thread number %1% in scheduler with id %2% and name '%3%' finish with unknown exception";
            m_log->Error(boost::str(boost::format(msg) % thread_number % id % name));
        }
    }
    CThreadStorage::SetScheduler(tmp);
}

boost::thread coro::CSchedulerImpl::CreateThread(uint32_t thread_number)
{
    return boost::thread([this, thread_number] {
            try
            {
                MainLoop(thread_number);
            }
            catch (std::exception& e)
            {
                auto msg = "Thread in scheduler with id %1% and name \"%2%\" finish with exception %3%";
                m_log->Error(boost::str(boost::format(msg) % id % name % e.what()));
            }
            catch (...)
            {
                auto msg = "Thread in scheduler with id %1% and name \"%2%\" finish with unknown exception";
                m_log->Error(boost::str(boost::format(msg) % id % name));
            }
        });
}

void coro::CSchedulerImpl::Start(const uint32_t& thread_count)
{
    boost::lock_guard<boost::mutex> guard(m_mutex);
    
    if (m_threads.size() != 0)
    {
        auto msg = "Scheduler with id %1% and name \"%2%\" is already running";
        m_log->Error(boost::str(boost::format(msg) % id % name));
    }

    m_work.reset(new boost::asio::io_service::work(m_service));
    m_threads.reserve(thread_count);

    for (uint32_t i = 0; i != thread_count; ++i)
        m_threads.emplace_back(CreateThread(i));
}

void coro::CSchedulerImpl::Add(tTask task)
{
    m_service.post(std::move(task));
}

void coro::CSchedulerImpl::Stop(void)
{
    boost::lock_guard<boost::mutex> guard(m_mutex);
    
    if (m_threads.size() != 0)
    {
        auto msg = "Start send stopped signal to scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
        m_log->Info("Start send stopped signal to schedulers");
        m_work.reset();
        m_service.stop();
        msg = "Finish send stopped signal to scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
    }
}

void coro::CSchedulerImpl::Join(void)
{
    boost::lock_guard<boost::mutex> guard(m_mutex);
    
    if (m_threads.size() != 0)
    {
        auto msg = "Start wait finished scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
        for (auto& thread : m_threads)
            thread.join();
        m_threads.clear();
        msg = "Finish wait finished scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
    }
}
