
#include "Emulator/Chip8.h"
#include "Emulator/Logging.h"

#include <gtest/gtest.h>

struct TestKeyPad final: public emu::IKeypad, public emu::ISerializable
{
	std::bitset<emu::Keys::END> toggle {};

	[[nodiscard]] bool IsPressed(const emu::Keys::Enum code) const override { return toggle[code]; }
	[[nodiscard]] emu::Byte GetSize() const override { return 0; }
	void Press(const emu::Keys::Enum code, const bool toggle_) override { toggle[code] = toggle_; }

	void Serialize(std::vector<emu::Byte>&) const override
	{

	}
	utils::Span<emu::Byte> Deserialize(const utils::Span<emu::Byte>&) override
	{
		return utils::Span<emu::Byte> {};
	}
};

struct TestRam final: public emu::IRam, public emu::ISerializable
{
	std::vector<emu::Byte> data {};
	std::vector<emu::Byte> fonts {};
	size_t instructionAddress = 0;

	[[nodiscard]] std::size_t GetSize() const override { return data.size(); }
	[[nodiscard]] std::size_t GetInstructionStartAddress() const override { return instructionAddress; }

	void Load(const std::string& /*buffer*/) override {}

	[[nodiscard]] emu::Byte GetAt(const std::size_t index) const override { return data.at(index); }
	void SetAt(const std::size_t index, const emu::Byte value) override { data[index] = value; }

	[[nodiscard]] emu::Byte GetFontAddressAt(const std::size_t index) const override { return fonts[index]; }

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

	void Serialize(std::vector<emu::Byte>&) const override { }
	utils::Span<emu::Byte> Deserialize(const utils::Span<emu::Byte>&) override
	{
		return utils::Span<emu::Byte> {};
	}
};

struct TestRng final: public emu::IRng
{
	emu::Byte totallyRandomNumber = 42;
	[[nodiscard]] emu::Byte Next() override { return totallyRandomNumber; }
};

struct TestDisplay final: public emu::IDisplay, public emu::ISerializable
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
	auto& GetPixels() const { return _pixels; }

	void Serialize(std::vector<emu::Byte>&) const override { }
	utils::Span<emu::Byte> Deserialize(const utils::Span<emu::Byte>&) override
	{
		return utils::Span<emu::Byte> {};
	}

private:
	std::vector<bool> _pixels {};
	size_t _width;
	size_t _height;
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
	using Base::_cpu;
	using Base::_display;
	using Base::_keypad;
	using Base::_ram;
	using Base::_rng;
};

class Chip8Tests: public ::testing::Test
{
public:
	bool ExecuteInstruction(TestChip8& chip8, const emu::TwoBytes instruction)
	{
		chip8._cpu._programCounter = 0;
		chip8._ram.data[chip8._cpu._programCounter] = static_cast<emu::Byte>((instruction & 0xFF00u) >> 8);
		chip8._ram.data[chip8._cpu._programCounter + 1] = instruction & 0x00FFu;

		return chip8.Cycle();
	}
};

TEST_F(Chip8Tests, Initialization)
{
	TestChip8 chip8;
	ASSERT_EQ(chip8.GetCpu().GetProgramCounter(), chip8.GetRam().GetInstructionStartAddress());
	ASSERT_TRUE(std::all_of(chip8.GetDisplay().GetPixels().begin(), chip8.GetDisplay().GetPixels().end(),
							[](auto pixel) { return !pixel; }));
	ASSERT_TRUE(chip8.GetKeypad().toggle.none());
}

TEST_F(Chip8Tests, ExecuteUniquePatternInstructionSets)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x1a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x1nnn);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x1a24);
	ASSERT_EQ(chip8._cpu.GetProgramCounter(), 0xa24);

	chip8._cpu._stackPointer = 0;
	ASSERT_TRUE(ExecuteInstruction(chip8, 0x2a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x2nnn);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x2a24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x3a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x3xkk);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x3a24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x4a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x4xkk);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x4a24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x5a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x5xy0);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x5a24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x6a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x6xkk);
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x6xkk);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x7a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x7a24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x9a24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x9xy0);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x9a24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xaa24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xAnnn);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xaa24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xba24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xBnnn);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xba24);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xca24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xCxkk);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xca24);

	chip8._cpu._indexRegister = 0;
	chip8._ram.data.resize(16);
	ASSERT_TRUE(ExecuteInstruction(chip8, 0xda24));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xDxyn);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xda24);
}

