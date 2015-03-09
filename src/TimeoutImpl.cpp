#include "stdafx.h"
#include "TimeoutImpl.h"
#include <boost/format.hpp>


coro::CTimeoutImpl::CTimeoutImpl(const std::chrono::milliseconds& duration)
    : m_duration(duration)
    , m_expiry_time(std::chrono::system_clock::now() + duration)
{
}

bool coro::CTimeoutImpl::IsExpired() const
{
    return (std::chrono::system_clock::now() > m_expiry_time);
}

void coro::CTimeoutImpl::ThrowIfInit() const
{
    if(m_duration != std::chrono::milliseconds::max())
    {
        auto msg = "coro: Operation timeout %1% ms";
        throw TimeoutError(boost::str(boost::format(msg) % m_duration.count()));
    }
}

