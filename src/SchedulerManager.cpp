#include "stdafx.h"
#include "SchedulerManager.h"

#include <map>
#include <boost/chrono.hpp>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "Log.h"
#include "Scheduler.h"
#include "ThreadStorage.h"

struct coro::CSchedulerManager::impl
{
    std::shared_ptr<CScheduler> Get(const uint32_t& id)
    {
        auto it = m_pool.find(id);
        if(it == m_pool.cend())
        {
            auto msg = boost::str(boost::format("coro: Scheduler with id %1% not found") % id);
            throw std::runtime_error(msg);
        }

        return it->second;
    }
    
    boost::mutex m_mutex;
    bool m_stopped = false;
    std::map<uint32_t, std::shared_ptr<CScheduler>> m_pool;
};

coro::CSchedulerManager::CSchedulerManager(std::shared_ptr<ILog> log)
    : CBase(log)
    , pimpl(new impl())
{
}

coro::CSchedulerManager::~CSchedulerManager()
{
    Stop(std::chrono::milliseconds(1));
}

void coro::CSchedulerManager::Create(const uint32_t& id, const std::string& name, const uint32_t thread_count, tTask init_task)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    if(pimpl->m_stopped)
        return;

    if((id == ERROR_SCHEDULER_ID) || (pimpl->m_pool.find(id) != pimpl->m_pool.cend()))
    {
        auto msg = boost::str(boost::format("coro: Scheduler with id %1% already exists") % id);
        throw std::runtime_error(msg);
    }

    auto ptr = std::make_shared<CScheduler>(m_log, id, name);
    ptr->Start(thread_count, std::move(init_task));
    pimpl->m_pool[id] = ptr;

    auto msg = boost::str(boost::format("Added scheduler with id %1% and name '%2%'") % id % name);
    m_log->Info(msg);
}

void coro::CSchedulerManager::Add(const uint32_t& id, tTask task)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    if(pimpl->m_stopped)
        return;

    pimpl->Get(id)->Add(std::move(task));
}

void coro::CSchedulerManager::AddTimeout(tTask task, const std::chrono::milliseconds& duration)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    if(pimpl->m_stopped)
        return;

    pimpl->Get(TIMEOUT_SCHEDULER_ID)->AddTimeout(std::move(task), duration);
}

void coro::CSchedulerManager::Stop(const std::chrono::milliseconds& max_duration)
{
    {
        boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);
        if(pimpl->m_stopped)
            return;
        pimpl->m_stopped = true;
    }
    
    m_log->Info("Start send stopped signal to all schedulers");
    for (auto& p : pimpl->m_pool)
        p.second->Stop();
    m_log->Info("Finish send stopped signal to all schedulers");

    m_log->Info("Start wait finished all schedulers");
    auto until_time = boost::chrono::steady_clock::now() + boost::chrono::milliseconds(max_duration.count());
    for (auto& p : pimpl->m_pool)
        p.second->JoinUntil(until_time);

    pimpl->m_pool.clear();
    m_log->Info("Finish wait finished all schedulers");
}

uint32_t coro::CSchedulerManager::CurrentId()
{
    auto scheduler_id = CThreadStorage::GetSchedulerId();
    if(scheduler_id == ERROR_SCHEDULER_ID)
        throw std::runtime_error("coro: Get scheduler id in not under scheduler thread");
    return scheduler_id;
}
