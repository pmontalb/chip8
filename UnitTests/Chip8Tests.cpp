
#include "Emulator/Chip8.h"
#include <gtest/gtest.h>

#include "Emulator/Logging.h"

struct TestKeyPad final: public emu::IKeypad, public emu::ISerializable
{
	std::bitset<emu::Keys::END> toggle {};

	[[nodiscard]] bool IsPressed(const emu::Keys::Enum code) const override { return toggle[code]; }
	[[nodiscard]] emu::Byte GetSize() const override { return 0; }
	void Press(const emu::Keys::Enum code, const bool toggle_) override { toggle[code] = toggle_; }

	void Serialize(std::vector<emu::Byte>&) const override { assert(false); }
	utils::Span<emu::Byte> Deserialize(const utils::Span<emu::Byte>&) override
	{
		assert(false);
		return utils::Span<emu::Byte> {};
	}
};

struct TestRam final: public emu::IRam, public emu::ISerializable
{
	std::vector<emu::Byte> data {};
	std::vector<emu::Byte> fonts {};

	[[nodiscard]] std::size_t GetSize() const override { return data.size(); }
	[[nodiscard]] std::size_t GetInstructionStartAddress() const override { return 0; }

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

	void Serialize(std::vector<emu::Byte>&) const override { assert(false); }
	utils::Span<emu::Byte> Deserialize(const utils::Span<emu::Byte>&) override
	{
		assert(false);
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

	void Serialize(std::vector<emu::Byte>&) const override { assert(false); }
	utils::Span<emu::Byte> Deserialize(const utils::Span<emu::Byte>&) override
	{
		assert(false);
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
	ASSERT_EQ(chip8._cpu.GetProgramCounter(), chip8._ram.GetInstructionStartAddress());
	ASSERT_TRUE(std::all_of(chip8._display.GetPixels().begin(), chip8._display.GetPixels().end(),
							[](auto pixel) { return !pixel; }));
}

TEST_F(Chip8Tests, ExecuteUniquePatternInstructionSets)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x1a24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x1nnn);
	ASSERT_EQ(chip8._cpu.GetProgramCounter(), 0xa24);

	chip8._cpu._stackPointer = 0;
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

	chip8._cpu._indexRegister = 0;
	chip8._ram.data.resize(16);
	ASSERT_TRUE(ExecuteInstruction(chip8, 0xda24));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xDxyn);
}

TEST_F(Chip8Tests, ExecuteInstruction0x00E)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0x00E0));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x00E0);

	chip8._cpu._stackPointer = 1;
	ASSERT_TRUE(ExecuteInstruction(chip8, 0x00EE));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0x00EE);
}

TEST_F(Chip8Tests, ExecuteInstruction0x80xy)
{
	TestChip8 chip8;
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

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
	chip8._cpu._programCounter = 0;
	chip8._ram.data.resize(10);
	chip8._ram.data[chip8._cpu._programCounter] = 0xA9;
	chip8._ram.data[chip8._cpu._programCounter + 1] = 0xF2;

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xEAA1));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xExA1);

	ASSERT_TRUE(ExecuteInstruction(chip8, 0xE09E));
	ASSERT_EQ(chip8.GetLastExecutedInstruction(), emu::Instruction::_0xEx9E);
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

	if (HasFailure())
		return;

	Chip8 deserializedChip8;
	deserializedChip8.Deserialize(readBytes);

	CheckAreEqual(deserializedChip8, chip8);
}