TEST_F(Chip8Tests, ExecuteInstruction0x00E)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x00E0));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x00E0);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x00E0);

	chip8._cpu._stackPointer = 1;
	ASSERT_TRUE(ExecuteInstruction(chip8, 0x00EE));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x00EE);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x00EE);

	// test invalid instruction
	ASSERT_FALSE(ExecuteInstruction(chip8, 0x00EF));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidInstruction);
}

TEST_F(Chip8Tests, ExecuteInstruction0x80xy)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a0));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy0);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a0);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a1));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy1);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a1);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a2));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy2);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a2);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a3));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy3);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a3);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a4));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy4);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a4);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a5));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy5);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a5);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a6));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy6);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a6);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81a7));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xy7);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81a7);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x81aE));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0x8xyE);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0x81aE);

	// test invalid instruction
	ASSERT_FALSE(ExecuteInstruction(chip8, 0x81aF));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidInstruction);
}

TEST_F(Chip8Tests, ExecuteInstruction0xExyz)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xEAA1));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xExA1);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xEAA1);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xE09E));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xEx9E);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xE09E);

	// test invalid instruction
	ASSERT_FALSE(ExecuteInstruction(chip8, 0xE09F));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidInstruction);
}

TEST_F(Chip8Tests, ExecuteInstruction0xFxyz)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(16);
	chip8._ram.fonts.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD07));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx07);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD07);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD0A));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx0A);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD0A);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD15));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx15);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD15);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD18));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx18);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD18);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD1E));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx1E);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD1E);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD29));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx29);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD29);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD33));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx33);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD33);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD55));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx55);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD55);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xFD65));
	ASSERT_EQ(chip8.GetLastExecutedInstructionCode(), emu::Instruction::_0xFx65);
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), 0xFD65);

	// test invalid instruction
	ASSERT_FALSE(ExecuteInstruction(chip8, 0xFD68));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidInstruction);
	ASSERT_FALSE(ExecuteInstruction(chip8, 0xFDFF));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidInstruction);
}

TEST_F(Chip8Tests, ExecuteInvalidInstruction)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(16);
	chip8._ram.fonts.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_FALSE(ExecuteInstruction(chip8, 0x00F9));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidInstruction);
}

TEST_F(Chip8Tests, TryToExecuteAnInstructionAtInvalidAddress)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.instructionAddress = 1;
	chip8._ram.data.resize(16);
	chip8._ram.fonts.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_FALSE(ExecuteInstruction(chip8, 0x00F9));
	ASSERT_EQ(chip8.GetLastError(), emu::Error::InvalidProgramCounter);
}

TEST_F(Chip8Tests, Rewind)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 4;
	chip8.Rewind();
	ASSERT_EQ(chip8._cpu._programCounter, 2);
}

TEST_F(Chip8Tests, LoadInvalidRom)
{
	TestChip8 chip8;
	ASSERT_FALSE(chip8.LoadRom("something_that/hopefully_does/not_exist"));
	ASSERT_FALSE(chip8.LoadRom("/"));
}

template<typename EmuT>
static void PrintDisplay(const EmuT& emulator)
{
	for (size_t row = 0; row < emulator.GetDisplay().GetHeight(); ++row)
	{
		for (size_t col = 0; col < emulator.GetDisplay().GetWidth(); ++col)
		{
			const size_t coord = col + row * emulator.GetDisplay().GetWidth();
			if (emulator.GetDisplay().GetAt(coord))
				std::cout << "█";
			else
				std::cout << " ";
		}
		std::cout << std::endl;
	}
}

