
#include "Display.h"

#include <cstring>

namespace emu
{
	void Display::Clear()
	{
		_data.reset();
		_hasChanged = true;
	}
}
