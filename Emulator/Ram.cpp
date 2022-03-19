
#include "Ram.h"

#include <vector>

namespace emu
{
	Ram::Ram()
	{
		// reset
		std::memset(_data.begin(), 0, _data.size() * sizeof(Byte));

		// store fonts in memory
		CopyFrom(fontsOffset, fonts.data(), fonts.size());
	}

	void Ram::SetAt(const size_t index, const Byte value)
	{
		assert(index >= Ram::instructionStart);
		_data[index] = value;
	}

	void Ram::Load(const std::string& buffer)
	{
		std::memcpy(_data.begin() + instructionStart, buffer.data(), buffer.size() * sizeof(Byte));

		// reset the rest of the memory
		std::fill(_data.begin() + buffer.size(), _data.end(), 0x0);
	}

	Byte Ram::GetFontAt(const size_t index) const
	{
		const auto offset = fontsOffset + index * fontSize;
		assert(offset < endFonts);
		return _data[offset];
	}
	void Ram::CopyFrom(const size_t memoryStart, const Byte* source, const size_t nElements)
	{
		std::memcpy(_data.begin() + memoryStart, source, nElements * sizeof(Byte));
	}
	void Ram::WriteTo(const size_t memoryStart, Byte* dest, const size_t nElements) const
	{
		std::memcpy(dest, _data.begin() + memoryStart, nElements * sizeof(Byte));
	}

}	 // namespace emu
