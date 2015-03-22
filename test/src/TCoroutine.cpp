#include "TCoroutine.h"

#include <thread>
#include "Interface.h"
#include "helper/TLogger.h"

void TestCoroutine::SetUp()
{
    m_cv_counter = 0;
    coro::Init(std::make_shared<CLogFake>());
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

TEST_F(TestCoroutine, Yield0)
{
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    uint32_t process(0);
    coro::Start([this, &process]
                {
                    process = 1;
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    ASSERT_EQ(1, process);
    coro::Stop();
}

TEST_F(TestCoroutine, Yield1)
{
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    uint32_t process(0);
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

TEST_F(TestCoroutine, Yield2)
{
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    uint32_t process(0);
    coro::tResumeHandle h_resume;
    coro::Start([this, &process, &h_resume]
                {
                    process = 1;
                    h_resume = coro::CurrentResumeId();
                    IncNotify();
                    coro::yield();
                    
                    process = 2;
                    h_resume = coro::CurrentResumeId();
                    IncNotify();
                    coro::yield();
                    
                    process = 3;
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    ASSERT_EQ(1, process);
    coro::Resume(h_resume, E_SH_1);
    
    Wait(2);
    ASSERT_EQ(2, process);
    coro::Resume(h_resume, E_SH_1);

    Wait(3);
    ASSERT_EQ(3, process);
    coro::Stop();
}

TEST_F(TestCoroutine, YieldInOtherScheduler)
{
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    coro::AddSheduler(E_SH_2, "dop", 1, []{});
    uint32_t scheduler_id(coro::ERROR_SCHEDULER_ID);
    coro::tResumeHandle h_resume;
    coro::Start([this, &scheduler_id, &h_resume]
                {
                    scheduler_id = coro::CurrentSchedulerId();
                    h_resume = coro::CurrentResumeId();
                    IncNotify();
                    coro::yield();

                    scheduler_id = coro::CurrentSchedulerId();
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    ASSERT_EQ(E_SH_1, scheduler_id);
    coro::Resume(h_resume, E_SH_2);

    Wait(2);
    ASSERT_EQ(E_SH_2, scheduler_id);
    coro::Stop();
}

TEST_F(TestCoroutine, TimeoutTrigger)
{
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    bool is_triggered(false);
    uint32_t scheduler_id(coro::ERROR_SCHEDULER_ID);
    coro::Start([this, &is_triggered, &scheduler_id]
                {
                    is_triggered = false;
                    try
                    {
                        coro::CTimeout(std::chrono::milliseconds(100));
                        coro::yield();
                    }
                    catch (const coro::TimeoutError&)
                    {
                        is_triggered = true;
                        scheduler_id = coro::CurrentSchedulerId();
                    }
                    catch (...)
                    {
                    }
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    ASSERT_EQ(true, is_triggered);
    ASSERT_EQ(E_SH_1, scheduler_id);
    coro::Stop();
}

TEST_F(TestCoroutine, TimeoutNotLoad)
{
    coro::AddSheduler(E_SH_1, "main", 1, []{});
    bool is_triggered(false);
    uint32_t scheduler_id(coro::ERROR_SCHEDULER_ID);
    coro::tResumeHandle h_resume;
    coro::Start([this, &is_triggered, &scheduler_id, &h_resume]
                {
                    is_triggered = false;
                    try
                    {
                        coro::CTimeout(std::chrono::seconds(10));
                        h_resume = coro::CurrentResumeId();
                        IncNotify();
                        coro::yield();
                    }
                    catch (const coro::TimeoutError&)
                    {
                        is_triggered = true;
                    }
                    catch (...)
                    {
                    }
                    scheduler_id = coro::CurrentSchedulerId();
                    IncNotify();
                }, E_SH_1);
    Wait(1);
    coro::Resume(h_resume, E_SH_1);
    Wait(2);
    ASSERT_EQ(false, is_triggered);
    ASSERT_EQ(E_SH_1, scheduler_id);
    coro::Stop();
}
