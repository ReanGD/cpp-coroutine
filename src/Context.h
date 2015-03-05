#pragma once
#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>
#include "Types.h"

namespace coro
{
  void YieldImpl(void);
  bool IsInsideCoro(void);
  uint32_t GetContextId(void);

  class ILog;
  class CContext
  {
  public:
      struct impl;
  public:
      CContext(std::shared_ptr<ILog> log, const uint32_t& id);
  public:
    void Start(const size_t stack_size, tTask task);
    void Resume(void);
  private:
      std::shared_ptr<impl> pimpl;
  };

}

#endif
