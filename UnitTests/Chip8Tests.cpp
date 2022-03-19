
#include <gtest/gtest.h>
#include "Emulator/Chip8.h"

class Chip8Tests: public ::testing::Test
{
};

TEST_F(Chip8Tests, TestRom1)
{
	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);

	emu::Chip8 chip8;
	chip8.LoadRom(std::string(dataPath) + "/test_rom_1.ch8");

	size_t i = 0;
	while(true)
	{
		chip8.Cycle();
		++i;
		if (i > 5000)
			break;
	}

}
