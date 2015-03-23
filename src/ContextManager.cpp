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
    
    std::shared_ptr<CContext> Create(const uint32_t id, tTask finish_task)
    {
        boost::lock_guard<boost::mutex> guard(m_mutex);

        auto ctx = std::make_shared<CContext>(m_log, id);
        m_pool[id] = PoolData{ctx, finish_task};

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
        return it->second.context;
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
        {
            try
            {
                it->second.finish_task();
                m_pool.erase(it);
            }
            catch (...)
            {
                m_pool.erase(it);
                throw;
            }

        }
    }

    struct PoolData
    {
        std::shared_ptr<CContext> context;
        tTask finish_task;
    };

    boost::mutex m_mutex;
    uint32_t m_counter = 1;
    std::shared_ptr<ILog> m_log;
    std::map<uint32_t, PoolData> m_pool;
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

void coro::CContextManager::Run(tTask task, const size_t stack_size, tTask finish_task)
{
    auto id = pimpl->NextId();
    try
    {
        auto coroutine = pimpl->Create(id, std::move(finish_task));
        if(coroutine->Start(std::move(task), stack_size))
            pimpl->Remove(id);
    }
    catch(...)
    {
        pimpl->Remove(id);
        throw;
    }
}

void coro::CContextManager::Resume(const tResumeHandle& resume_handle)
{
    try
    {
        if(pimpl->Get(resume_handle.coroutine_id)->Resume(resume_handle.resume_id))
            pimpl->Remove(resume_handle.coroutine_id);
    }
    catch(...)
    {
        pimpl->Remove(resume_handle.coroutine_id);
        throw;
    }
}

void coro::CContextManager::ResumeTimeout(const uint32_t& context_id)
{
    try
    {
        if(pimpl->Get(context_id)->ResumeTimeout())
            pimpl->Remove(context_id);
    }
    catch(...)
    {
        pimpl->Remove(context_id);
        throw;
    }
}

void coro::CContextManager::ActivateTimeout(const uint32_t& context_id, const uint32_t& timeout_id)
{
    try
    {
        pimpl->Get(context_id)->ActivateTimeout(timeout_id);
    }
    catch(...)
    {
        pimpl->Remove(timeout_id);
        throw;
    }
}
