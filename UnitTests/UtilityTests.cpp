
#include "Emulator/Utilities.h"

#include <bitset>
#include <gtest/gtest.h>

class UtilityTests: public ::testing::Test
{
};

template<size_t A, size_t B>
struct CompileTimeEqualityCheck
{
	static constexpr bool value = A == B;
};


TEST_F(UtilityTests, ConstexprFor)
{
	constexpr std::array<double, 4> a = {{
		0.1234,
		0.2345,
		0.3456,
		0.4567,
	}};
	constexpr std::array<double, 4> b = {{
		0.4567,
		0.3456,
		0.2345,
		0.1234,
	}};

	constexpr bool compileTimeTest = [&]() {
		static_assert(a.size() == b.size());
		utils::ConstexprFor<0, 4>([&](auto i) { static_assert((a[i] - b[b.size() - 1 - i]) < 1e-16); });
		return true;
	}();
	static_assert(compileTimeTest);

	// runtime test
	utils::ConstexprFor<0, 4>([&](auto i) { ASSERT_DOUBLE_EQ(a[i], b[b.size() - 1 - i]); });
}

TEST_F(UtilityTests, LowerTwelveBits)
{
	constexpr emu::TwoBytes value = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::LowerTwelveBits(value), 0x0A2F>::value);
	ASSERT_EQ(utils::LowerTwelveBits(value), 0x0A2F);
}

TEST_F(UtilityTests, LowestFourBits)
{
	constexpr emu::TwoBytes value = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::LowestFourBits(value), 0x000F>::value);
	ASSERT_EQ(utils::LowestFourBits(value), 0x000F);
}

TEST_F(UtilityTests, LowerFourBitsHighByte)
{
	constexpr emu::TwoBytes value = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::LowerFourBitsHighByte(value), 0x000A>::value);
	ASSERT_EQ(utils::LowerFourBitsHighByte(value), 0x000A);
}

TEST_F(UtilityTests, UpperFourBitsLowByte)
{
	constexpr emu::TwoBytes value = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::UpperFourBitsLowByte(value), 0x0002>::value);
	ASSERT_EQ(utils::UpperFourBitsLowByte(value), 0x0002);
}

TEST_F(UtilityTests, LowestByte)
{
	constexpr emu::TwoBytes value = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::LowestByte(value), 0x002F>::value);
	ASSERT_EQ(utils::LowestByte(value), 0x002F);
}

TEST_F(UtilityTests, HighestByte)
{
	constexpr emu::TwoBytes value = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::HighestByte(value), 0x00FA>::value);
	ASSERT_EQ(utils::HighestByte(value), 0x00FA);
}

TEST_F(UtilityTests, LastBit)
{
	constexpr emu::TwoBytes value1 = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::LastBit(value1), 1>::value);
	ASSERT_EQ(utils::LastBit(value1), 0x0001);

	constexpr emu::TwoBytes value2 = 0xFA2E;
	static_assert(CompileTimeEqualityCheck<utils::LastBit(value2), 0>::value);
	ASSERT_EQ(utils::LastBit(value2), 0x0000);

	constexpr emu::TwoBytes value3 = 0xFA2D;
	static_assert(CompileTimeEqualityCheck<utils::LastBit(value3), 1>::value);
	ASSERT_EQ(utils::LastBit(value3), 0x0001);
}


TEST_F(UtilityTests, FirstBit)
{
	constexpr emu::TwoBytes value1 = 0xFAEF;
	static_assert(CompileTimeEqualityCheck<utils::FirstBit(value1), 1>::value);
	ASSERT_EQ(utils::FirstBit(value1), 0x0001);

	constexpr emu::TwoBytes value2 = 0xFAAF;
	static_assert(CompileTimeEqualityCheck<utils::FirstBit(value2), 1>::value);
	ASSERT_EQ(utils::FirstBit(value2), 0x0001);

	constexpr emu::TwoBytes value3 = 0xFA2F;
	static_assert(CompileTimeEqualityCheck<utils::FirstBit(value3), 0>::value);
	ASSERT_EQ(utils::FirstBit(value3), 0x0000);
}

TEST_F(UtilityTests, GetBitAt)
{
	constexpr emu::TwoBytes value = 0xFA2F;

	constexpr bool compileTimeTest = [&]() {
		utils::ConstexprFor<0, 8>(
			[&](auto i)
			{
				if constexpr (i == 0)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x0>::value);
				else if constexpr (i == 1)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x0>::value);
				else if constexpr (i == 2)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x20>::value);
				else if constexpr (i == 3)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x0>::value);
				else if constexpr (i == 4)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x8>::value);
				else if constexpr (i == 5)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x4>::value);
				else if constexpr (i == 6)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x2>::value);
				else if constexpr (i == 7)
					static_assert(CompileTimeEqualityCheck<utils::GetBitAt<i>(value), 0x1>::value);
				else
					static_assert(CompileTimeEqualityCheck<i, std::numeric_limits<decltype(i)>::max()>::value);
			});
		return true;
	}();
	static_assert(compileTimeTest);

	// runtime test
	ASSERT_EQ(utils::GetBitAt<0>(value), 0x00);
	ASSERT_EQ(utils::GetBitAt<1>(value), 0x00);
	ASSERT_EQ(utils::GetBitAt<2>(value), 0x20);
	ASSERT_EQ(utils::GetBitAt<3>(value), 0x00);
	ASSERT_EQ(utils::GetBitAt<4>(value), 0x08);
	ASSERT_EQ(utils::GetBitAt<5>(value), 0x04);
	ASSERT_EQ(utils::GetBitAt<6>(value), 0x02);
	ASSERT_EQ(utils::GetBitAt<7>(value), 0x01);
}
