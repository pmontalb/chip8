
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

	void Display::Serialize(std::vector<Byte>& byteArray) const
	{
		const auto flag = _data.to_ullong();
		static_assert(sizeof(flag) == 8);
		byteArray.reserve(byteArray.size() + sizeof(flag));
		utils::ConstexprFor<0, sizeof(flag)>(
			[&](const auto i) { byteArray[i] = static_cast<Byte>((flag & (0xFFul << 4 * i)) >> (4 * i)); });
	}

	void Display::Deserialize(const std::vector<Byte>& byteArray)
	{
		unsigned long long flag;
		utils::ConstexprFor<0, sizeof(flag)>([&](const auto i)
											 { flag |= static_cast<unsigned long>(byteArray[i] << 4 * i); });
		_data = std::bitset<size>(flag);
	}
}	 // namespace emu
