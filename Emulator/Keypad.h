
#pragma once

#include "Interfaces/IKeypad.h"
#include "ISerializable.h"

#include <bitset>

namespace emu
{
	class Keypad final: public IKeypad, public ISerializable
	{
		static constexpr Byte size = 16;

	public:
		[[nodiscard]] bool IsPressed(const Keys::Enum keyCode) const override { return _data[keyCode]; }
		[[nodiscard]] Byte GetSize() const override { return static_cast<Byte>(_data.size()); }
		void Press(const Keys::Enum keyCode, const bool toggle) override;

		void Serialize(std::vector<Byte>& byteArray) const override;
		utils::Span<Byte> Deserialize(const utils::Span<Byte>& byteArray) override;

	private:
		std::bitset<size> _data {};
	};
}	 // namespace emu
