
#include <gtest/gtest.h>
#include "Emulator/Chip8.h"

#include "Emulator/Logging.h"

struct TestKeyPad final: public emu::IKeypad
{
	std::bitset<emu::Keys::END> toggle {};

	[[nodiscard]] bool IsPressed(const emu::Keys::Enum code) const override { return toggle[code]; }
	[[nodiscard]] emu::Byte GetSize() const override { return 0; }
	void Press(const emu::Keys::Enum code, const bool toggle_) override { toggle[code] = toggle_; }
};

struct TestRam final: public emu::IRam
{
	std::vector<emu::Byte> data {};
	std::vector<emu::Byte> fonts {};

	[[nodiscard]] std::size_t GetSize() const override { return data.size(); }

	void Load(const std::string& /*buffer*/) override {}

	[[nodiscard]] emu::Byte GetAt(const std::size_t index) const override { return data.at(index); }
	void SetAt(const std::size_t index, const emu::Byte value) override { data[index] = value; }

	[[nodiscard]] emu::Byte GetFontAt(const std::size_t index) const override { return fonts[index]; }

	void CopyFrom(const std::size_t idx, const emu::Byte* source, const std::size_t nElems) override
	{
		for (size_t i = 0; i <= nElems; ++i)
			data[i + idx] = source[i];
	}
	void WriteTo(const std::size_t idx, emu::Byte* dest, const std::size_t nElems) const override
	{
		for (size_t i = 0; i <= nElems; ++i)
			dest[i] = data[i + idx];
	}
};

struct TestRng final: public emu::IRng
{
	emu::Byte totallyRandomNumber = 42;
	[[nodiscard]] emu::Byte Next() override { return totallyRandomNumber; }
};

struct TestDisplay final: public emu::IDisplay
{
	using Width = std::size_t;
	using Height = std::size_t;
	TestDisplay(const Width width = 8, const Height height = 12) : _width(width), _height(height) { Clear(); }

	[[nodiscard]] std::size_t GetWidth() const override { return _width; }
	[[nodiscard]] std::size_t GetHeight() const override { return _height; }
	void Clear() override { _pixels.assign(_width * _height, false); }

	void Reset() override {}
	[[nodiscard]] bool HasChanged() const override { return false; }

	// coord = (x % width) + (y % height) * width
	[[nodiscard]] bool GetAt(const std::size_t coord) const override { return _pixels.at(coord); }
	void FlipAt(const std::size_t coord) override { _pixels.at(coord).flip(); }

	auto& GetPixels() { return _pixels; }

private:
	std::vector<bool> _pixels {};
	const size_t _width;
	const size_t _height;
};

struct TestCpu: public emu::Cpu
{
	using emu::Cpu::_indexRegister;
	using emu::Cpu::_programCounter;

	using emu::Cpu::_registers;
	using emu::Cpu::_stack;
	using emu::Cpu::_stackPointer;

	using emu::Cpu::_delayTimer;
	using emu::Cpu::_soundTimer;
};

struct TestChip8: public emu::detail::Chip8<TestCpu, TestRng, TestRam, TestDisplay, TestKeyPad>
{
	using Base = emu::detail::Chip8<TestCpu, TestRng, TestRam, TestDisplay, TestKeyPad>;
	using Base::cpu;
	using Base::rng;
	using Base::ram;
	using Base::display;
	using Base::keypad;

	using Base::FetchInstruction;
	using Base::ExecuteInstruction;
};

class Chip8Tests: public ::testing::Test
{
};

TEST_F(Chip8Tests, Initialization)
{
	TestChip8 chip8;
	ASSERT_EQ(chip8.cpu.GetProgramCounter(), emu::Ram::instructionStart);
	ASSERT_TRUE(std::all_of(chip8.display.GetPixels().begin(), chip8.display.GetPixels().end(), [](auto pixel) { return !pixel; }));
}

TEST_F(Chip8Tests, FetchInstruction)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;
	ASSERT_EQ(chip8.FetchInstruction(), 0xA9F2);
}

TEST_F(Chip8Tests, ExecuteInstruction0x00E)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(chip8.ExecuteInstruction(0x00E0));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x00E0);

	chip8.cpu._stackPointer = 1;
	ASSERT_TRUE(chip8.ExecuteInstruction(0x00EE));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x00EE);
}

TEST_F(Chip8Tests, ExecuteInstructionUniqueSets)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(chip8.ExecuteInstruction(0x1a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x1nnn);
	ASSERT_EQ(chip8.cpu.GetProgramCounter(), 0xa24);

	chip8.cpu._stackPointer = 0;
	ASSERT_TRUE(chip8.ExecuteInstruction(0x2a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x2nnn);

	ASSERT_TRUE(chip8.ExecuteInstruction(0x3a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x3xkk);

	ASSERT_TRUE(chip8.ExecuteInstruction(0x4a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x4xkk);

	ASSERT_TRUE(chip8.ExecuteInstruction(0x5a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x5xy0);

	ASSERT_TRUE(chip8.ExecuteInstruction(0x6a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x6xkk);

	ASSERT_TRUE(chip8.ExecuteInstruction(0x7a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x7xkk);

	ASSERT_TRUE(chip8.ExecuteInstruction(0x9a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x9xy0);

	ASSERT_TRUE(chip8.ExecuteInstruction(0xaa24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xAnnn);

	ASSERT_TRUE(chip8.ExecuteInstruction(0xba24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xBnnn);

	ASSERT_TRUE(chip8.ExecuteInstruction(0xca24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xCxkk);

	chip8.cpu._indexRegister = 0;
	chip8.ram.data.resize(16);
	ASSERT_TRUE(chip8.ExecuteInstruction(0xda24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xDxyn);
}

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
