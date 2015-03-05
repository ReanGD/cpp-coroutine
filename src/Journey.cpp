#include "stdafx.h"
#include "Journey.h"

#include <iostream>
#include <stdexcept>

namespace coro
{
    static TLS CJourney* t_journey = nullptr;

    CJourney& GetCurrentJourney()
    {
        if (t_journey == nullptr)
            throw std::runtime_error("There is no current journey executed");
        return *t_journey;
    }

}

coro::CJourney::CJourney(IScheduler::Ptr scheduler)
    : m_scheduler(scheduler)
{
}

coro::CGoer coro::CJourney::Create(tTask task, IScheduler::Ptr scheduler)
{
    return (new CJourney(scheduler))->Start(std::move(task));
}

void coro::CJourney::Defer(tTask task)
{
    HandleEvents();
    m_defer_task = task;
    coro::yield();
    HandleEvents();
}

void coro::CJourney::HandleEvents()
{
    if (!m_events_allowed || std::uncaught_exception())
        return;
    auto s = m_goer.Reset();
    if (s == EventStatus::ES_NORMAL)
        return;
    throw EventException(s);
}

void coro::CJourney::Schedule(tTask task)
{
    if(!m_scheduler)
        throw std::runtime_error("Scheduler must be set in journey");

    m_scheduler->Add(std::move(task));
}

coro::CGoer coro::CJourney::Start(tTask task)
{
    CGoer goer = GetGoer();
    Schedule([task, this] {
       ContextGuard(*this).Get().Start([task] {
            try
            {
                task();
            }
            catch (std::exception& e)
            {
                std::cout << "ContextGuard exception" << e.what() << std::endl;
                throw;
            }
        });
    });
    return goer;
}

void coro::CJourney::OnEnter()
{
    t_journey = this;
}

void coro::CJourney::OnExit()
{
    if (m_defer_task == nullptr)
    {
        delete this;
    }
    else
    {
        tTask task = std::move(m_defer_task);
        m_defer_task = nullptr;
        task();
    }
    t_journey = nullptr;
}
