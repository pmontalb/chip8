
#include "Keypad.h"

namespace emu
{
	void Keypad::Press(const Keys::Enum keyCode, const bool toggle)
	{
		_data[keyCode] = toggle;
	}

}	 // namespace emu
