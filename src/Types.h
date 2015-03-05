#pragma once
#ifndef TYPES_H
#define TYPES_H

#include <functional>

namespace coro
{
    using tTask = std::function < void(void) > ;

    const size_t STACK_SIZE = 256 * 1024; // 256 kB
}

#if defined(_MSC_VER)
#   define TLS                      __declspec(thread)
#else
#   define TLS                      __thread
#endif

#endif
