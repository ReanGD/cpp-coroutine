#include "TCoroutine.h"

#include <thread>
#include "Interface.h"
#include "helper/TLogger.h"

void TestCoroutine::SetUp()
{
    m_cv_counter = 0;
    m_wait_counter = 0;
    coro::Init(std::make_shared<CLogFake>());
}

void TestCoroutine::TearDown()
{
    coro::Stop();
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

void TestCoroutine::IncWait(const std::chrono::milliseconds& duration)
{
    ++m_wait_counter;
    Wait(m_wait_counter, duration);
}

enum E_SHEDULERS
{
    E_SH_1,
    E_SH_2
};

TEST_F(TestCoroutine, DisallowToUseServiceSchedulerId)
{
    ASSERT_THROW(coro::AddScheduler(coro::ERROR_SCHEDULER_ID, "main"), std::runtime_error);
    ASSERT_THROW(coro::AddScheduler(coro::TIMEOUT_SCHEDULER_ID, "main"), std::runtime_error);
}

TEST_F(TestCoroutine, DisallowToUseDuplicateSchedulerId)
{
    ASSERT_NO_THROW(coro::AddScheduler(E_SH_1, "main"));
    ASSERT_THROW(coro::AddScheduler(E_SH_1, "dop"), std::runtime_error);
}

TEST_F(TestCoroutine, Yield0)
{
    coro::AddScheduler(E_SH_1, "main");
    uint32_t process(0);
    coro::Run([this, &process]
              {
                  process = 1;
                  IncNotify();
              }, E_SH_1);
    IncWait();
    ASSERT_EQ(1, process);
}

TEST_F(TestCoroutine, Yield1)
{
    coro::AddScheduler(E_SH_1, "main");
    uint32_t process(0);
    coro::tResumeHandle h_resume;
    coro::Run([this, &process, &h_resume]
              {
                  process = 1;
                  h_resume = coro::CurrentResumeId();
                  IncNotify();
                  coro::yield();

                  process = 2;
                  IncNotify();
              }, E_SH_1);
    IncWait();
    ASSERT_EQ(1, process);
    coro::Resume(h_resume, E_SH_1);

    IncWait();
    ASSERT_EQ(2, process);
}

TEST_F(TestCoroutine, Yield2)
{
    coro::AddScheduler(E_SH_1, "main");
    uint32_t process(0);
    coro::tResumeHandle h_resume;
    coro::Run([this, &process, &h_resume]
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
    IncWait();
    ASSERT_EQ(1, process);
    coro::Resume(h_resume, E_SH_1);
    
    IncWait();
    ASSERT_EQ(2, process);
    coro::Resume(h_resume, E_SH_1);

    IncWait();
    ASSERT_EQ(3, process);
}

TEST_F(TestCoroutine, YieldInOtherScheduler)
{
    coro::AddScheduler(E_SH_1, "main");
    coro::AddScheduler(E_SH_2, "dop");
    uint32_t scheduler_id(coro::ERROR_SCHEDULER_ID);
    coro::tResumeHandle h_resume;
    coro::Run([this, &scheduler_id, &h_resume]
              {
                  scheduler_id = coro::CurrentSchedulerId();
                  h_resume = coro::CurrentResumeId();
                  IncNotify();
                  coro::yield();

                  scheduler_id = coro::CurrentSchedulerId();
                  IncNotify();
              }, E_SH_1);
    IncWait();
    ASSERT_EQ(E_SH_1, scheduler_id);
    coro::Resume(h_resume, E_SH_2);

    IncWait();
    ASSERT_EQ(E_SH_2, scheduler_id);
}

TEST_F(TestCoroutine, TimeoutTriggered)
{
    coro::AddScheduler(E_SH_1, "main");
    bool is_triggered(false);
    uint32_t scheduler_id(coro::ERROR_SCHEDULER_ID);
    coro::Run([this, &is_triggered, &scheduler_id]
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
    IncWait();
    ASSERT_EQ(true, is_triggered);
    ASSERT_EQ(E_SH_1, scheduler_id);
}

TEST_F(TestCoroutine, TimeoutNotTriggered)
{
    coro::AddScheduler(E_SH_1, "main");
    bool is_triggered(false);
    uint32_t scheduler_id(coro::ERROR_SCHEDULER_ID);
    coro::tResumeHandle h_resume;
    coro::Run([this, &is_triggered, &scheduler_id, &h_resume]
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
    IncWait();
    coro::Resume(h_resume, E_SH_1);
    IncWait();
    ASSERT_EQ(false, is_triggered);
    ASSERT_EQ(E_SH_1, scheduler_id);
}
