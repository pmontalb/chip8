
#include "Ram.h"

#include "Emulator/Logging.h"
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
		LOG_DEBUG("ram[{0:d}|{0:X}] = {1}", index, value);

		_data[index] = value;
	}

	void Ram::Load(const std::string& buffer)
	{
		LOG_DEBUG("loaded from buffer, size({})", buffer.size());

		std::memcpy(_data.begin() + instructionStart, buffer.data(), buffer.size() * sizeof(Byte));

		// reset the rest of the memory
		std::fill(_data.begin() + instructionStart + buffer.size(), _data.end(), 0x0);
	}

	Byte Ram::GetFontAddressAt(const size_t index) const
	{
		const auto address = fontsOffset + index * fontSize;
		assert(address < endFonts);
		LOG_DEBUG("index({}) offset({}) fontSize({}): address({})", index, fontsOffset, fontSize, address);

		return static_cast<Byte>(address);
	}
	void Ram::CopyFrom(const size_t memoryStart, const Byte* source, const size_t nElements)
	{
		LOG_DEBUG("memoryStart({}) source({}) nElements({})", memoryStart, fmt::ptr(source), nElements);
		std::memcpy(_data.begin() + memoryStart, source, nElements * sizeof(Byte));
	}
	void Ram::WriteTo(const size_t memoryStart, Byte* dest, const size_t nElements) const
	{
		LOG_DEBUG("memoryStart({}) dest({}) nElements({})", memoryStart, fmt::ptr(dest), nElements);
		std::memcpy(dest, _data.begin() + memoryStart, nElements * sizeof(Byte));
	}

	void Ram::Serialize(std::vector<Byte>& byteArray) const
	{
		byteArray.reserve(byteArray.size() + size);
		std::copy(_data.begin(), _data.end(), std::back_inserter(byteArray));
	}

	utils::Span<Byte> Ram::Deserialize(const utils::Span<Byte>& byteArray)
	{
		std::copy(byteArray.begin(), byteArray.begin() + size, _data.begin());
		return utils::Span<Byte>{ byteArray.begin() + size, byteArray.end() };
	}

}	 // namespace emu
