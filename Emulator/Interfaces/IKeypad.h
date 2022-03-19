
#pragma once

#include "Emulator/Types.h"

namespace emu
{
	namespace Keys
	{
		/*
			* Keypad       Keyboard
			+-+-+-+-+    +-+-+-+-+
			|1|2|3|C|    |1|2|3|4|
			+-+-+-+-+    +-+-+-+-+
			|4|5|6|D|    |Q|W|E|R|
			+-+-+-+-+ => +-+-+-+-+
			|7|8|9|E|    |A|S|D|F|
			+-+-+-+-+    +-+-+-+-+
			|A|0|B|F|    |Z|X|C|V|
			+-+-+-+-+    +-+-+-+-+
		*/
		enum Enum
		{
			START = 0,
			One = START,
			Two,
			Three,
			C,
			Four,
			Five,
			Six,
			D,
			Seven,
			Eight,
			Nine,
			E,
			A,
			Zero,
			B,
			F,
			END,
		};
	}	 // namespace Keys

	class IKeypad
	{
	public:
		virtual ~IKeypad() = default;
		[[nodiscard]] virtual bool IsPressed(const Keys::Enum keyCode) const = 0;
		[[nodiscard]] virtual Byte GetSize() const = 0;
		virtual void Press(const Keys::Enum keyCode, const bool toggle) = 0;
	};
}
