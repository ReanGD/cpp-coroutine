#include "stdafx.h"
#include "Manager.h"

#include <stdexcept>
#include "SchedulerManager.h"
#include "ContextManager.h"


struct coro::CManager::impl
{
    bool m_is_init = false;
    std::shared_ptr<ILog> m_log;
    std::shared_ptr<coro::CSchedulerManager> m_scheduler_manager;
    std::shared_ptr<coro::CContextManager> m_context_manager;
};

coro::CManager::CManager()
    : pimpl(std::make_shared<impl>())
{
}

void coro::CManager::Init(std::shared_ptr<ILog> log)
{
    pimpl->m_is_init = true;
    pimpl->m_log = log;
    pimpl->m_scheduler_manager = std::make_shared<CSchedulerManager>(log);
    pimpl->m_context_manager = std::make_shared<CContextManager>(log);
    pimpl->m_scheduler_manager->Create(TIMEOUT_SCHEDULER_ID, "System timeout scheduler", 1, []{});
}

std::shared_ptr<coro::CSchedulerManager> coro::CManager::SchedulerManager()
{
    if (!pimpl->m_is_init)
        throw std::runtime_error("Manager is not init");
    return pimpl->m_scheduler_manager;
}

std::shared_ptr<coro::CContextManager> coro::CManager::ContextManager()
{
    if (!pimpl->m_is_init)
        throw std::runtime_error("Manager is not init");
    return pimpl->m_context_manager;
}
