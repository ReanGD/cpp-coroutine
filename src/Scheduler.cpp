#include "stdafx.h"
#include "Scheduler.h"

#include <vector>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "Log.h"
#include "ThreadStorage.h"

struct coro::CScheduler::impl
{
    boost::mutex mutex;
    std::vector<boost::thread> threads;
    boost::asio::io_service service;
    std::unique_ptr<boost::asio::io_service::work> work;
};

coro::CScheduler::CScheduler(std::shared_ptr<ILog> log,
                             const uint32_t& scheduler_id, const std::string& scheduler_name)
    : CBase(log)
    , id(scheduler_id)
    , name(scheduler_name)
    , pimpl(new impl())

{
}

coro::CScheduler::~CScheduler()
{
    Stop();
    JoinUntil(boost::chrono::steady_clock::now());
}

void coro::CScheduler::MainLoop(const uint32_t& thread_number, const tTask& init_task)
{
    try
    {
        auto tmp = CThreadStorage::SetSchedulerId(id);
        init_task();
        while (!pimpl->service.stopped())
        {
            try
            {
                pimpl->service.run_one();
            }
            catch (const boost::thread_interrupted&)
            {
                auto msg =
                    "Task in thread number %1% in scheduler with id %2% and name '%3%' finish by boost::thread_interrupted";
                m_log->Info(boost::str(boost::format(msg) % thread_number % id % name));
            }
            catch (std::exception& e)
            {
                auto msg =
                    "Task in thread number %1% in scheduler with id %2% and name '%3%' finish with exception %4%";
                m_log->Error(boost::str(boost::format(msg) % thread_number % id % name % e.what()));
            }
            catch (...)
            {
                auto msg =
                    "Task in thread number %1% in scheduler with id %2% and name '%3%' finish with unknown exception";
                m_log->Error(boost::str(boost::format(msg) % thread_number % id % name));
            }
        }

        CThreadStorage::SetSchedulerId(tmp);
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
}

void coro::CScheduler::Start(const uint32_t& thread_count, const tTask& init_task)
{
    boost::lock_guard<boost::mutex> guard(pimpl->mutex);
    
    if (pimpl->threads.size() != 0)
    {
        auto msg = "Scheduler with id %1% and name \"%2%\" is already running";
        m_log->Error(boost::str(boost::format(msg) % id % name));
    }

    pimpl->work.reset(new boost::asio::io_service::work(pimpl->service));
    pimpl->threads.reserve(thread_count);

    for (uint32_t i = 0; i != thread_count; ++i)
        pimpl->threads.emplace_back([this, i, init_task]
                                    {
                                        MainLoop(i, init_task);
                                    });
}

void coro::CScheduler::Add(tTask task)
{
    pimpl->service.post(std::move(task));
}

void coro::CScheduler::AddTimeout(tTask task, const std::chrono::milliseconds& duration)
{
    auto boost_duration = boost::posix_time::milliseconds(duration.count());
    auto timer = std::make_shared<boost::asio::deadline_timer>(pimpl->service, boost_duration);
    timer->async_wait([timer, task] (const boost::system::error_code& e) mutable
                      {
                          if (!e)
                              task();
                          timer.reset();
                      });
}

void coro::CScheduler::Stop()
{
    boost::lock_guard<boost::mutex> guard(pimpl->mutex);
    
    if (pimpl->threads.size() != 0)
    {
        auto msg = "Start send stopped signal to scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
        m_log->Info("Start send stopped signal to schedulers");
        pimpl->work.reset();
        pimpl->service.stop();
        msg = "Finish send stopped signal to scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
    }
}

void coro::CScheduler::JoinUntil(const boost::chrono::steady_clock::time_point& until_time)
{
    boost::lock_guard<boost::mutex> guard(pimpl->mutex);
    
    if (pimpl->threads.size() != 0)
    {
        auto msg = "Start wait finished scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
        for (auto& thread : pimpl->threads)
            if (!thread.try_join_until(until_time))
                thread.interrupt();

        pimpl->threads.clear();
        msg = "Finish wait finished scheduler with id %1% and name \"%2%\"";
        m_log->Info(boost::str(boost::format(msg) % id % name));
    }
}

void coro::CScheduler::InterruptionPoint(void)
{
    boost::this_thread::interruption_point();
}
