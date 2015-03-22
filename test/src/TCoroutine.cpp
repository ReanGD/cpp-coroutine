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

TEST_F(TestCoroutine, RunAndYield)
{
    coro::Init(std::make_shared<CLogFake>());
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    std::atomic<uint32_t> process(0);
    coro::tResumeHandle h_resume;
    coro::Start([this, &process, &h_resume]
                {
                    process = 1;
                    h_resume = coro::CurrentResumeId();
                    IncNotify();
                    coro::yield();
                    process = 2;
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    ASSERT_EQ(1, process);
    coro::Resume(h_resume, E_SH_1);
    Wait(2);
    ASSERT_EQ(2, process);
    coro::Stop();
}
