#include "stdafx.h"
#include "Context.h"
#include "ContextImpl.h"
#include "ThreadStorage.h"

coro::CContext::CContext(std::shared_ptr<ILog> log, const uint32_t& id)
    : pimpl(new CContextImpl(log, id))
{
}

bool coro::CContext::Start(tTask task, const size_t stack_size)
{
    return pimpl->Start(std::move(task), stack_size);
}

bool coro::CContext::Resume(void)
{
    return pimpl->Resume();
}

uint32_t coro::CContext::CurrentId()
{
    if(!IsInsideCoroutine())
        throw std::runtime_error("coro: Get context id in not coro-mode");
    return CThreadStorage::GetContext()->id;
}

void coro::CContext::YieldImpl()
{
    if(!IsInsideCoroutine())
        throw std::runtime_error("coro: Yield in not coro-mode");
    CThreadStorage::GetContext()->YieldImpl();
}

bool coro::CContext::IsInsideCoroutine()
{
    return static_cast<bool>(CThreadStorage::GetContext());
}
