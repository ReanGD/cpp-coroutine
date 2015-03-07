#include "stdafx.h"
#include "ContextImpl.h"

#include "Log.h"
#include "ThreadStorage.h"
#include <boost/format.hpp>


coro::CContextImpl::CContextImpl(std::shared_ptr<ILog> log, const uint32_t& context_id)
    : id(context_id)
    , m_log(log)
    , m_started(false)
    , m_running(false)
{
}

coro::CContextImpl::~CContextImpl()
{
    if (m_started || m_running)
    {
        auto msg = "Context with id %1% destroying in started state";
        m_log->Warning(boost::str(boost::format(msg) % id));
    }
}    

void coro::CContextImpl::EntryPoint(intptr_t fn)
{
    m_started = true;
    try
    {
        tTask task = std::move(*reinterpret_cast<tTask*>(fn));
        m_exception = nullptr;
        OnEnter();
        task();
    }
    catch (...)
    {
        m_exception = std::current_exception();
    }
    m_started = false;
    YieldImpl();
}

void coro::CContextImpl::EntryPointWrapper(intptr_t fn)
{
    if(!CThreadStorage::GetContext())
        throw std::runtime_error("coro: Context varaible not set before start coroutine");
    CThreadStorage::GetContext()->EntryPoint(fn);
}

void coro::CContextImpl::OnEnter()
{
    m_log->EnterCoroutine(id);
    m_running = true;
}

void coro::CContextImpl::OnExit()
{
    m_running = false;
    m_log->ExitCoroutine();
}

void coro::CContextImpl::Jump(intptr_t fn/* = 0*/)
{
    auto tmp = CThreadStorage::SetContext(shared_from_this());
    boost::context::jump_fcontext(&m_saved_context, m_context, fn);
    CThreadStorage::SetContext(tmp);

    if (m_exception != std::exception_ptr())
        std::rethrow_exception(m_exception);
}

bool coro::CContextImpl::Start(tTask task, const size_t stack_size)
{
    if (m_started)
    {
        auto msg = "Coroutine with id %1% cannot starting in started state";
        m_log->Error(boost::str(boost::format(msg) % id));
    }
    else
    {
        m_stack.resize(stack_size);
        m_context = boost::context::make_fcontext(m_stack.data() + stack_size,
                                                  stack_size,
                                                  &CContextImpl::EntryPointWrapper);
        Jump(reinterpret_cast<intptr_t>(&task));
    }

    bool is_finish = (!m_started);
    return is_finish;
}

bool coro::CContextImpl::Resume()
{
    if (!m_started)
    {
        auto msg = "coro: Coroutine with id %1% cannot resume (not started)";
        throw std::runtime_error(boost::str(boost::format(msg) % id));
    }
    // if (m_running)
    //     return false;???
    Jump();

    bool is_finish = (!m_started);
    return is_finish;
}

void coro::CContextImpl::YieldImpl()
{
    OnExit();
    boost::context::jump_fcontext(&m_context, m_saved_context, 0);
    OnEnter();
}
