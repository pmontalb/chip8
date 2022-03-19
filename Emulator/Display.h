
#pragma once

#include "Interfaces/IDisplay.h"
#include "Utilities.h"

#include <bitset>
#include <vector>

namespace emu
{
	class Display final: public IDisplay
	{
		static constexpr std::size_t width = 64;
		static constexpr std::size_t height = 32;
		static constexpr std::size_t size = width * height;

	public:
		[[nodiscard]] std::size_t GetWidth() const override { return width; }
		[[nodiscard]] std::size_t GetHeight() const override { return height; }
		void Clear() override;

		[[nodiscard]] bool GetAt(const std::size_t coord) const override
		{
			return _data[coord];
		}

		void FlipAt(const std::size_t coord) override
		{
			// this is an XOR with 1
			_data[coord] = !_data[coord];
			_hasChanged = true;
		}

		void Reset() override { _hasChanged = false; }
		[[nodiscard]] virtual bool HasChanged() const override { return _hasChanged; }

	private:
		std::bitset<size> _data = std::bitset<size>(0);
		bool _hasChanged = false;
	};
}	 // namespace emu
