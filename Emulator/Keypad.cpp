
#include "Keypad.h"

namespace emu
{
	void Keypad::Press(const Keys::Enum keyCode, const bool toggle)
	{
		_data[keyCode] = toggle;
	}

	// https://stackoverflow.com/questions/5251403/binary-serialization-of-stdbitset
	void Keypad::Serialize(std::vector<Byte>& byteArray) const
	{
		constexpr size_t byteSize = size / 8;
		std::vector<Byte> localByteArray(byteSize);
		for (size_t i = 0; i < size; ++i)
			localByteArray[i >> 3] |= static_cast<Byte>(_data[i] << (i & 7));

		byteArray.reserve(byteArray.size() + byteSize);
		std::copy(localByteArray.begin(), localByteArray.end(), std::back_inserter(byteArray));
	}

	utils::Span<Byte> Keypad::Deserialize(const utils::Span<Byte>& byteArray)
	{
		for (size_t i = 0; i < size; ++i)
			_data[i] = static_cast<bool>((byteArray[i >> 3] >> (i & 7)) & 1);

		constexpr size_t byteSize = size / 8;
		return utils::Span<Byte>{ byteArray.begin() + byteSize, byteArray.end() };
	}

}	 // namespace emu
