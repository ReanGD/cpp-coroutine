#pragma once
#ifndef TEST_COROUTINE_H
#define TEST_COROUTINE_H

#include <atomic>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <condition_variable>
#include "gtest/gtest.h"

class TestCoroutine
	: public ::testing::Test
{
public:
    void SetUp() override;
protected:
    void IncNotify();
    void Wait(uint32_t val, const std::chrono::milliseconds& duration = std::chrono::milliseconds(1000));
protected:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<uint32_t> m_cv_counter;
};

#endif
