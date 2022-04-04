
#pragma once

#include "Emulator/Types.h"
#include <string>

namespace emu
{
	class IRam
	{
	public:
		virtual ~IRam() = default;
		[[nodiscard]] virtual std::size_t GetSize() const = 0;
		[[nodiscard]] virtual std::size_t GetInstructionStartAddress() const = 0;

		virtual void Load(const std::string& buffer) = 0;

		[[nodiscard]] virtual Byte GetAt(const std::size_t index) const = 0;
		virtual void SetAt(const std::size_t index, const Byte value) = 0;

		[[nodiscard]] virtual Byte GetFontAddressAt(const std::size_t index) const = 0;

		virtual void CopyFrom(const std::size_t memoryStart, const Byte* source, const std::size_t nElements) = 0;
		virtual void WriteTo(const std::size_t memoryStart, Byte* dest, const std::size_t nElements) const = 0;
	};
}
