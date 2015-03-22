#include "stdafx.h"
#include "Base.h"


coro::CBase::CBase(std::shared_ptr<ILog> log)
    : m_log(log)
{
}

coro::CBase::~CBase()
{
}
