#pragma once
#ifndef MAIN_H
#define MAIN_H

#include "Log.h"

class CLogImpl : public coro::ILog
{
public:
    void Debug(const std::string& msg) override;
    void Info(const std::string& msg) override;
    void Warning(const std::string& msg) override;
    void Error(const std::string& msg) override;
};

#endif
