
#pragma once

#include "Interfaces/IDisplay.h"
#include "Utilities.h"
#include "ISerializable.h"

#include <bitset>
#include <vector>

namespace emu
{
	class Display final: public IDisplay, public ISerializable
	{
		static constexpr std::size_t width = 64;
		static constexpr std::size_t height = 32;
		static constexpr std::size_t size = width * height;

	public:
		[[nodiscard]] std::size_t GetWidth() const override { return width; }
		[[nodiscard]] std::size_t GetHeight() const override { return height; }
		void Clear() override;

		[[nodiscard]] bool GetAt(const std::size_t coord) const override;

		void FlipAt(const std::size_t coord) override;

		void Reset() override;
		[[nodiscard]] bool HasChanged() const override;

		void Serialize(std::vector<Byte>& byteArray) const override;
		utils::Span<Byte> Deserialize(const utils::Span<Byte>& byteArray) override;

	private:
		std::bitset<size> _data = std::bitset<size>(0);
		bool _hasChanged = false;
	};
}	 // namespace emu
