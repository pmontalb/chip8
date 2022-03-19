
#include "Rng.h"

namespace emu
{
	Rng::Rng(unsigned int seed)
		: _generator(_device())
	{
		_generator.seed(seed);
	}

	Rng::Rng()
		: _generator(_device())
	{
	}

	Byte Rng::Next() { return _distribution(_generator); }

}	 // namespace emu
