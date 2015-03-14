#include "stdafx.h"
#include "Types.h"


coro::TimeoutError::TimeoutError(const std::string& what_arg)
    : std::runtime_error(what_arg)
{
}

coro::TimeoutError::TimeoutError(const char* what_arg)
    : std::runtime_error(what_arg)
{
}

const char* coro::TimeoutError::what() const NOEXCEPT
{
    return std::runtime_error::what();
}
