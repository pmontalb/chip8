
#include "Display.h"
#include "Logging.h"

#include <cassert>

namespace emu
{
	void Display::Clear()
	{
		LOG_TRACE("clearing");
		_data.reset();
		_hasChanged = true;
	}

	void Display::FlipAt(const std::size_t coord)
	{
		LOG_TRACE("coord({}) screen({} -> {})", coord, _data[coord], !_data[coord]);
		// this is an XOR with 1
		_data[coord] = !_data[coord];
		_hasChanged = true;
	}

	void Display::Reset() { _hasChanged = false; }

	bool Display::HasChanged() const { return _hasChanged; }

	bool Display::GetAt(const std::size_t coord) const
	{
		assert(coord < _data.size());
		return _data[coord];
	}

	// https://stackoverflow.com/questions/5251403/binary-serialization-of-stdbitset
	void Display::Serialize(std::vector<Byte>& byteArray) const
	{
		constexpr size_t byteSize = size / 8;
		std::vector<Byte> localByteArray(byteSize);
		for (size_t i = 0; i < size; ++i)
			localByteArray[i >> 3] |= static_cast<Byte>(_data[i] << (i & 7));

		byteArray.reserve(byteArray.size() + byteSize);
		std::copy(localByteArray.begin(), localByteArray.end(), std::back_inserter(byteArray));
	}

	utils::Span<Byte> Display::Deserialize(const utils::Span<Byte>& byteArray)
	{
		for (size_t i = 0; i < size; ++i)
			_data[i] = static_cast<bool>((byteArray[i >> 3] >> (i & 7)) & 1);

		constexpr size_t byteSize = size / 8;
		return utils::Span<Byte>{ byteArray.begin() + byteSize, byteArray.end() };
	}
}	 // namespace emu
