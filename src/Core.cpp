#include "stdafx.h"
#include "Core.h"

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
    Mng().ShedulerManager()->StopAll();
}

uint32_t coro::Id()
{
    return GetContextId();
}

void coro::yield()
{
    YieldImpl();
}

void coro::impl::AddSheduler(const uint32_t& id, const std::string& name, const uint32_t thread_count)
{
    Mng().ShedulerManager()->Add(id, name, thread_count);
}

void coro::impl::Start(std::function<void(void)> task, const uint32_t& sheduler_id, const size_t stack_size)
{   
    Mng().ShedulerManager()->Get(sheduler_id)->Add([&task, stack_size] { Mng().ContextManager()->Start(std::move(task), stack_size); });
}

void coro::impl::Resume(const uint32_t& coroutine_id, const uint32_t& sheduler_id)
{
    Mng().ShedulerManager()->Get(sheduler_id)->Add([coroutine_id] { Mng().ContextManager()->Resume(coroutine_id); });
}