template<typename EmuT>
static void CheckDisplay(const EmuT& emulator, const std::string& expected)
{
	std::string stringifiedDisplay {};
	stringifiedDisplay.reserve(emulator.GetDisplay().GetHeight() * emulator.GetDisplay().GetWidth());
	for (size_t row = 0; row < emulator.GetDisplay().GetHeight(); ++row)
	{
		for (size_t col = 0; col < emulator.GetDisplay().GetWidth(); ++col)
		{
			const size_t coord = col + row * emulator.GetDisplay().GetWidth();
			stringifiedDisplay += emulator.GetDisplay().GetAt(coord) ? '1' : '0';
		}
	}
	//	std::cout << stringifiedDisplay << std::endl;

	// make it smaller
	ASSERT_EQ(stringifiedDisplay.size(), emulator.GetDisplay().GetHeight() * emulator.GetDisplay().GetWidth());
	ASSERT_TRUE(stringifiedDisplay.size() % 4 == 0);

	std::stringstream hexStringifiedDisplay {};
	for (size_t i = 0; i < stringifiedDisplay.size() / 4; ++i)
	{
		size_t hex = 0;
		for (size_t j = 0; j < 4; ++j)
			hex += stringifiedDisplay[4 * i + j] == '1' ? (1ul << j) : 0ul;

		hexStringifiedDisplay << std::hex << hex;
	}
	//	std::cout << hexStringifiedDisplay.str() << std::endl;

	ASSERT_EQ(hexStringifiedDisplay.str(), expected);
}

TEST_F(Chip8Tests, TestOpCode)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/test_opcode.ch8"));

	for (size_t i = 0; i < 500; ++i)
	{
		//		LOG_CRITICAL("i={}", i);
		//		if (i >= 172 && i <= 184)
		//		{
		//			spdlog::set_level(spdlog::level::trace);
		//		}
		//		else
		//		{
		//			spdlog::set_level(spdlog::level::off);
		//		}
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();
	}

	PrintDisplay(chip8);

	CheckDisplay(
		chip8, "0000000000000000eac518b375076750c44d08a1530725308a4518a055054550eac518b3750727500000000000000000aac518b"
			   "375077750e44d08b2530715308a4518a2550575508ac518b3750777500000000000000000cac518b175077750444d0831530735"
			   "308a451821550515504ac518b3750777500000000000000000eac518b375076750844d0832530125308a4518a1550345508ac51"
			   "8b3750127500000000000000000eac518b375077750e44d0833530165308a45182255034550eac518b375017750000000000000"
			   "00004ac518b275035750a44d08b353022530ea45182255025550aac518327507575000000000000000000000000000000000");
}

TEST_F(Chip8Tests, TestRom1)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/test-rom.ch8"));

	for (size_t i = 0; i < 500; ++i)
	{
		//		LOG_CRITICAL("i={}", i);
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();
	}

	PrintDisplay(chip8);
	CheckDisplay(
		chip8, "f2100000000000009a0000000000000096000000000000009a00000000000000f21000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
}

TEST_F(Chip8Tests, TestRom2)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/c8_test.c8"));

	for (size_t i = 0; i < 500; ++i)
	{
		//		LOG_CRITICAL("i={}", i);
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();
	}

	PrintDisplay(chip8);
	CheckDisplay(
		chip8, "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "00000000000000000000000000000000000000000000000000000000c0900000000000002150000000000000213000000000000"
			   "02150000000000000c0900000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
}

TEST_F(Chip8Tests, IbmLogo)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/ibm_logo.ch8"));

	for (size_t i = 0; i < 500; ++i)
	{
		//		LOG_CRITICAL("i={}", i);
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();
	}

	PrintDisplay(chip8);
	CheckDisplay(
		chip8, "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000ffef3e308f0000000000000000000000ffeffe70cf0000000000000000000000c383e8f0e30"
			   "000000000000000000000c38f38fbf30000000000000000000000c38f38bfb30000000000000000000000c383e83f9300000000"
			   "00000000000000ffeffe3e8f0000000000000000000000ffef3e348f00000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
}

