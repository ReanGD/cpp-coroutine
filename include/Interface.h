#pragma once
#ifndef INTERFACE_H
#define INTERFACE_H

#include <memory>
#include "Log.h"
#include "Types.h"
#include "Timeout.h"

namespace coro
{
    namespace impl
    {
        void AddScheduler(const uint32_t& id, const std::string& name, const uint32_t thread_count, tTask init_task);
        void Start(tTask task, const uint32_t& sheduler_id, const size_t stack_size);
        void Resume(const tResumeHandle& resume_handle, const uint32_t& sheduler_id);
    }

    void Init(std::shared_ptr<ILog> log);
    void Stop(void);
    tResumeHandle CurrentResumeId();
    uint32_t CurrentSchedulerId();
    void yield();

    template<class IdType> inline void AddScheduler(const IdType& id, const std::string& name, const uint32_t thread_count = 1, tTask init_task = []{})
    {
        uint32_t uid = static_cast<uint32_t>(id);
        impl::AddScheduler(uid, name, thread_count, std::move(init_task));
    }
    template<class IdType> inline void Start(tTask task, const IdType& sheduler_id, const size_t stack_size = STACK_SIZE)
    {
        uint32_t uid = static_cast<uint32_t>(sheduler_id);
        impl::Start(std::move(task), uid, stack_size);
    }
    template<class IdType> inline void Resume(const tResumeHandle& resume_handle, const IdType& sheduler_id)
    {
        uint32_t uid = static_cast<uint32_t>(sheduler_id);
        impl::Resume(resume_handle, uid);
    }
}

#endif
