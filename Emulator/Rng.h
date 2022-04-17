
#pragma once

#include "Types.h"
#include "Interfaces/IRng.h"

#ifdef CLANG_UBSAN
#pragma clang attribute push (__attribute__((no_sanitize("unsigned-integer-overflow"))), apply_to=function)
#endif
#include <random>
#ifdef CLANG_UBSAN
#pragma clang attribute pop
#endif

namespace emu
{
	class Rng final: public IRng
	{
	public:
		explicit Rng(unsigned int seed);

		explicit Rng();

		[[nodiscard]] Byte Next() override;

	private:
		std::random_device _device{};
		std::mt19937 _generator;

		std::uniform_int_distribution<Byte> _distribution { std::numeric_limits<Byte>::min(), std::numeric_limits<Byte>::max() };
	};
}	 // namespace emu
