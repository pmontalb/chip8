
#include <gtest/gtest.h>
#include "Emulator/Chip8.h"

#include "Emulator/Logging.h"

class Chip8Tests: public ::testing::Test
{
};

TEST_F(Chip8Tests, DISABLED_TestRom1)
{
	spdlog::set_level(spdlog::level::trace);
	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);

	emu::Chip8 chip8;
	if (!chip8.LoadRom(std::string(dataPath) + "/test_rom_1.ch8"))
		return;

	size_t i = 0;
	while(true)
	{
		chip8.Cycle();
		++i;
		if (i > 5000)
			break;
	}

}

TEST_F(Chip8Tests, DISABLED_TestPong)
{
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");
	spdlog::set_level(spdlog::level::trace);
	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);

	emu::Chip8 chip8;
	if (!chip8.LoadRom(std::string(dataPath) + "/PONG"))
		return;

	size_t i = 0;
	while (true)
	{
		chip8.Cycle();
		++i;
		if (i > 5000)
			break;
	}
}