TEST_F(Chip8Tests, BcTest)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/BC_test.ch8"));

	for (size_t i = 0; i < 500; ++i)
	{
		//		LOG_CRITICAL("i={}", i);
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();
	}

	PrintDisplay(chip8);
	CheckDisplay(
		chip8, "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "000000000000000000000000000000000000000000000000000000000000000000000000000000e1c3240000000000222464000"
			   "00000002224a40000000000e124250000000000222426000000000022242400000000002224240000000000e1c3240000000000"
			   "000000000000000000000000000000000000000000000000000000000000000000000000000c00060001e0020004100a0001200"
			   "20004920a81332802600c8206490124135c04930ac01124923404120a40212492140c02068916e8036410830000000000000");
}

TEST_F(Chip8Tests, ScTest)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/SCTEST.CH8"));

	for (size_t i = 0; i < 500; ++i)
	{
		//		LOG_CRITICAL("i={}", i);
		if (i == 80 || i == 91)
		{
			// test rom assumes `1-1` sets V[0xF] = 1
			// but documentation http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy5
			// says otherwise
			// https://old.reddit.com/r/programming/comments/3ca4ry/writing_a_chip8_interpreteremulator_in_c14_10/csu7w8k/
			// above link means people have different opinion about it
			chip8._cpu.AdvanceProgramCounter();
			chip8._cpu.AdvanceProgramCounter();
		}
		if (i == 131)
		{
			// SCHIP instruction 0xFx75 and 0xFx85: skip
			for (size_t j = 0; j < 21; ++j, ++i)
				chip8._cpu.AdvanceProgramCounter();
		}
		if (i == 157)
		{
			// Fx1E buffer overflow, again, people have mixed opinions about it
			chip8._cpu.AdvanceProgramCounter();
			chip8._cpu.AdvanceProgramCounter();
		}
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();
	}

	PrintDisplay(chip8);
	CheckDisplay(
		chip8, "f2100000000000009a0000000000000096000000000000009a00000000000000f21000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			   "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
}

template<typename EmuT>
static void CheckAreEqual(const EmuT& lhs, const EmuT& rhs)
{
	// check cpu
	ASSERT_EQ(lhs._cpu._programCounter, rhs._cpu._programCounter);
	ASSERT_EQ(lhs._cpu._indexRegister, rhs._cpu._indexRegister);
	ASSERT_EQ(lhs._cpu._stackPointer, rhs._cpu._stackPointer);
	for (size_t i = 0; i < lhs._cpu._stack.size(); ++i)
		ASSERT_EQ(lhs._cpu._stack[i], rhs._cpu._stack[i]);
	for (size_t i = 0; i < lhs._cpu._registers.size(); ++i)
		ASSERT_EQ(lhs._cpu._registers[i], rhs._cpu._registers[i]);
	ASSERT_EQ(lhs._cpu._delayTimer, rhs._cpu._delayTimer);
	ASSERT_EQ(lhs._cpu._soundTimer, rhs._cpu._soundTimer);

	// check ram
	for (size_t i = 0; i < lhs._ram.GetSize(); ++i)
		ASSERT_EQ(lhs._ram.GetAt(i), rhs._ram.GetAt(i));

	// check display
	for (size_t i = 0; i < lhs._display.GetWidth() * lhs._display.GetHeight(); ++i)
		ASSERT_EQ(lhs._display.GetAt(i), rhs._display.GetAt(i));

	// check keypad
	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
	{
		auto keyCode = static_cast<emu::Keys::Enum>(key);
		ASSERT_EQ(lhs._keypad.IsPressed(keyCode), rhs._keypad.IsPressed(keyCode));
	}
}

TEST_F(Chip8Tests, Serialize)
{
	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_ram;
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_display;
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_keypad;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/test_opcode.ch8"));

	for (size_t i = 0; i < 50; ++i)
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();

	std::vector<emu::Byte> byteArray;
	chip8.Serialize(byteArray);

	Chip8 deserializedChip8;
	deserializedChip8.Deserialize(byteArray);

	CheckAreEqual(deserializedChip8, chip8);
}

