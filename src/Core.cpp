#include "stdafx.h"
#include "Core.h"

#include <thread>
#include "Manager.h"
#include "ContextManager.h"
#include "SchedulerManager.h"
#include "Scheduler.h"
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

uint32_t coro::CurrentCoroutineId()
{
    return CContext::CurrentId();
}

uint32_t coro::CurrentSchedulerId()
{
    return CScheduler::CurrentId();
}

void coro::yield()
{
    CContext::YieldImpl();
}

void coro::impl::AddSheduler(const uint32_t& id, const std::string& name, const uint32_t thread_count)
{
    Mng().ShedulerManager()->Create(id, name, thread_count);
}

void coro::impl::Start(std::function<void(void)> task,
                       const uint32_t& sheduler_id,
                       const size_t stack_size/* = STACK_SIZE*/)
{
    Mng().ShedulerManager()->Add(sheduler_id,
        [task, stack_size]
        {
            coro::Get::Instance().ContextManager()->Start(std::move(task), stack_size);
        });
}

void coro::impl::Resume(const uint32_t& coroutine_id, const uint32_t& sheduler_id)
{
    Mng().ShedulerManager()->Add(sheduler_id,
        [coroutine_id]
        {
            coro::Get::Instance().ContextManager()->Resume(coroutine_id);
        });
}
