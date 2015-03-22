#pragma once
#ifndef LOG_H
#define LOG_H

#include <string>
#include <stdint.h>


namespace coro
{
    class ILog
    {
    public:
        virtual ~ILog();
    public:
        virtual void EnterCoroutine(const uint32_t& id) = 0;
        virtual void ExitCoroutine(void) = 0;
    public:
        virtual void Debug(const std::string& msg) = 0;
        virtual void Info(const std::string& msg) = 0;
        virtual void Warning(const std::string& msg) = 0;
        virtual void Error(const std::string& msg) = 0;
    };
}

#endif
