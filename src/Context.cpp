#include "stdafx.h"
#include "Context.h"

#include <vector>
#include <stdint.h>
#include <atomic>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/thread/tss.hpp>
#include <boost/context/all.hpp>
#include "Log.h"
#include "Manager.h"
#include "ContextManager.h"

namespace coro
{
    struct ContextStorage
    {
        std::shared_ptr<CContext::impl> context_pimpl;
    };

    static boost::thread_specific_ptr<ContextStorage> gt_instance;
}

struct coro::CContext::impl
{
    impl(std::shared_ptr<ILog> log, const uint32_t& id)
        : m_id(id)
        , m_log(log)
    {
    }
    
    impl& operator=(const impl&) = delete;

    ~impl()
    {
        if (IsStarted())
        {
            auto msg = boost::str(boost::format("CContext with id %1% destroying in started state") % m_id);
            m_log->Warning(msg);
        }
    }    

    bool IsStarted(void) const
    {
        return (m_started || m_running);
    }

    void Start(std::shared_ptr<CContext::impl> this_ptr, const size_t stack_size, tTask task)
    {
        if (IsStarted())
        {
            auto msg = boost::str(boost::format("coro: Coroutine  with id %1% already started") % m_id);
            throw std::runtime_error(msg);
        }

        m_stack.resize(stack_size);
        const auto stack_ptr = &m_stack[m_stack.size() - 1];
        m_context = boost::context::make_fcontext(stack_ptr, stack_size, &CContext::impl::StartWrapper);
        
        Jump(this_ptr, reinterpret_cast<intptr_t>(&task));
    }

    bool Resume(std::shared_ptr<CContext::impl> this_ptr)
    {
        if (!m_started)
        {
            auto msg = boost::str(boost::format("coro: Coroutine with id %1% cannot resume (not started)") % m_id);
            throw std::runtime_error(msg);
        }
        if (m_running)
            return false;
        Jump(this_ptr);

        return true;
    }

    void Jump(std::shared_ptr<CContext::impl> this_ptr, intptr_t fn = 0)
    {
        auto tmp = gt_instance.release();
        gt_instance.reset(new ContextStorage{ this_ptr });
        m_running = true;

        boost::context::jump_fcontext(&m_saved_context, m_context, fn);

        m_running = false;
        gt_instance.reset(tmp);

        if (m_exception != std::exception_ptr())
            std::rethrow_exception(m_exception);
    }

    void YieldImpl(void)
    {
        m_log->ExitCoroutine();
        boost::context::jump_fcontext(&m_context, m_saved_context, 0);
        m_log->EnterCoroutine(m_id);
    }

    void StartImpl(intptr_t fn)
    {
        m_started = true;
        try
        {
            m_log->EnterCoroutine(m_id);
            m_exception = nullptr;
            tTask task = std::move(*reinterpret_cast<tTask*>(fn));
            task();
        }
        catch (...)
        {
            m_exception = std::current_exception();
        }
        m_started = false;
        YieldImpl();
    }

    static void StartWrapper(intptr_t fn)
    {
        auto id = gt_instance->context_pimpl->m_id;
        gt_instance->context_pimpl->StartImpl(fn);
        Get::Instance().ContextManager()->Remove(id);
    }

    uint32_t m_id = 0;
    std::shared_ptr<ILog> m_log;    
    std::atomic<bool> m_started = false;
    std::atomic<bool> m_running = false;
    std::exception_ptr m_exception = nullptr;
    std::vector<uint8_t> m_stack;
    boost::context::fcontext_t m_context = nullptr;
    boost::context::fcontext_t m_saved_context = nullptr;
};

void coro::YieldImpl(void)
{
    if (IsInsideCoro())
        gt_instance->context_pimpl->YieldImpl();
    else
        throw std::runtime_error("coro: Yield not in coro-mode");
}

bool coro::IsInsideCoro(void)
{
    return (gt_instance.get() != nullptr);
}

uint32_t coro::GetContextId(void)
{
    if(IsInsideCoro())
        return gt_instance->context_pimpl->m_id;
    throw std::runtime_error("coro: Get context id not in coro-mode");
}

coro::CContext::CContext(std::shared_ptr<ILog> log, const uint32_t& id)
    : pimpl(std::make_shared<impl>(log, id))
{
}

void coro::CContext::Start(const size_t stack_size, tTask task)
{
    pimpl->Start(pimpl, stack_size, std::move(task));
}

bool coro::CContext::Resume(void)
{
    return pimpl->Resume(pimpl);
}
