#pragma once
#ifndef CONTEXT_MANAGER_H
#define CONTEXT_MANAGER_H

#include <memory>
#include <stdint.h>
#include "Types.h"


namespace coro
{
    class ILog;
    class CContextManager
    {
    public:
        CContextManager() = delete;
        CContextManager(std::shared_ptr<ILog> log);
        CContextManager(const CContextManager&) = delete;
        CContextManager& operator=(const CContextManager&) = delete;
        ~CContextManager();
    public:
        void Start(tTask task, const size_t stack_size);
        void Resume(const tResumeHandle& resume_handle);
        void ResumeTimeout(const uint32_t& context_id);
        void ActivateTimeout(const uint32_t& context_id, const uint32_t& timeout_id);
    private:
        struct impl; std::shared_ptr<impl> pimpl;
    };
}

#endif
