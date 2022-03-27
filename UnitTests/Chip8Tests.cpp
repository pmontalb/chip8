
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
};

class Chip8Tests: public ::testing::Test
{
public:
	bool ExecuteInstruction(TestChip8& chip8, const emu::TwoBytes instruction)
	{
		chip8.cpu._programCounter = 0;
		chip8.ram.data[chip8.cpu._programCounter] = static_cast<emu::Byte>((instruction & 0xFF00u) >> 8);
		chip8.ram.data[chip8.cpu._programCounter + 1] = instruction & 0x00FFu;

		return chip8.Cycle();
	}
};

TEST_F(Chip8Tests, Initialization)
{
	TestChip8 chip8;
	ASSERT_EQ(chip8.cpu.GetProgramCounter(), emu::Ram::instructionStart);
	ASSERT_TRUE(std::all_of(chip8.display.GetPixels().begin(), chip8.display.GetPixels().end(), [](auto pixel) { return !pixel; }));
}

TEST_F(Chip8Tests, ExecuteUniquePatternInstructionSets)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x1a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x1nnn);
	ASSERT_EQ(chip8.cpu.GetProgramCounter(), 0xa24);

	chip8.cpu._stackPointer = 0;
	ASSERT_TRUE(ExecuteInstruction(chip8, 0x2a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x2nnn);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x3a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x3xkk);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x4a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x4xkk);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x5a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x5xy0);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x6a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x6xkk);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x7a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x7xkk);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x9a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x9xy0);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xaa24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xAnnn);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xba24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xBnnn);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xca24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xCxkk);

	chip8.cpu._indexRegister = 0;
	chip8.ram.data.resize(16);
	ASSERT_TRUE(ExecuteInstruction(chip8, 0xda24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xDxyn);
}

TEST_F(Chip8Tests, ExecuteInstruction0x00E)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x00E0));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x00E0);

	chip8.cpu._stackPointer = 1;
	ASSERT_TRUE(ExecuteInstruction(chip8, 0x00EE));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x00EE);
}

TEST_F(Chip8Tests, ExecuteInstruction0x80xy)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a0));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy0);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a1));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy1);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a2));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy2);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a3));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy3);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a4));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy4);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a5));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy5);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a6));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy6);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a7));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xy7);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81aE));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x8xyE);
}

TEST_F(Chip8Tests, ExecuteInstruction0xExyz)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xEAA1));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xExA1);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xE09E));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xEx9E);
}

TEST_F(Chip8Tests, ExecuteInstruction0xFxyz)
{
	TestChip8 chip8;
	chip8.cpu._programCounter = 0;
	chip8.ram.data.resize(16);
	chip8.ram.fonts.resize(10);
	chip8.ram.data[chip8.cpu._programCounter] = 0xA9;
	chip8.ram.data[chip8.cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD07));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx07);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD0A));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx0A);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD15));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx15);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD18));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx18);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD1E));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx1E);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD29));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx29);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD33));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx33);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD55));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx55);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD65));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xFx65);
}

TEST_F(Chip8Tests, TestOpCode)
{
	spdlog::set_level(spdlog::level::info);
	emu::Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/test_opcode.ch8"));

	for (size_t i = 0; i < 5000; ++i)
		ASSERT_TRUE(chip8.Cycle());
}
