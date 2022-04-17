
#include "Emulator/Stopwatch.h"

#include <thread>
#include <gtest/gtest.h>

class StopwatchTests: public ::testing::Test
{
};

TEST_F(StopwatchTests, StartAtCreation)
{
	utils::StopWatch sw{};
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.Stop();

	ASSERT_GT(sw.GetNanoSeconds(), 0.0);
}

TEST_F(StopwatchTests, DelayedStart)
{
	utils::StopWatch sw(false);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	ASSERT_DOUBLE_EQ(sw.GetNanoSeconds(), 0.0);
}

TEST_F(StopwatchTests, Reset)
{
	utils::StopWatch sw{};
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.Reset();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.Stop();

	ASSERT_GT(sw.GetNanoSeconds(), 0.0);
}

TEST_F(StopwatchTests, Measurements)
{
	utils::StopWatch sw{};
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.Stop();

	const auto ns = sw.GetNanoSeconds();
	ASSERT_DOUBLE_EQ(sw.GetMicroSeconds(), ns * 1e-3);
	ASSERT_DOUBLE_EQ(sw.GetMilliSeconds(), ns * 1e-6);
	ASSERT_DOUBLE_EQ(sw.GetSeconds(), ns * 1e-9);
}
