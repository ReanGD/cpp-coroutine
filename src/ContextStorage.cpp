#include "stdafx.h"
#include "ContextStorage.h"

#include <map>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

struct coro::CContextStorage::impl
{
    uint32_t Next(void)
    {
        return m_counter++;
    }

    boost::mutex m_mutex;
    uint32_t m_counter = 1;
    std::map<uint32_t, std::weak_ptr<CContext::impl>> m_ctx_map;
};

coro::CContextStorage::CContextStorage()
    : pimpl(std::make_shared<impl>())
{
}

coro::CContextStorage::~CContextStorage()
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    pimpl->m_ctx_map.clear();
}

uint32_t coro::CContextStorage::Add(std::weak_ptr<CContext::impl> ctx)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    uint32_t id = pimpl->Next();
    pimpl->m_ctx_map[id] = ctx;

    return id;
}

void coro::CContextStorage::Resume(uint32_t id)
{
    std::shared_ptr<CContext::impl> ctx;
    {
        boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

        auto it = pimpl->m_ctx_map.find(id);
        if (it == pimpl->m_ctx_map.cend())
            throw std::runtime_error("coro: Resume coroutine by unknown id");
        if (!it->second.expired())
            throw std::runtime_error("coro: Resume removed coroutine");

        ctx = it->second.lock();
        if (!ctx)
            throw std::runtime_error("coro: Resume removed coroutine");
    }

    CContext(ctx).Resume();
}

void coro::CContextStorage::Remove(uint32_t id)
{
    boost::lock_guard<boost::mutex> guard(pimpl->m_mutex);

    pimpl->m_ctx_map.erase(id);
}
