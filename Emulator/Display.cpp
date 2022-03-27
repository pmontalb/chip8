
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

	void Display::Reset()
	{
		_hasChanged = false;
	}

	bool Display::HasChanged() const
	{
		return _hasChanged;
	}

	bool Display::GetAt(const std::size_t coord) const
	{
		assert(coord < _data.size());
		return _data[coord];
	}
}
