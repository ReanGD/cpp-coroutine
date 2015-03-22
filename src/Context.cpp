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

bool coro::CContext::Resume(const uint32_t resume_id)
{
    return pimpl->Resume(resume_id);
}

bool coro::CContext::ResumeTimeout()
{
    return pimpl->ResumeTimeout();
}

void coro::CContext::ActivateTimeout(const uint32_t& timeout_id)
{
    pimpl->ActivateTimeout(timeout_id);
}

coro::tResumeHandle coro::CContext::CurrentResumeId()
{
    if(!IsInsideCoroutine())
        throw std::runtime_error("coro: Get context id in not coro-mode");
    tResumeHandle resume_handle;
    auto ctx = CThreadStorage::GetContext();
    resume_handle.coroutine_id = ctx->id;
    resume_handle.resume_id = ctx->GetResumeId();

    return resume_handle;
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
