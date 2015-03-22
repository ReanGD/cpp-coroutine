#include "stdafx.h"
#include "SchedulerManager.h"

#include <map>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "Log.h"
#include "Scheduler.h"
#include "ThreadStorage.h"

struct coro::CSchedulerManager::impl
{
    boost::mutex m_mutex;
    std::map<uint32_t, std::shared_ptr<CScheduler>> m_pool;
};

coro::CSchedulerManager::CSchedulerManager(std::shared_ptr<ILog> log)
    : CBase(log)
    , pimpl(new impl())
{
}

coro::CSchedulerManager::~CSchedulerManager()
{
    Stop();
}

void coro::CSchedulerManager::Create(const uint32_t& id, const std::string& name, const uint32_t thread_count, tTask init_task)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

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

    auto it = pimpl->m_pool.find(id);
    if(it == pimpl->m_pool.cend())
    {
        auto msg = boost::str(boost::format("coro: Scheduler with id %1% not found") % id);
        throw std::runtime_error(msg);
    }

    it->second->Add(std::move(task));
}

void coro::CSchedulerManager::AddTimeout(tTask task, const std::chrono::milliseconds& duration)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    auto it = pimpl->m_pool.find(TIMEOUT_SCHEDULER_ID);
    if(it == pimpl->m_pool.cend())
    {
        auto msg = boost::str(boost::format("coro: Timeout scheduler with id %1% not found") % TIMEOUT_SCHEDULER_ID);
        throw std::runtime_error(msg);
    }

    it->second->AddTimeout(std::move(task), duration);
}

void coro::CSchedulerManager::Stop()
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);
    
    m_log->Info("Start send stopped signal to all schedulers");
    for (auto& p : pimpl->m_pool)
        p.second->Stop();
    m_log->Info("Finish send stopped signal to all schedulers");
    
    m_log->Info("Start wait finished all schedulers");
    for (auto& p : pimpl->m_pool)
        p.second->Join();
    pimpl->m_pool.clear();
    m_log->Info("Finish wait finished all schedulers");
}

uint32_t coro::CSchedulerManager::CurrentId()
{
    if(CThreadStorage::GetSchedulerId() == ERROR_SCHEDULER_ID)
        throw std::runtime_error("coro: Get scheduler id in not under scheduler thread");
    return CThreadStorage::GetSchedulerId();
}
