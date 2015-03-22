#include "stdafx.h"
#include "ThreadStorage.h"

#include <boost/thread/tss.hpp>
#include "Types.h"

struct SThreadStorage
{
    std::shared_ptr<coro::CContextImpl> context;
    uint32_t scheduler_id = coro::ERROR_SCHEDULER_ID;
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

uint32_t coro::CThreadStorage::SetSchedulerId(uint32_t scheduler_id)
{
    Init();
    auto tmp = gt_instance->scheduler_id;
    gt_instance->scheduler_id = scheduler_id;
    return tmp;
}

uint32_t coro::CThreadStorage::GetSchedulerId(void)
{
    Init();
    return gt_instance->scheduler_id;
}
