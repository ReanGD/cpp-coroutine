#include "stdafx.h"
#include "Interface.h"

#include <thread>
#include "Manager.h"
#include "ContextManager.h"
#include "SchedulerManager.h"
#include "Context.h"

namespace
{
    coro::CManager& Mng()
    {
        return coro::Get::Instance();
    }
}

void coro::Init(std::shared_ptr<ILog> log)
{
    Mng().Init(log);
}

void coro::Stop(void)
{
    Mng().ShedulerManager()->Stop();
}

coro::tResumeHandle coro::CurrentResumeId()
{
    return CContext::CurrentResumeId();
}

uint32_t coro::CurrentSchedulerId()
{
    return CSchedulerManager::CurrentId();
}

void coro::yield()
{
    CContext::YieldImpl();
}

void coro::impl::AddScheduler(const uint32_t& id, const std::string& name, const uint32_t thread_count, tTask init_task)
{
    Mng().ShedulerManager()->Create(id, name, thread_count, std::move(init_task));
}

void coro::impl::Start(tTask task,
                       const uint32_t& sheduler_id,
                       const size_t stack_size)
{
    Mng().ShedulerManager()->Add(sheduler_id,
        [task, stack_size]
        {
            coro::Get::Instance().ContextManager()->Start(std::move(task), stack_size, []{});
        });
}

void coro::impl::Resume(const tResumeHandle& resume_handle, const uint32_t& sheduler_id)
{
    Mng().ShedulerManager()->Add(sheduler_id,
        [resume_handle]
        {
            coro::Get::Instance().ContextManager()->Resume(resume_handle);
        });
}
