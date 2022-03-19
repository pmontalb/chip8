
#pragma once

#include "Interfaces/IKeypad.h"
#include <bitset>

namespace emu
{
	class Keypad final: public IKeypad
	{
		static constexpr Byte size = 16;

	public:
		[[nodiscard]] bool IsPressed(const Keys::Enum keyCode) const override { return _data[keyCode]; }
		[[nodiscard]] Byte GetSize() const override { return _data.size(); };
		void Press(const Keys::Enum keyCode, const bool toggle) override;

	private:
		std::bitset<size> _data {};
	};
}	 // namespace emu
