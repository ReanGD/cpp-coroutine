#include "stdafx.h"
#include "ThreadStorage.h"

#include <boost/thread/tss.hpp>


struct SThreadStorage
{
    std::shared_ptr<coro::CContextImpl> context;
    std::shared_ptr<coro::CSchedulerImpl> scheduler;
};

static boost::thread_specific_ptr<SThreadStorage> gt_instance;

namespace
{
    void Init()
    {
        if(gt_instance.get() == nullptr)
            gt_instance.reset(new SThreadStorage());
    }
}

std::shared_ptr<coro::CContextImpl> coro::CThreadStorage::SetContext(std::shared_ptr<CContextImpl> context)
{
    Init();
    auto tmp = gt_instance->context;
    gt_instance->context = context;
    return tmp;
}

std::shared_ptr<coro::CContextImpl> coro::CThreadStorage::GetContext(void)
{
    Init();
    return gt_instance->context;
}


std::shared_ptr<coro::CSchedulerImpl> coro::CThreadStorage::SetScheduler(std::shared_ptr<CSchedulerImpl> scheduler)
{
    Init();
    auto tmp = gt_instance->scheduler;
    gt_instance->scheduler = scheduler;
    return tmp;
}


std::shared_ptr<coro::CSchedulerImpl> coro::CThreadStorage::GetScheduler(void)
{
    Init();
    return gt_instance->scheduler;
}

