#include "TSyncRun.h"

#include <thread>
#include <chrono>
#include "Interface.h"
#include "helper/TLogger.h"

void TestSyncRun::SetUp()
{
    coro::Init(std::make_shared<CLogFake>());
}

void TestSyncRun::TearDown()
{
    coro::Stop(std::chrono::milliseconds(10));
}

enum E_SHEDULERS
{
    E_SH_1,
    E_SH_2
};

TEST_F(TestSyncRun, CompletedBeforeTimeout)
{
    coro::AddScheduler(E_SH_1, "main");
    auto operation_duration_limit = std::chrono::seconds(10);
    auto real_operation_duration = operation_duration_limit / 100;
    coro::SyncRun([&]
                  {
                      std::this_thread::sleep_for(real_operation_duration);
                      SUCCEED();
                  }, E_SH_1, operation_duration_limit);
}

TEST_F(TestSyncRun, CompletedByTimeout)
{
    coro::AddScheduler(E_SH_1, "main");
    auto operation_duration_limit = std::chrono::milliseconds(100);
    auto real_operation_duration = operation_duration_limit * 100;
    coro::SyncRun([&]
                  {
                      std::this_thread::sleep_for(real_operation_duration);
                      FAIL();
                  }, E_SH_1, operation_duration_limit);
}
