
#pragma once

#include "Emulator/Types.h"

namespace emu
{
	class IRng
	{
	public:
		virtual ~IRng() = default;
		[[nodiscard]] virtual Byte Next() = 0;
	};
}
