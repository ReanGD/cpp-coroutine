#include "stdafx.h"

#include <map>
#include <thread>
#include <iostream>
#include "Core.h"
#include "main.h"
#include <boost\lexical_cast.hpp>

void sleep(uint32_t sec);
void task(uint32_t coro_num);
void run(void);

static TLS bool is_coro_mode = false;
static TLS uint32_t coro_id = 0;

std::string FormatMsg(const std::string& s)
{
    if (!is_coro_mode)
        return "[base] " + s;
    return "[" + boost::lexical_cast<std::string>(coro_id)+"] " + s;
}

void CLogImpl::Debug(const std::string& msg)
{
   std::cout << "Debug: " << FormatMsg(msg) << std::endl;
}
void CLogImpl::Info(const std::string& msg)
{
    std::cout << "Info: " << FormatMsg(msg) << std::endl;
}
void CLogImpl::Warning(const std::string& msg)
{
    std::cout << "Warning: " << FormatMsg(msg) << std::endl;
}
void CLogImpl::Error(const std::string& msg)
{
    std::cout << "Error: " << FormatMsg(msg) << std::endl;
}

void CLogImpl::EnterCoroutine(const uint32_t& id)
{
    coro_id = id;
    is_coro_mode = true;    
}

void CLogImpl::ExitCoroutine(void)
{
    is_coro_mode = false;
    coro_id = 0;
}

void sleep(uint32_t sec)
{
    std::this_thread::sleep_for(std::chrono::seconds(sec));
}

static std::map<uint32_t, uint32_t> g_map;

enum E_SHEDULERS
{
    E_SH_MAIN
};

void task(uint32_t coro_num)
{
    CLogImpl().Debug("coro start");
    sleep(coro_num);
    //if(coro_num == 3)
    //throw std::runtime_error("error 3");
    try
    {
        //throw std::runtime_error("123");
        g_map[coro_num] = coro::Id();        
        sleep(1);
        coro::yield();
        /*coro::Defer([coro_num]{
          std::cout << "defer " << std::endl;
          });*/
    }
    catch(const std::exception& e)
    {
        std::cout << "task ex = " << e.what() << std::endl;
    }
    CLogImpl().Debug("coro stop");
}

void run()
{
    coro::Init(std::make_shared<CLogImpl>());
    coro::AddSheduler(E_SH_MAIN, "main", 3);
    coro::Start([]{ task(1); }, E_SH_MAIN);
    coro::Start([]{ task(2); }, E_SH_MAIN);

    sleep(6);
    for (auto& v_map : g_map)
    {
        auto ctx_id = v_map.second;
        coro::Resume(ctx_id, E_SH_MAIN);
    }
    sleep(2);
    coro::Stop();
}

int main()
{
    try
    {        
        run();
        std::cout << "finish success" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "run exception" << e.what() << std::endl;
    }

    return 0;
}
