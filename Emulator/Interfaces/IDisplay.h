
#pragma once

#include <cstddef>

namespace emu
{
	class IDisplay
	{
	public:
		virtual ~IDisplay() = default;

		[[nodiscard]] virtual std::size_t GetWidth() const = 0;
		[[nodiscard]] virtual std::size_t GetHeight() const = 0;
		virtual void Clear() = 0;

		virtual void Reset() = 0;
		[[nodiscard]] virtual bool HasChanged() const = 0;

		// coord = (x % width) + (y % height) * width
		[[nodiscard]] virtual bool GetAt(const std::size_t coord) const = 0;
		virtual void FlipAt(const std::size_t coord) = 0;
	};
}
