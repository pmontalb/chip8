
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
	template<typename OStream>
	OStream &operator<<(OStream &os, const Keys::Enum key)
	{
		switch (key)
		{
			case Keys::One:
				return os << "1";
			case Keys::Two:
				return os << "2";
			case Keys::Three:
				return os << "3";
			case Keys::C:
				return os << "C";
			case Keys::Four:
				return os << "4";
			case Keys::Five:
				return os << "5";
			case Keys::Six:
				return os << "6";
			case Keys::D:
				return os << "D";
			case Keys::Seven:
				return os << "7";
			case Keys::Eight:
				return os << "8";
			case Keys::Nine:
				return os << "9";
			case Keys::E:
				return os << "E";
			case Keys::A:
				return os << "A";
			case Keys::Zero:
				return os << "0";
			case Keys::B:
				return os << "B";
			case Keys::F:
				return os << "F";
			case Keys::END:
				return os << "???";
		}
	}

	class IKeypad
	{
	public:
		virtual ~IKeypad() = default;
		[[nodiscard]] virtual bool IsPressed(const Keys::Enum keyCode) const = 0;
		[[nodiscard]] virtual Byte GetSize() const = 0;
		virtual void Press(const Keys::Enum keyCode, const bool toggle) = 0;
	};
}
