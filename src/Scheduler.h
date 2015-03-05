#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Types.h"


namespace coro
{
    class IScheduler
    {
    public:
        virtual ~IScheduler();
        virtual void Add(tTask task) = 0;
        virtual void Stop() = 0;
        virtual void Join() = 0;
    };
}

#endif
