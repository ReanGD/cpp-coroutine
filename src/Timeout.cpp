#include "stdafx.h"
#include "Timeout.h"

#include "Context.h"
#include "ContextImpl.h"
#include "ThreadStorage.h"


coro::CTimeout::CTimeout(const std::chrono::milliseconds& duration)
{
    if(!CContext::IsInsideCoroutine())
        throw std::runtime_error("coro: Cannot create coroutine timer not in coroutine");

    m_id = CThreadStorage::GetContext()->AddTimeout(duration);
}

coro::CTimeout::~CTimeout()
{
    CThreadStorage::GetContext()->CancelTimeout(m_id);
}
