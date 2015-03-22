#pragma once
#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

#include "Log.h"

class CLogFake : public coro::ILog
{
public:
    void EnterCoroutine(const uint32_t&) override;
    void ExitCoroutine(void) override;
    void Debug(const std::string&) override;
    void Info(const std::string&) override;
    void Warning(const std::string&) override;
    void Error(const std::string&) override;
};

#endif
