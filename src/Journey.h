#pragma once
#ifndef JOURNEY_H
#define JOURNEY_H

#include "Goer.h"
#include "Context.h"
#include "Scheduler.h"

namespace coro
{
    class CJourney
    {
    private:
        struct ContextGuard
        {
            ContextGuard(CJourney& j_)
                : j(j_)
                { j.OnEnter(); }
            ~ContextGuard()
                { j.OnExit(); }
            ContextGuard& operator=(const ContextGuard&) = delete;
            coro::CContext Get()
            {
                return j.m_context;
            }
        private:
            CJourney& j;
        };
    private:
        CJourney(IScheduler::Ptr scheduler);
    public:
        CJourney() = delete;
        ~CJourney() = default;
    public:
        static CGoer Create(tTask task, IScheduler::Ptr scheduler);
    public:
        void Defer(tTask task);
        void HandleEvents();
        void DisableEvents();
        void EnableEvents();

        CGoer GetGoer(void) const { return m_goer; }
    private:
        void Schedule(tTask task);
        CGoer Start(tTask task);
        void OnEnter();
        void OnExit();
    private:
        bool m_events_allowed = true;
        CGoer m_goer;
        IScheduler::Ptr m_scheduler;
        CContext m_context;
        tTask m_defer_task;
    };

    CJourney& GetCurrentJourney();

}

#endif
