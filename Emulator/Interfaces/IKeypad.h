
#pragma once

#include "Emulator/Types.h"
#include <string_view>

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

	static inline std::string_view ToString(Keys::Enum key)
	{
		switch (key)
		{
			case Keys::One:
				return "1";
			case Keys::Two:
				return "2";
			case Keys::Three:
				return "3";
			case Keys::C:
				return "C";
			case Keys::Four:
				return "4";
			case Keys::Five:
				return "5";
			case Keys::Six:
				return "6";
			case Keys::D:
				return "D";
			case Keys::Seven:
				return "7";
			case Keys::Eight:
				return "8";
			case Keys::Nine:
				return "9";
			case Keys::E:
				return "E";
			case Keys::A:
				return "A";
			case Keys::Zero:
				return "0";
			case Keys::B:
				return "B";
			case Keys::F:
				return "F";
			case Keys::END:
				return "???";
			default:
				return "???";
		}
	}

	template<typename OStream>
	OStream &operator<<(OStream &os, const Keys::Enum key)
	{
		return os << ToString(key);
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
