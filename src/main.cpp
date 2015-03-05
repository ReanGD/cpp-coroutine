#include "stdafx.h"

#include <map>
#include <thread>
#include <iostream>
#include "Core.h"

void sleep(uint32_t sec);
void task(uint32_t coro_num);
void run(void);

class CLogImpl : public coro::ILog
{
public:
    void Debug(const std::string& msg) override
    {
        std::cout << "Debug: " << msg << std::endl;
    }
    void Info(const std::string& msg) override
    {
        std::cout << "Info: " << msg << std::endl;
    }
    void Warning(const std::string& msg) override
    {
        std::cout << "Warning: " << msg << std::endl;
    }
    void Error(const std::string& msg) override
    {
        std::cout << "Error: " << msg << std::endl;
    }
};


void sleep(uint32_t sec)
{
    std::this_thread::sleep_for(std::chrono::seconds(sec));
}

static std::map<uint32_t, uint32_t> g_map;

void task(uint32_t coro_num)
{
    std::cout << "coro start = " << coro_num << std::endl;

    sleep(coro_num);
    //if(coro_num == 3)
    //throw std::runtime_error("error 3");
    try
    {
        //throw std::runtime_error("123");
        g_map[coro_num] = coro::Id();
        coro::yield();
        /*coro::Defer([coro_num]{
                std::cout << "defer " << std::endl;
            });*/
    }
    catch(const std::exception& e)
    {
        std::cout << "task ex = " << e.what() << std::endl;
    }
    std::cout << "coro stop = " << coro_num << std::endl;
}

enum E_SHEDULERS
{
    E_SH_MAIN
};

void run()
{
    coro::Init(std::make_shared<CLogImpl>());
    coro::AddSheduler(E_SH_MAIN, "main", 1);
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
