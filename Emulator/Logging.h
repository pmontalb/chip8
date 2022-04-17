
#pragma once

#include "IgnoreWarning.h"
#include "RingBuffer.h"

#if defined(__clang__)
	#define __IGNORE_SPDLOG_WARNINGS__                                                                                 \
		__IGNORE_WARNING__("-Weffc++")                                                                                 \
		__IGNORE_WARNING__("-Wsigned-enum-bitfield")                                                                   \
		__IGNORE_WARNING__("-Wnewline-eof")                                                                            \
		__IGNORE_WARNING__("-Wundefined-func-template")                                                                \
		__IGNORE_WARNING__("-Wreserved-identifier")                                                                    \
		__IGNORE_WARNING__("-Wdocumentation-unknown-command")
#else
	#define __IGNORE_SPDLOG_WARNINGS__                                                                                 \
		__IGNORE_WARNING__("-Weffc++")                                                                                 \
		__IGNORE_WARNING__("-Wctor-dtor-privacy")                                                                      \
		__IGNORE_WARNING__("-Wswitch-default")
#endif

__START_IGNORING_WARNINGS__
__IGNORE_SPDLOG_WARNINGS__
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"
__STOP_IGNORING_WARNINGS__

#ifndef DISABLE_LOGGING
	#ifndef LOGGER_NAME
		#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)
		#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
		#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
		#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
		#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
		#define LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
	#else
		#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(LOGGER_NAME), __VA_ARGS__)
		#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::get(LOGGER_NAME), __VA_ARGS__)
		#define LOG_WARN(...) SPDLOG_LOGGER_WARN(spdlog::get(LOGGER_NAME), __VA_ARGS__)
		#define LOG_INFO(...) SPDLOG_LOGGER_INFO(spdlog::get(LOGGER_NAME), __VA_ARGS__)
		#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::get(LOGGER_NAME), __VA_ARGS__)
		#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::get(LOGGER_NAME), __VA_ARGS__)
	#endif
#else
	#define LOG_CRITICAL(...) static_cast<void>(0)
	#define LOG_ERROR(...) static_cast<void>(0)
	#define LOG_WARN(...) static_cast<void>(0)
	#define LOG_INFO(...) static_cast<void>(0)
	#define LOG_DEBUG(...) static_cast<void>(0)
	#define LOG_TRACE(...) static_cast<void>(0)
#endif

namespace detail
{
#ifdef __clang__
	__START_IGNORING_WARNINGS__
	__IGNORE_SPDLOG_WARNINGS__
#endif
	template<typename Mutex>
	class RingBufferSink final:
		public spdlog::sinks::base_sink<Mutex>
#ifdef __clang__
			__STOP_IGNORING_WARNINGS__
#endif
	{
	public:
		auto& GetRingBuffer() { return _ringBuffer; }

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

			_ringBuffer.Add(std::make_pair(msg.level, fmt::to_string(formatted)));
		}

		void flush_() override {}

	private:
		static constexpr auto ringbufferSize = 64;
		utils::RingBuffer<std::pair<spdlog::level::level_enum, std::string>, ringbufferSize> _ringBuffer {};
	};
}	 // namespace detail

namespace utils
{
	using RingBufferSinkSt = ::detail::RingBufferSink<spdlog::details::null_mutex>;
	using RingBufferSinkMt = ::detail::RingBufferSink<std::mutex>;
}	 // namespace utils
