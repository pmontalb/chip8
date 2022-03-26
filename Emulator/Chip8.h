
#pragma once

#include "Cpu.h"
#include "Display.h"
#include "Keypad.h"
#include "Ram.h"
#include "Rng.h"

#include <filesystem>

// https://austinmorlan.com/posts/chip8_emulator/
namespace emu
{
	class Chip8
	{
	public:
		Chip8();
		bool LoadRom(const std::filesystem::path& path);

		bool Cycle();


	private:
		TwoBytes FetchInstruction();
		bool ExecuteInstruction(const TwoBytes instruction);

	private:
		Cpu cpu {};
		Rng rng {};
		Ram ram {};
		Display display {};
		Keypad keypad {};
	};
}	 // namespace emu
