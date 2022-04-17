
#include "Emulator/Logging.h"

#include <gtest/gtest.h>

class LoggingTests: public ::testing::Test
{
};

TEST_F(LoggingTests, SinkSingleThreaded)
{
	constexpr const char* loggerName = "loggerNameSt";
	auto rbSink = std::make_shared<utils::RingBufferSinkSt>();
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(rbSink);
	spdlog::register_logger(std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end()));

	spdlog::get(loggerName)->set_level(spdlog::level::trace);
	spdlog::get(loggerName)->set_pattern("%v");

	ASSERT_EQ(rbSink->GetRingBuffer().Size(), 0);

	constexpr const char* logLine = "this line should be in the ring buffer sink";
	SPDLOG_LOGGER_CRITICAL(spdlog::get(loggerName), logLine);
	ASSERT_EQ(rbSink->GetRingBuffer().Size(), 1);
	ASSERT_EQ(rbSink->GetRingBuffer().Back().second, std::string(logLine) + "\n");
}

TEST_F(LoggingTests, SinkMultiThreaded)
{
	constexpr const char* loggerName = "loggerNameMt";
	auto rbSink = std::make_shared<utils::RingBufferSinkMt>();
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(rbSink);
	spdlog::register_logger(std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end()));

	spdlog::get(loggerName)->set_level(spdlog::level::trace);
	spdlog::get(loggerName)->set_pattern("%v");

	ASSERT_EQ(rbSink->GetRingBuffer().Size(), 0);

	constexpr const char* logLine = "this line should be in the ring buffer sink";
	SPDLOG_LOGGER_CRITICAL(spdlog::get(loggerName), logLine);
	spdlog::get(loggerName)->flush();

	ASSERT_EQ(rbSink->GetRingBuffer().Size(), 1);
	ASSERT_EQ(rbSink->GetRingBuffer().Back().second, std::string(logLine) + "\n");
}
