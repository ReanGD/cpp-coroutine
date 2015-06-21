#include "stdafx.h"
#include "TimeoutState.h"

#include <boost/format.hpp>
#include "Types.h"


void coro::CTimeoutState::RecalcLock()
{
    for (const auto& it : m_storage)
        if (it.second.state == State::TRIGGERED)
        {
            m_lock = true;
            return;
        }
    m_lock = false;
}

uint32_t coro::CTimeoutState::Add(const uint32_t& scheduler_id, const std::chrono::milliseconds& duration)
{
    auto id = m_last_id++;
    
    if (m_storage.size() > 10) // clear old elements
    {
        for (auto it = m_storage.begin(); it != m_storage.end();)
            if (it->second.state == State::OFF)
                m_storage.erase(it++);
            else
                ++it;
    }
    m_storage[id] = SData(scheduler_id, duration);

    return id;
}

bool coro::CTimeoutState::Activate(const uint32_t& id, uint32_t& scheduler_id)
{
    auto it = m_storage.find(id);
    if ((it != m_storage.cend()) && (it->second.state == State::WAIT))
    {
        it->second.state = State::TRIGGERED;
        scheduler_id = it->second.scheduler_id;
        m_lock = true;
        return true;
    }

    return false;
}

void coro::CTimeoutState::Cancel(const uint32_t& id)
{
    auto it = m_storage.find(id);
    if(it != m_storage.cend())
    {
        it->second.state = State::OFF;
        RecalcLock();
    }
}

void coro::CTimeoutState::CallThrow(const uint32_t& scheduler_id)
{
    if(!m_lock)
        return;

    m_lock = false;
    std::chrono::milliseconds duration;
    for (auto& it : m_storage)
    {
        if (it.second.state == State::TRIGGERED)
        {
            it.second.state = State::OFF;
            if (it.second.scheduler_id == scheduler_id)
                duration = it.second.duration;
        }
    }
    auto msg = "coro: Operation timeout %1% ms";
    throw TimeoutError(boost::str(boost::format(msg) % duration.count()));
}

bool coro::CTimeoutState::IsLock() const
{
    return m_lock;
}

bool coro::CTimeoutState::CheckScheduler(const uint32_t& scheduler_id, uint32_t& next_scheduler_id) const
{
    for (const auto& it : m_storage)
        if (it.second.state == State::TRIGGERED)
        {
            next_scheduler_id = it.second.scheduler_id;
            if (it.second.scheduler_id == scheduler_id)
                return true;
        }

    return false;
}
