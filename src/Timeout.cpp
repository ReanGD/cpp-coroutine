#include "stdafx.h"
#include "Timeout.h"

#include "Context.h"
#include "ContextImpl.h"
#include "ThreadStorage.h"
#include "Manager.h"
#include "SchedulerManager.h"
#include "ContextManager.h"

coro::CTimeout::CTimeout(const std::chrono::milliseconds& duration)
{
    if(!CContext::IsInsideCoroutine())
        throw std::runtime_error("coro: Cannot create coroutine timer not in coroutine");

    auto context_id = CThreadStorage::GetContext()->id;
    auto timeout_id = CThreadStorage::GetContext()->AddTimeout(duration);
    m_id = timeout_id;
    auto lambda = [context_id, timeout_id]
        {
            Get::Instance().ContextManager()->ActivateTimeout(context_id, timeout_id);
        };
    Get::Instance().ShedulerManager()->AddTimeout(std::move(lambda), duration);
}

coro::CTimeout::~CTimeout()
{
    CThreadStorage::GetContext()->CancelTimeout(m_id);
}
