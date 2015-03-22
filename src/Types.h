#pragma once
#ifndef TYPES_H
#define TYPES_H

#if defined(_MSC_VER)
#   define TLS                      __declspec(thread)
#   define NOEXCEPT
#else
#   define TLS                      __thread
#   define NOEXCEPT                 noexcept
#endif

#include <string>
#include <stdint.h>
#include <stdexcept>
#include <functional>

namespace coro
{
    using tTask = std::function < void(void) >;
    struct tResumeHandle
    {
        uint32_t coroutine_id;
        uint32_t resume_id;
    };

    const uint32_t TIMEOUT_SCHEDULER_ID = 0xffffffff;

    const size_t STACK_SIZE = 256 * 1024; // 256 kB

    class TimeoutError
        : public std::runtime_error
    {
    public:
        explicit TimeoutError(const std::string& what_arg);
        explicit TimeoutError(const char* what_arg);
    public:
        const char* what() const NOEXCEPT override;
    };
}

#endif
