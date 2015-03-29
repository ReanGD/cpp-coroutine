#pragma once
#ifndef TEST_SYNC_RUN_H
#define TEST_SYNC_RUN_H

#include "gtest/gtest.h"

class TestSyncRun
	: public ::testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;
};

#endif
