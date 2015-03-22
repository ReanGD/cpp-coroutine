#include "stdafx.h"
#include "Scheduler.h"
#include "SchedulerImpl.h"
#include "ThreadStorage.h"

coro::CScheduler::CScheduler(std::shared_ptr<ILog> log, const uint32_t& id, const std::string& name)
    : pimpl(new CSchedulerImpl(log, id, name))
{
}

void coro::CScheduler::Start(const uint32_t& thread_count, tTask init_task)
{
    pimpl->Start(thread_count, std::move(init_task));
}

void coro::CScheduler::Add(tTask task)
{
    pimpl->Add(std::move(task));
}

void coro::CScheduler::AddTimeout(tTask task, const std::chrono::milliseconds& duration)
{
    pimpl->AddTimeout(std::move(task), duration);
}

void coro::CScheduler::Stop()
{
    pimpl->Stop();
}

void coro::CScheduler::Join()
{
    pimpl->Join();
}

uint32_t coro::CScheduler::CurrentId()
{
    if(!IsInsideScheduler())
        throw std::runtime_error("coro: Get scheduler id in not under scheduler thread");
    return CThreadStorage::GetScheduler()->id;
}

bool coro::CScheduler::IsInsideScheduler()
{
    return static_cast<bool>(CThreadStorage::GetScheduler());
}
