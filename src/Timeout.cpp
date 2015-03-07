#include "stdafx.h"
#include "Timeout.h"


struct coro::CTimeout::impl
{
    boost::asio::deadline_timer timer;
};

coro::CTimeout::CTimeout(std::chrono::milliseconds ms)
{
}


coro::CTimeout::~CTimeout()
{
}
