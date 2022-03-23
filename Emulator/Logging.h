
#pragma once

#include "spdlog/spdlog.h"

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
