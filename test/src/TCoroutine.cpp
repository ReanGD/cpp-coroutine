#include "TCoroutine.h"

#include <thread>
#include "Interface.h"
#include "helper/TLogger.h"

void TestCoroutine::SetUp()
{
    m_cv_counter = 0;
}

void TestCoroutine::IncNotify()
{
    ++m_cv_counter;
    m_cv.notify_all();
}

void TestCoroutine::Wait(uint32_t val, const std::chrono::milliseconds& duration)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_cv.wait_for(lck, duration, [&] {return (m_cv_counter == val);});
}

enum E_SHEDULERS
{
    E_SH_1,
    E_SH_2
};

TEST_F(TestCoroutine, Run)
{
    coro::Init(std::make_shared<CLogFake>());
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    bool is_called = false;
    coro::Start([this, &is_called]
                {
                    is_called = true;
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    coro::Stop();
    ASSERT_EQ(true, is_called);
}
