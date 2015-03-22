#include "stdafx.h"
#include "ContextImpl.h"

#include "Log.h"
#include "Manager.h"
#include "SchedulerManager.h"
#include "ContextManager.h"
#include "ThreadStorage.h"
#include <boost/format.hpp>
#include <boost/thread/lock_guard.hpp>


void coro::CContextImpl::SResume::Init(const uint32_t& p_resume_id)
{
    is_init = true;
    resume_id = p_resume_id;
    scheduler_id = CSchedulerManager::CurrentId();
}

bool coro::CContextImpl::SResume::IsInit()
{
    return is_init;
}

void coro::CContextImpl::SResume::Reset()
{
    is_init = false;
}

void coro::CContextImpl::SResume::Call(const uint32_t& context_id)
{
    Reset();
    auto mng = Get::Instance().ShedulerManager();
    tResumeHandle h_resume;
    h_resume.coroutine_id = context_id;
    h_resume.resume_id = resume_id;
    mng->Add(scheduler_id,
             [h_resume]
             {
                 coro::Get::Instance().ContextManager()->Resume(h_resume);
             });
}

void coro::CContextImpl::STimeoutResume::Init()
{
    is_init = true;
}

void coro::CContextImpl::STimeoutResume::Reset()
{
    is_init = false;
}

bool coro::CContextImpl::STimeoutResume::Call(const uint32_t& context_id, const CTimeoutState& timeouts)
{
    if((!is_init) || (!timeouts.IsLock()))
    {
        Reset();
        return false;
    }
    uint32_t scheduler_id = CSchedulerManager::CurrentId();
    uint32_t next_scheduler_id;
    if(!timeouts.CheckScheduler(scheduler_id, next_scheduler_id))
        scheduler_id = next_scheduler_id;

    auto mng = Get::Instance().ShedulerManager();
    mng->Add(scheduler_id,
             [context_id]
             {
                 coro::Get::Instance().ContextManager()->ResumeTimeout(context_id);
             });
    
    return true;
}

coro::CContextImpl::CContextImpl(std::shared_ptr<ILog> log, const uint32_t& context_id)
    : id(context_id)
    , m_log(log)
    , m_started(false)
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
    boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

    m_log->EnterCoroutine(id);
    if(m_timeouts.IsLock())
    {
        ++m_resume_id;
        m_timeouts.CallThrow(CSchedulerManager::CurrentId());
    }
}

void coro::CContextImpl::OnExit()
{
    boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

    m_log->ExitCoroutine();
}

void coro::CContextImpl::Jump(intptr_t fn/* = 0*/)
{
    std::shared_ptr<CContextImpl> tmp;
    {
        boost::lock_guard<boost::recursive_mutex> guard(m_mutex);
        m_running = true;
        tmp = CThreadStorage::SetContext(shared_from_this());
    }
 
    boost::context::jump_fcontext(&m_saved_context, m_context, fn);

    {
        boost::lock_guard<boost::recursive_mutex> guard(m_mutex);
        CThreadStorage::SetContext(tmp);
        m_running = false;
        if (m_exception != std::exception_ptr())
            std::rethrow_exception(m_exception);
        if (m_next_resume.is_init)
        {
            if(m_next_timeout_resume.Call(id, m_timeouts))
            {
                m_next_resume.Reset();
                return;
            }

            if(m_timeouts.IsLock())
            {
                m_next_resume.Reset();
                return;
            }

            if(!m_next_resume.IsInit())
                return;
            
            if(!CheckResume(m_next_resume.resume_id))
            {
                m_next_resume.Reset();
                return;
            }
                
            m_next_resume.Call(id);
        }
    }
}

bool coro::CContextImpl::CheckResume(const uint32_t resume_id)
{
    if(resume_id != m_resume_id)
    {
        auto msg = "Context with id %1% can't resume with old resume_id %2%";
        m_log->Warning(boost::str(boost::format(msg) % id % resume_id));
        return false;
    }

    return true;
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

bool coro::CContextImpl::Resume(const uint32_t resume_id)
{
    if (!m_started)
    {
        auto msg = "coro: Coroutine with id %1% cannot resume (not started)";
        throw std::runtime_error(boost::str(boost::format(msg) % id));
    }

    bool is_finish = false;
    {
        boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

        if(!CheckResume(resume_id))
            return is_finish;
        
        if(m_running)
        {
            m_next_resume.Init(resume_id);
            return is_finish;
        }

        m_running = true;
    }
    
    Jump();
    is_finish = (!m_started);
    return is_finish;
}

void coro::CContextImpl::YieldImpl()
{
    OnExit();
    boost::context::jump_fcontext(&m_context, m_saved_context, 0);
    OnEnter();
}

uint32_t coro::CContextImpl::GetResumeId(void)
{
    boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

    return ++m_resume_id;
}

uint32_t coro::CContextImpl::AddTimeout(const std::chrono::milliseconds& duration)
{
    boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

    return m_timeouts.Add(CSchedulerManager::CurrentId(), duration);
}

void coro::CContextImpl::ActivateTimeout(const uint32_t& timeout_id)
{
    boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

    uint32_t scheduler_id;
    if(m_timeouts.Activate(timeout_id, scheduler_id))
    {
        auto const ctx_id = id;
        auto mng = Get::Instance().ShedulerManager();
        mng->Add(scheduler_id,
                 [ctx_id]
                 {
                     coro::Get::Instance().ContextManager()->ResumeTimeout(ctx_id);
                 });
    }
}

void coro::CContextImpl::CancelTimeout(const uint32_t& timeout_id)
{
    boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

    m_timeouts.Cancel(timeout_id);
}

bool coro::CContextImpl::ResumeTimeout()
{
    bool is_finish = false;
    {
        boost::lock_guard<boost::recursive_mutex> guard(m_mutex);

        if(!m_timeouts.IsLock())
            return is_finish;

        if(m_running)
        {
            m_next_timeout_resume.Init();
            return is_finish;

        }else
        {
            uint32_t next_scheduler_id;
            if(!m_timeouts.CheckScheduler(CSchedulerManager::CurrentId(), next_scheduler_id))
            {
                auto mng = Get::Instance().ShedulerManager();
                auto const ctx_id = id;
                mng->Add(next_scheduler_id,
                         [ctx_id]
                         {
                             coro::Get::Instance().ContextManager()->ResumeTimeout(ctx_id);
                         });

                return is_finish;
            }

            m_running = true;
        }
    }
    Jump();
    is_finish = (!m_started);
    return is_finish;
}