TEST_F(Chip8Tests, SerializeToFile)
{
	struct TemporaryFile
	{
		std::filesystem::path path;
		~TemporaryFile() { std::filesystem::remove(path); }
	};

	spdlog::set_level(spdlog::level::off);
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	struct Chip8: public emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>
	{
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_cpu;
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_ram;
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_display;
		using emu::detail::Chip8<TestCpu, emu::Rng, emu::Ram, emu::Display, emu::Keypad>::_keypad;
	};
	Chip8 chip8;

	auto* dataPath = std::getenv("DATA_PATH");
	ASSERT_NE(dataPath, nullptr);
	ASSERT_TRUE(chip8.LoadRom(std::string(dataPath) + "/test_opcode.ch8"));

	for (size_t i = 0; i < 50; ++i)
		ASSERT_TRUE(chip8.Cycle()) << chip8.GetLastError();

	std::vector<emu::Byte> writeBytes;
	chip8.Serialize(writeBytes);

	TemporaryFile f { "tmp.sav" };

	{
		std::ofstream writeBinaryFile(f.path.string(), std::ios::binary);
		writeBinaryFile.write(reinterpret_cast<const char*>(&writeBytes[0]),
							  static_cast<long>(writeBytes.size() * sizeof(emu::Byte)));
	}

	std::ifstream readBinaryFile(f.path.string(), std::ios::binary | std::ios::ate);

	const auto fileSize = static_cast<size_t>(readBinaryFile.tellg());
	ASSERT_EQ(fileSize, writeBytes.size());

	std::vector<emu::Byte> readBytes(fileSize);
	readBinaryFile.seekg(readBinaryFile.beg);
	readBinaryFile.read(reinterpret_cast<char*>(readBytes.data()), static_cast<long>(fileSize));

	ASSERT_EQ(writeBytes.size(), readBytes.size());
	for (size_t i = 0; i < writeBytes.size(); ++i)
		EXPECT_EQ(writeBytes[i], readBytes[i]) << i;

	Chip8 deserializedChip8;
	deserializedChip8.Deserialize(readBytes);

	CheckAreEqual(deserializedChip8, chip8);
}

TEST_F(Chip8Tests, PrintError)
{
	for (size_t err = emu::Error::START; err <= emu::Error::END; ++err)
	{
		if (err < emu::Error::END)
			ASSERT_NE(emu::ToString(static_cast<emu::Error::Enum>(err)), "?");
		else
			ASSERT_EQ(emu::ToString(static_cast<emu::Error::Enum>(err)), "?");
	}
}

TEST_F(Chip8Tests, PrintInstruction)
{
	for (size_t instr = emu::Instruction::START; instr <= emu::Instruction::END; ++instr)
	{
		if (instr < emu::Instruction::END)
			ASSERT_NE(emu::ToString(static_cast<emu::Instruction::Enum>(instr)), "INVALID");
		else
			ASSERT_EQ(emu::ToString(static_cast<emu::Instruction::Enum>(instr)), "INVALID");
	}
}

TEST_F(Chip8Tests, CheckTestKeypad)
{
	TestChip8 chip8;
	ASSERT_EQ(chip8._keypad.GetSize(), 0);
	chip8._keypad.Press(emu::Keys::B, true);
	ASSERT_TRUE(chip8._keypad.IsPressed(emu::Keys::B));

	std::vector<emu::Byte> bytes;
	chip8._keypad.Serialize(bytes);
	chip8._keypad.Deserialize(bytes);
}

TEST_F(Chip8Tests, CheckTestRam)
{
	TestChip8 chip8;
	chip8._ram.data.resize(10);
	ASSERT_EQ(chip8._ram.GetSize(), chip8._ram.data.size());
	chip8._ram.Load("");
	ASSERT_EQ(chip8._ram.GetSize(), chip8._ram.data.size());

	std::vector<emu::Byte> bytes;
	chip8._ram.Serialize(bytes);
	chip8._ram.Deserialize(bytes);
}

TEST_F(Chip8Tests, CheckTestDisplay)
{
	TestChip8 chip8;
	ASSERT_FALSE(chip8._display.HasChanged());
	chip8._display.Reset();

	std::vector<emu::Byte> bytes;
	chip8._display.Serialize(bytes);
	chip8._display.Deserialize(bytes);
}
