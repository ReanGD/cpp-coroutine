#pragma once
#ifndef CONTEXT_IMPL_H
#define CONTEXT_IMPL_H

#include <memory>
#include <atomic>
#include <vector>
#include <exception>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/context/all.hpp>
#include "TimeoutState.h"
#include "Types.h"


namespace coro
{
    class ILog;
    class CContextImpl
        : public std::enable_shared_from_this<CContextImpl>
    {
    private:
        struct SResume
        {
            void Init(const uint32_t& p_resume_id);
            bool IsInit();
            void Reset();
            void Call(const uint32_t& context_id);
            
            uint32_t resume_id;
            uint32_t scheduler_id;
            bool is_init = false;
        };
        struct STimeoutResume
        {
            void Init();
            void Reset();
            bool Call(const uint32_t& context_id, const CTimeoutState& timeouts);
            
            bool is_init = false;
        };
        
    public:
        CContextImpl(std::shared_ptr<ILog> log, const uint32_t& context_id);
        ~CContextImpl();
        CContextImpl() = delete;
        CContextImpl(const CContextImpl&) = delete;
        CContextImpl& operator=(const CContextImpl&) = delete;
    private:
        void EntryPoint(intptr_t fn);
        static void EntryPointWrapper(intptr_t fn);
    private:
        void OnEnter();
        void OnExit();
        void Jump(intptr_t fn = 0);
        bool CheckResume(const uint32_t resume_id);
    public:
        bool Start(tTask task, const size_t stack_size);
        bool Resume(const uint32_t resume_id);
        void YieldImpl();
        uint32_t GetResumeId(void);
    public:
        uint32_t AddTimeout(const std::chrono::milliseconds& duration);
        void ActivateTimeout(const uint32_t& timeout_id);
        void CancelTimeout(const uint32_t& timeout_id);
        bool ResumeTimeout();
    public:
        const uint32_t id = 0;
    private:
        std::shared_ptr<ILog> m_log;
        std::atomic<bool> m_started;
        bool m_running = false;
        SResume m_next_resume;
        STimeoutResume m_next_timeout_resume;
        uint32_t m_resume_id = 0;
        CTimeoutState m_timeouts;
        boost::recursive_mutex m_mutex;
        std::exception_ptr m_exception = nullptr;
        std::vector<uint8_t> m_stack;
        boost::context::fcontext_t m_context = nullptr;
        boost::context::fcontext_t m_saved_context = nullptr;
    };
}
#endif
