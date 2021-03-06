#include "TCoroutine.h"

#include <thread>
#include "Interface.h"
#include "helper/TConst.h"
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

TEST_F(TestCoroutine, TaskWithoutYield)
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

TEST_F(TestCoroutine, TaskWithOneYield)
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

TEST_F(TestCoroutine, TaskWithTwoYield)
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

TEST_F(TestCoroutine, ResumeInOtherScheduler)
{
    coro::AddScheduler(E_SH_1, "main");
    coro::AddScheduler(E_SH_2, "dop");
    coro::tResumeHandle h_resume;
    coro::Run([this, &h_resume]
              {
                  ASSERT_EQ(E_SH_1, coro::CurrentSchedulerId());
                  h_resume = coro::CurrentResumeId();
                  IncNotify();

                  coro::yield();

                  ASSERT_EQ(E_SH_2, coro::CurrentSchedulerId());
                  IncNotify();
              }, E_SH_1);
    IncWait();
    coro::Resume(h_resume, E_SH_2);
    IncWait();
}

TEST_F(TestCoroutine, RaiseTimeOutExceptionForLongCall)
{
    coro::AddScheduler(E_SH_1, "main");
    coro::Run([this]
              {
                  try
                  {
                      auto short_timeout = std::chrono::milliseconds(100);
                      coro::CTimeout t(short_timeout);
                      coro::yield();
                      FAIL();
                  }
                  catch (const coro::TimeoutError&)
                  {
                      ASSERT_EQ(E_SH_1, coro::CurrentSchedulerId());
                  }
                  catch (...)
                  {
                      FAIL();
                  }
                  IncNotify();
              }, E_SH_1);
    IncWait();
}

TEST_F(TestCoroutine, NoExceptionIfTimeoutExitFromScope)
{
    coro::AddScheduler(E_SH_1, "main");
    coro::tResumeHandle h_resume;
    coro::Run([this, &h_resume]
              {
                  try
                  {
                      {
                          auto canceled_timeout = std::chrono::milliseconds(0);
                          coro::CTimeout t(canceled_timeout);
                      }
                      auto not_expired_timeout = std::chrono::seconds(10);
                      coro::CTimeout t(not_expired_timeout);

                      auto wait_expired_canceled_timeout = std::chrono::milliseconds(100);
                      std::this_thread::sleep_for(wait_expired_canceled_timeout);
                      
                      h_resume = coro::CurrentResumeId();
                      IncNotify();
                      coro::yield();
                      SUCCEED();
                      IncNotify();
                  }
                  catch (const coro::TimeoutError&)
                  {
                      FAIL();
                  }
                  catch (...)
                  {
                      FAIL();
                  }
                  IncNotify();
              }, E_SH_1);
    IncWait();
}

TEST_F(TestCoroutine, NoExceptionIfOperationCompletedBeforeTimeOut)
{
    coro::AddScheduler(E_SH_1, "main");
    coro::tResumeHandle h_resume;
    coro::Run([this, &h_resume]
              {
                  try
                  {
                      auto long_timeout = std::chrono::seconds(10);
                      coro::CTimeout t(long_timeout);
                      h_resume = coro::CurrentResumeId();
                      IncNotify();
                      coro::yield();
                      ASSERT_EQ(E_SH_1, coro::CurrentSchedulerId());
                      IncNotify();
                  }
                  catch (const coro::TimeoutError&)
                  {
                      FAIL();
                  }
                  catch (...)
                  {
                      FAIL();
                  }
              }, E_SH_1);
    IncWait();
    coro::Resume(h_resume, E_SH_1);
    IncWait();
}

TEST_F(TestCoroutine, StopInfiniteLoopCoroutine)
{
    coro::AddScheduler(E_SH_1, "main");
    coro::Run([]
              {
                  while(true) {};
                  FAIL();
              }, E_SH_1);
    coro::Stop(std::chrono::milliseconds(10));
}
