#include <map>
#include <thread>
#include <iostream>
#include "Interface.h"
#include "main.h"
#include <boost/lexical_cast.hpp>

void local_sleep(uint32_t sec);
std::string FormatMsg(const std::string& s);
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

void local_sleep(uint32_t sec)
{
    std::this_thread::sleep_for(std::chrono::seconds(sec));
}

static std::map<uint32_t, coro::tResumeHandle> g_map;

enum E_SHEDULERS
{
    E_SH_MAIN,
    E_SH_DOP
};

void task(uint32_t coro_num)
{
    auto scheduler_id = coro::CurrentSchedulerId();
    CLogImpl().Debug("coro start in scheduler " + boost::lexical_cast<std::string>(scheduler_id) +
                     " num=" + boost::lexical_cast<std::string>(coro_num));
    try
    {
        std::chrono::seconds sec(coro_num);
        coro::CTimeout t(sec);
        coro::yield();
    }
    catch(const std::exception& e)
    {
        std::cout << "task ex = " << e.what() << std::endl;
    }
    scheduler_id = coro::CurrentSchedulerId();
    CLogImpl().Debug("coro stop in scheduler " + boost::lexical_cast<std::string>(scheduler_id));
}

void run()
{
    coro::Init(std::make_shared<CLogImpl>());
    coro::AddScheduler(E_SH_MAIN, "main", 2, []{ std::cout << "init thread" << std::endl;});
    coro::Run([]{ task(1); }, E_SH_MAIN);
    coro::Run([]{ task(2); }, E_SH_MAIN);
    local_sleep(4);
    CLogImpl().Debug("before stop all");
    coro::Stop(std::chrono::milliseconds(10));
    CLogImpl().Debug("after stop all");
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
