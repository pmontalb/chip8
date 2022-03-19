
#include <gtest/gtest.h>
#include "Emulator/Rng.h"

class RngTests: public ::testing::Test
{
};

TEST_F(RngTests, InitializationWithNoSeed)
{
	emu::Rng rng;
	ASSERT_GE(rng.Next(), std::numeric_limits<emu::Byte>::min());
	ASSERT_LE(rng.Next(), std::numeric_limits<emu::Byte>::max());
}


TEST_F(RngTests, InitializationWithSeed)
{
	emu::Rng rng(1234);
	ASSERT_GE(rng.Next(), std::numeric_limits<emu::Byte>::min());
	ASSERT_LE(rng.Next(), std::numeric_limits<emu::Byte>::max());
}

TEST_F(RngTests, InitializationWithSeedConsistentValues)
{
	emu::Rng rng(1234);

	std::array<emu::Byte, 5> expectedValues = {{
		0x31, 0x7F, 0x9F, 0xD1, 0x70
	}};

	for (size_t i = 0; i < expectedValues.size(); ++i)
		EXPECT_EQ(rng.Next(), expectedValues[i]) << i;
}

