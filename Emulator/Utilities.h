
#pragma once

#include "Types.h"
#include <type_traits>

namespace utils
{
	template <auto Start, auto End, class F>
	constexpr void constexprFor(F&& loopBody)
	{
		if constexpr (Start < End)
		{
			loopBody(std::integral_constant<decltype(Start), Start>());
			constexprFor<Start + 1, End>(loopBody);
		}
	}

	namespace detail
	{
		template<typename T>
		static constexpr T Mask(const T value, const T mask) { return value & mask; }

		template<typename T>
		static constexpr T ShiftDown(const T value, const emu::Byte shift) { return value >> shift; }

	}	 // namespace detail
	static constexpr emu::TwoBytes LowerTwelveBits(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0xFFFu;
		return detail::Mask(instruction, mask);
	}
	static constexpr emu::Byte LowestFourBits(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x00Fu;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 8 bits
		constexpr emu::Byte rightShift = 8u;
		return detail::ShiftDown(maskedValue, rightShift);
	}
	static constexpr emu::Byte LowerFourBitsHighByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0xF00u;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 8 bits
		constexpr emu::Byte rightShift = 8u;
		return detail::ShiftDown(maskedValue, rightShift);
	}
	static constexpr emu::Byte UpperFourBitsLowByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x00F0u;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 4 bits
		constexpr auto rightShift = 4u;
		return detail::ShiftDown(maskedValue, rightShift);
	}
	static constexpr emu::Byte LowestByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x0FFu;
		return detail::Mask(instruction, mask);
	}
	// least significant
	static constexpr emu::Byte LastBit(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x001u;
		return detail::Mask(instruction, mask);
	}
	// most significant
	static constexpr emu::Byte FirstBit(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x080u;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 7 bits
		constexpr auto rightShift = 7u;
		return detail::ShiftDown(maskedValue, rightShift);
	}
	// most significant
	template<emu::Byte index>
	static constexpr emu::Byte GetBitAt(const emu::TwoBytes instruction)
	{
		constexpr emu::Byte maxShift = 8;
		static_assert(index < maxShift);

		constexpr emu::TwoBytes mask = 0x080u >> index;
		return detail::Mask(instruction, mask);
	}
}	 // namespace utils
