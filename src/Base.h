#pragma once
#ifndef BASE_H
#define BASE_H

#include <memory>

namespace coro
{
    class ILog;
    class CBase
    {
    public:
        CBase() = delete;
        CBase(std::shared_ptr<ILog> log);
        virtual ~CBase();
    protected:
        std::shared_ptr<ILog> m_log;
    };
}

#endif
