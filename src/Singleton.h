#pragma once
#ifndef SINGLETON_H
#define SINGLETON_H

#include <mutex>
#include <memory>

namespace coro
{
  template<typename T>
  class CSingleton
  {
  public:
    CSingleton(const CSingleton&) = delete;
    CSingleton& operator=(const CSingleton&) = delete;
  public:
    static T& Instance()
    {
      std::call_once (CSingleton::GetOnceFlag(),
     []{ CSingleton::m_instance.reset( new T() ); });
      return *m_instance.get();
    }
  private:
    static std::once_flag& GetOnceFlag()
    {
      static std::once_flag once;
      return once;
    }
  private:
    static std::unique_ptr<T> m_instance;
  };

  template<typename T> std::unique_ptr<T> CSingleton<T>::m_instance = nullptr;
}

#endif
