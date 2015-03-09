#pragma once
#ifndef CORE_H
#define CORE_H

#include <memory>
#include "Log.h"
#include "Types.h"
#include "Timeout.h"

namespace coro
{
    namespace impl
    {
        void AddSheduler(const uint32_t& id, const std::string& name, const uint32_t thread_count);
        void Start(std::function<void(void)> task, const uint32_t& sheduler_id, const size_t stack_size);
        void Resume(const uint32_t& coroutine_id, const uint32_t& sheduler_id);
    }

    void Init(std::shared_ptr<ILog> log);
    void Stop(void);
    uint32_t CurrentCoroutineId();
    uint32_t CurrentSchedulerId();
    void yield();

    template<class IdType> inline void AddSheduler(const IdType& id, const std::string& name, const uint32_t thread_count = 1)
    {
        uint32_t uid = static_cast<uint32_t>(id);
        impl::AddSheduler(uid, name, thread_count);
    }
    template<class IdType> inline void Start(std::function<void(void)> task, const IdType& sheduler_id, const size_t stack_size = STACK_SIZE)
    {
        uint32_t uid = static_cast<uint32_t>(sheduler_id);
        impl::Start(std::move(task), uid, stack_size);
    }
    template<class IdType> inline void Resume(const uint32_t& coroutine_id, const IdType& sheduler_id)
    {
        uint32_t uid = static_cast<uint32_t>(sheduler_id);
        impl::Resume(coroutine_id, uid);
    }
}

#endif
