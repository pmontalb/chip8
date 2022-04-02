
#pragma once

#include "Types.h"
#include "Interfaces/IRam.h"
#include "ISerializable.h"

#include <array>
#include <cstring>
#include <string>
#include <cassert>

namespace emu
{
	namespace
	{
		// 5 bytes per font char, 16 chars, 0-F
		constexpr std::size_t fontSize = 5;
		constexpr std::size_t nFonts = 16;
		constexpr std::size_t fontsTotalSize = fontSize * nFonts;

		// example: the character 'F' is 0xF0, 0x80, 0xF0, 0x80, 0x80. Take a look at the binary representation:
		//	11110000
		//	10000000
		//	11110000
		//	10000000
		//	10000000
		constexpr std::array<Byte, fontsTotalSize> fonts = {
			0xF0, 0x90, 0x90, 0x90, 0xF0,	 // 0
			0x20, 0x60, 0x20, 0x20, 0x70,	 // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0,	 // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0,	 // 3
			0x90, 0x90, 0xF0, 0x10, 0x10,	 // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0,	 // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0,	 // 6
			0xF0, 0x10, 0x20, 0x40, 0x40,	 // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0,	 // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0,	 // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90,	 // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0,	 // B
			0xF0, 0x80, 0x80, 0x80, 0xF0,	 // C
			0xE0, 0x90, 0x90, 0x90, 0xE0,	 // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0,	 // E
			0xF0, 0x80, 0xF0, 0x80, 0x80	 // F
		};

		constexpr size_t fontsOffset = 0x050;
		constexpr size_t endFonts = 0x0A0;
		static_assert(fontsOffset + fontsTotalSize == endFonts);
	}	 // namespace

	class Ram final: public IRam, public ISerializable
	{
		static constexpr std::size_t size = 4096;
		static constexpr std::size_t instructionStart = 0x200;
	public:


		Ram();

		void Load(const std::string& buffer) override;

		[[nodiscard]] size_t GetSize() const override { return size; }
		[[nodiscard]] std::size_t GetInstructionStartAddress() const override { return instructionStart; }

		[[nodiscard]] Byte GetAt(const size_t index) const override
		{
			return _data[index];
		}
		void SetAt(const size_t index, const Byte value) override;
		void CopyFrom(const size_t memoryStart, const Byte* source, const size_t nElements) override;
		void WriteTo(const size_t memoryStart, Byte* dest, const size_t nElements) const override;

		[[nodiscard]] Byte GetFontAt(const size_t index) const override;

		void Serialize(std::vector<Byte>& byteArray) const override;
		void Deserialize(const std::vector<Byte>& byteArray) override;

	private:
		// 0x000-0x1FF not in use
		// 0x050-0x0A0 we store the fonts
		// 0x200-0xFFF ROM instructions
		std::array<Byte, size> _data {};
	};
}	 // namespace emu
