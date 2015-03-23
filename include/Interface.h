#pragma once
#ifndef INTERFACE_H
#define INTERFACE_H

#include <memory>
#include "Log.h"
#include "Types.h"
#include "Timeout.h"

namespace coro
{
    void Init(std::shared_ptr<ILog> log);
    void Stop(void);
    tResumeHandle CurrentResumeId();
    uint32_t CurrentSchedulerId();
    void yield();
    void AddScheduler(const uint32_t& id,
                      const std::string& name,
                      const uint32_t thread_count = 1,
                      tTask init_task = []{});
    void Start(tTask task,
               const uint32_t& sheduler_id,
               const size_t stack_size = STACK_SIZE);
    void Resume(const tResumeHandle& resume_handle, const uint32_t& sheduler_id);
}

#endif
