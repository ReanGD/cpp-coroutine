#include "stdafx.h"
#include "ContextManager.h"

#include <map>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include "Log.h"
#include "Context.h"


struct coro::CContextManager::impl
{
    uint32_t NextId()
    {
        boost::lock_guard<boost::mutex> guard(m_mutex);
        
        return m_counter++;
    }
    
    std::shared_ptr<CContext> Create(const uint32_t id)
    {
        boost::lock_guard<boost::mutex> guard(m_mutex);

        auto ctx = std::make_shared<CContext>(m_log, id);
        m_pool[id] = ctx;

        return ctx;
    }

    std::shared_ptr<CContext> Get(const uint32_t& id)
    {
        boost::lock_guard<boost::mutex> guard(m_mutex);
        
        auto it = m_pool.find(id);
        if (it == m_pool.cend())
        {
            auto msg = boost::str(boost::format("coro: Not fount context by id (%1%)") % id);
            throw std::runtime_error(msg);
        }
        return it->second;
    }

    void Remove(const uint32_t& id)
    {
        boost::lock_guard<boost::mutex> guard(m_mutex);

        auto it = m_pool.find(id);
        if (it == m_pool.cend())
        {
            auto msg = boost::str(boost::format("coro: Not fount context by id (%1%) for remove") % id);
            m_log->Warning(msg);
        }
        else
            m_pool.erase(it);
    }

    boost::mutex m_mutex;
    uint32_t m_counter = 1;
    std::shared_ptr<ILog> m_log;
    std::map<uint32_t, std::shared_ptr<CContext>> m_pool;
};

coro::CContextManager::CContextManager(std::shared_ptr<ILog> log)
    : pimpl(std::make_shared<impl>())
{
    pimpl->m_log = log;
}

coro::CContextManager::~CContextManager()
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    pimpl->m_pool.clear();
}

void coro::CContextManager::Start(tTask task, const size_t stack_size)
{
    auto id = pimpl->NextId();
    try
    {
        auto coroutine = pimpl->Create(id);
        if(coroutine->Start(std::move(task), stack_size))
            pimpl->Remove(id);
    }
    catch(...)
    {
        pimpl->Remove(id);
        throw;
    }
}

void coro::CContextManager::Resume(const uint32_t& id)
{
    try
    {
        if(pimpl->Get(id)->Resume())
            pimpl->Remove(id);
    }
    catch(...)
    {
        pimpl->Remove(id);
        throw;
    }
}
