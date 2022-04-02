
#pragma once

#include "Types.h"
#include <type_traits>
#include <array>

namespace utils
{
	template <auto Start, auto End, class F>
	constexpr void ConstexprFor(F&& loopBody)
	{
		if constexpr (Start < End)
		{
			loopBody(std::integral_constant<decltype(Start), Start>());
			ConstexprFor<Start + 1, End>(loopBody);
		}
	}

	template<typename T, std::size_t N>
	constexpr std::array<T, N> FlatInitializedArray(const T value = T())
	{
		std::array<T, N> ret {};
		ret.fill(value);
		return ret;
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
		constexpr emu::TwoBytes mask = 0x0FFFu;
		return detail::Mask(instruction, mask);
	}
	static constexpr emu::Byte LowestFourBits(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x000Fu;
		return static_cast<emu::Byte>(detail::Mask(instruction, mask));
	}
	static constexpr emu::Byte LowerFourBitsHighByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x0F00u;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 8 bits
		constexpr emu::Byte rightShift = 8u;
		return static_cast<emu::Byte>(detail::ShiftDown(maskedValue, rightShift));
	}
	static constexpr emu::Byte UpperFourBitsLowByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x00F0u;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 4 bits
		constexpr auto rightShift = 4u;
		return static_cast<emu::Byte>(detail::ShiftDown(maskedValue, rightShift));
	}
	static constexpr emu::Byte LowestByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x00FFu;
		return static_cast<emu::Byte>(detail::Mask(instruction, mask));
	}
	static constexpr emu::Byte HighestByte(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0xFF00u;
		const auto maskedValue = detail::Mask(instruction, mask);

		constexpr emu::Byte rightShift = 8u;
		return static_cast<emu::Byte>(detail::ShiftDown(maskedValue, rightShift));
	}
	// least significant
	static constexpr emu::Byte LastBit(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x0001u;
		return static_cast<emu::Byte>(detail::Mask(instruction, mask));
	}
	// most significant
	static constexpr emu::Byte FirstBit(const emu::TwoBytes instruction)
	{
		constexpr emu::TwoBytes mask = 0x0080u;
		const auto maskedValue = detail::Mask(instruction, mask);

		// to be able to compare the masked values with other bytes, we have to shift it down by 7 bits
		constexpr auto rightShift = 7u;
		return static_cast<emu::Byte>(detail::ShiftDown(maskedValue, rightShift));
	}

	template<emu::Byte index>
	static constexpr emu::Byte GetBitAt(const emu::TwoBytes instruction)
	{
		constexpr emu::Byte maxShift = 8;
		static_assert(index < maxShift);

		constexpr emu::TwoBytes mask = 0x0080u >> index;
		return static_cast<emu::Byte>(detail::Mask(instruction, mask));
	}
}	 // namespace utils
