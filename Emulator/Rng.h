
#pragma once

#include "Types.h"
#include "Interfaces/IRng.h"

#include <random>

namespace emu
{
	class Rng final: public IRng
	{
	public:
		explicit Rng(unsigned int seed);

		explicit Rng();

		[[nodiscard]] Byte Next() override;

	private:
		std::random_device _device;
		std::mt19937 _generator;

		std::uniform_int_distribution<Byte> _distribution { std::numeric_limits<Byte>::min(), std::numeric_limits<Byte>::max() };
	};
}	 // namespace emu
