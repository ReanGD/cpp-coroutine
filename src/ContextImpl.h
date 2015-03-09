#pragma once
#ifndef CONTEXT_IMPL_H
#define CONTEXT_IMPL_H

#include <map>
#include <memory>
#include <atomic>
#include <vector>
#include <exception>
#include <stdint.h>
#include <boost/thread/mutex.hpp>
#include <boost/context/all.hpp>
#include "TimeoutImpl.h"

namespace coro
{
    class ILog;
    class CContextImpl
        : public std::enable_shared_from_this<CContextImpl>
    {
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
        void CheckTimeouts();
        void OnEnter();
        void OnExit();
        void Jump(intptr_t fn = 0);
    public:
        bool Start(tTask task, const size_t stack_size);
        bool Resume();
        void YieldImpl();
        uint32_t AddTimeout(const std::chrono::milliseconds& duration);
        void CancelTimeout(const uint32_t timeout_id);
    public:
        const uint32_t id = 0;
    private:
        std::shared_ptr<ILog> m_log;
        std::atomic<bool> m_started;
        bool m_running = false;
        bool m_planned_resume = false;
        uint32_t m_resume_scheduler = 0;
        boost::mutex m_mutex;
        std::map<uint32_t, CTimeoutImpl> m_timeouts;
        uint32_t m_timeout_id_counter = 0;
        std::exception_ptr m_exception = nullptr;
        std::vector<uint8_t> m_stack;
        boost::context::fcontext_t m_context = nullptr;
        boost::context::fcontext_t m_saved_context = nullptr;
    };
}
#endif
