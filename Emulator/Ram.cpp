
#include "Ram.h"

#include "spdlog/spdlog.h"
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
		SPDLOG_DEBUG("ram[{0:d}|{0:X}] = {1}", index, value);

		_data[index] = value;
	}

	void Ram::Load(const std::string& buffer)
	{
		SPDLOG_DEBUG("loaded from buffer, size({})", buffer.size());

		std::memcpy(_data.begin() + instructionStart, buffer.data(), buffer.size() * sizeof(Byte));

		// reset the rest of the memory
		std::fill(_data.begin() + instructionStart + buffer.size(), _data.end(), 0x0);
	}

	Byte Ram::GetFontAt(const size_t index) const
	{
		const auto offset = fontsOffset + index * fontSize;
		assert(offset < endFonts);
		SPDLOG_DEBUG("index({}) offset({}) fontSize({}): value[{}]({})", index, fontsOffset, fontSize, offset, _data[offset]);

		return _data[offset];
	}
	void Ram::CopyFrom(const size_t memoryStart, const Byte* source, const size_t nElements)
	{
		SPDLOG_DEBUG("memoryStart({}) source({}) nElements({})", memoryStart, fmt::ptr(source), nElements);
		std::memcpy(_data.begin() + memoryStart, source, nElements * sizeof(Byte));
	}
	void Ram::WriteTo(const size_t memoryStart, Byte* dest, const size_t nElements) const
	{
		SPDLOG_DEBUG("memoryStart({}) dest({}) nElements({})", memoryStart, fmt::ptr(dest), nElements);
		std::memcpy(dest, _data.begin() + memoryStart, nElements * sizeof(Byte));
	}

}	 // namespace emu