#include "stdafx.h"
#include "SchedulerManager.h"

#include <map>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include "Log.h"
#include "Scheduler.h"


struct coro::CSchedulerManager::impl
{
    boost::mutex m_mutex;
    std::shared_ptr<ILog> m_log;
    std::map<uint32_t, std::shared_ptr<CScheduler>> m_pool;
};

coro::CSchedulerManager::CSchedulerManager(std::shared_ptr<ILog> log)
    : pimpl(std::make_shared<impl>())
{
    pimpl->m_log = log;
}

coro::CSchedulerManager::~CSchedulerManager()
{
    Stop();
}

void coro::CSchedulerManager::Create(const uint32_t& id, const std::string& name, const uint32_t thread_count)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    if(pimpl->m_pool.find(id) != pimpl->m_pool.cend())
    {
        auto msg = boost::str(boost::format("coro: Scheduler with id %1% already exists") % id);
        throw std::runtime_error(msg);
    }

    auto ptr = std::make_shared<CScheduler>(pimpl->m_log, id, name);
    ptr->Start(thread_count);
    pimpl->m_pool[id] = ptr;

    auto msg = boost::str(boost::format("Added scheduler with id %1% and name '%2%'") % id % name);
    pimpl->m_log->Info(msg);
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

void coro::CSchedulerManager::Stop()
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);
    
    pimpl->m_log->Info("Start send stopped signal to all schedulers");
    for (auto& p : pimpl->m_pool)
        p.second->Stop();
    pimpl->m_log->Info("Finish send stopped signal to all schedulers");
    
    pimpl->m_log->Info("Start wait finished all schedulers");
    for (auto& p : pimpl->m_pool)
        p.second->Join();
    pimpl->m_pool.clear();
    pimpl->m_log->Info("Finish wait finished all schedulers");
}
