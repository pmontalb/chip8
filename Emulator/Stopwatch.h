
#pragma once

#include <cassert>
#include <ctime>

namespace utils
{
	class StopWatch
	{
		static constexpr auto clockId = CLOCK_MONOTONIC_RAW;
	public:
		explicit StopWatch(const bool start = true)
		{
			if (start)
				Start();
		}

		void Start()
		{
			const auto err = clock_gettime(clockId, &_cache);
			assert(err == 0);

			_start = 1e9 * static_cast<double>(_cache.tv_sec) + static_cast<double>(_cache.tv_nsec);
		}
		void Stop()
		{
			const auto err = clock_gettime(clockId, &_cache);
			assert(err == 0);

			_end = 1e9 * static_cast<double>(_cache.tv_sec) + static_cast<double>(_cache.tv_nsec);
		}
		void Reset()
		{
			_start = _end = 0;
			Start();
		}

		[[nodiscard]] auto GetNanoSeconds() const { return _end - _start; }
		[[nodiscard]] auto GetMicroSeconds() const { return GetNanoSeconds() * 1e-3; }
		[[nodiscard]] auto GetMilliSeconds() const { return GetNanoSeconds() * 1e-6; }
		[[nodiscard]] auto GetSeconds() const { return GetNanoSeconds() * 1e-9; }

	private:
		double _start = 0;
		double _end = 0;
		timespec _cache {};
	};
}
