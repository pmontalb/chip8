
#include "Emulator/Cpu.h"
#include "Emulator/Interfaces/IDisplay.h"
#include "Emulator/Interfaces/IKeypad.h"
#include "Emulator/Interfaces/IRam.h"
#include "Emulator/Interfaces/IRng.h"

#include "Emulator/Logging.h"

#include <bitset>
#include <gtest/gtest.h>

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
	[[nodiscard]] std::size_t GetInstructionStartAddress() const override { return 0; }

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
	TestDisplay(const Width width, const Height height) : _width(width), _height(height) { Clear(); }

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

class CpuTests: public ::testing::Test
{
};

TEST_F(CpuTests, StackSizeIs16)
{
	TestCpu cpu;
	ASSERT_EQ(cpu._stack.size(), 16);
}
TEST_F(CpuTests, RegisterSizeIs16)
{
	TestCpu cpu;
	ASSERT_EQ(cpu._stack.size(), 16);
}
TEST_F(CpuTests, InitializationValues)
{
	TestCpu cpu;
	ASSERT_EQ(cpu._programCounter, 0);
	ASSERT_EQ(cpu._indexRegister, 0);
	ASSERT_EQ(cpu._stackPointer, 0);
	ASSERT_TRUE(std::all_of(cpu._stack.begin(), cpu._stack.end(), [](const auto x) { return x == 0; }));
	ASSERT_TRUE(std::all_of(cpu._registers.begin(), cpu._registers.end(), [](const auto x) { return x == 0; }));
	ASSERT_EQ(cpu._delayTimer, 0);
	ASSERT_EQ(cpu._soundTimer, 0);
}

TEST_F(CpuTests, GetProgramCounter)
{
	TestCpu cpu;
	cpu._programCounter = 42;
	ASSERT_EQ(cpu.GetProgramCounter(), 42);
}
TEST_F(CpuTests, SetProgramCounter)
{
	TestCpu cpu;
	cpu.SetProgramCounter(512);
	ASSERT_EQ(cpu.GetProgramCounter(), 512);
}
TEST_F(CpuTests, AdvanceProgramCounter)
{
	TestCpu cpu;
	cpu.SetProgramCounter(512);
	cpu.AdvanceProgramCounter();
	ASSERT_EQ(cpu.GetProgramCounter(), 514);

#ifndef NDEBUG
	cpu._programCounter = std::numeric_limits<emu::TwoBytes>::max() - 1;

	__START_IGNORING_WARNINGS__
	#ifdef __clang__
	__IGNORE_WARNING__("-Wused-but-marked-unused")
	#endif
	ASSERT_DEATH({ cpu.AdvanceProgramCounter(); }, "");
	__STOP_IGNORING_WARNINGS__
#endif
}
TEST_F(CpuTests, RetreatProgramCounter)
{
	TestCpu cpu;
	cpu.SetProgramCounter(512);
	cpu.RetreatProgramCounter();
	ASSERT_EQ(cpu.GetProgramCounter(), 510);

#ifndef NDEBUG
	cpu._programCounter = 1;
	__START_IGNORING_WARNINGS__
	#ifdef __clang__
	__IGNORE_WARNING__("-Wused-but-marked-unused")
	#endif
	ASSERT_DEATH({ cpu.RetreatProgramCounter(); }, "");
	__STOP_IGNORING_WARNINGS__
#endif
}

TEST_F(CpuTests, ReturnFromSubRoutine)
{
	TestCpu cpu;
#ifndef NDEBUG
	__START_IGNORING_WARNINGS__
	#ifdef __clang__
	__IGNORE_WARNING__("-Wused-but-marked-unused")
	#endif
	ASSERT_DEATH({ cpu.ReturnFromSubRoutine(); }, "");
	__STOP_IGNORING_WARNINGS__
#endif

	cpu._stackPointer = 4;
	cpu._stack[cpu._stackPointer - 1] = 42;
	cpu.ReturnFromSubRoutine();

	ASSERT_EQ(cpu.GetProgramCounter(), 42);
}

TEST_F(CpuTests, JumpToAddress)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xFD4E;
	cpu.JumpToAddress(instruction);
	ASSERT_EQ(cpu.GetProgramCounter(), 0xD4E);
}

TEST_F(CpuTests, CallSubRoutine)
{
	TestCpu cpu;
	cpu._stackPointer = 7;
	cpu._stack[cpu._stackPointer] = 42;
	cpu._programCounter = 24;

	emu::TwoBytes instruction = 0xFD4E;
	cpu.CallSubRoutine(instruction);

	ASSERT_EQ(cpu._stackPointer, 8);
	ASSERT_EQ(cpu._stack[cpu._stackPointer - 1], 24);
	ASSERT_EQ(cpu.GetProgramCounter(), 0xD4E);
}

TEST_F(CpuTests, ConditionalSkipIfByteEqual)
{
	TestCpu cpu;
	cpu._programCounter = 42;

	cpu._registers[0xC] = 0xBF;
	emu::TwoBytes instruction = 0xFCAF;
	cpu.ConditionalSkipIfByteEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 42);

	instruction = 0xACBF;
	cpu.ConditionalSkipIfByteEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 44);
}
TEST_F(CpuTests, ConditionalSkipIfByteNotEqual)
{
	TestCpu cpu;
	cpu._programCounter = 42;

	cpu._registers[0xC] = 0xBF;
	emu::TwoBytes instruction = 0xFCAF;
	cpu.ConditionalSkipIfByteNotEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 44);

	instruction = 0xACBF;
	cpu.ConditionalSkipIfByteNotEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 44);
}
TEST_F(CpuTests, ConditionalSkipIfRegistersEqual)
{
	TestCpu cpu;
	cpu._programCounter = 42;

	cpu._registers[0xC] = 0xBF;
	cpu._registers[0xA] = 0xCF;
	emu::TwoBytes instruction = 0xFCAF;
	cpu.ConditionalSkipIfRegistersEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 42);

	cpu.ConditionalSkipIfRegistersNotEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 44);

	cpu._registers[0xC] = 0xBF;
	cpu._registers[0xA] = 0xBF;
	cpu.ConditionalSkipIfRegistersEqual(instruction);
	ASSERT_EQ(cpu.GetProgramCounter(), 46);
}
TEST_F(CpuTests, ConditionalSkipIfRegistersNotEqual)
{
	TestCpu cpu;
	cpu._programCounter = 42;

	cpu._registers[0xC] = 0xBF;
	cpu._registers[0xA] = 0xCF;
	emu::TwoBytes instruction = 0xFCAF;
	cpu.ConditionalSkipIfRegistersNotEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 44);

	cpu.ConditionalSkipIfRegistersEqual(instruction);

	ASSERT_EQ(cpu.GetProgramCounter(), 44);

	cpu._registers[0xC] = 0xBF;
	cpu._registers[0xA] = 0xBF;
	cpu.ConditionalSkipIfRegistersNotEqual(instruction);
	ASSERT_EQ(cpu.GetProgramCounter(), 44);
}
TEST_F(CpuTests, ConditionalSkipIfKeyPressed)
{
	TestCpu cpu;
	cpu._programCounter = 42;
	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0x9] = emu::Keys::Seven;

	TestKeyPad testKeyPad;
	testKeyPad.toggle[emu::Keys::Seven] = true;
	cpu.ConditionalSkipIfKeyPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 44);

	testKeyPad.toggle[emu::Keys::Seven] = false;
	cpu.ConditionalSkipIfKeyPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 44);

	testKeyPad.toggle[emu::Keys::Seven] = true;
	testKeyPad.toggle.flip();
	cpu.ConditionalSkipIfKeyPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 44);
}
TEST_F(CpuTests, ConditionalSkipIfKeyNotPressed)
{
	TestCpu cpu;
	cpu._programCounter = 42;
	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0x9] = emu::Keys::Seven;

	TestKeyPad testKeyPad;
	testKeyPad.toggle[emu::Keys::Seven] = true;
	cpu.ConditionalSkipIfKeyNotPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 42);

	testKeyPad.toggle[emu::Keys::Seven] = false;
	cpu.ConditionalSkipIfKeyNotPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 44);

	testKeyPad.toggle[emu::Keys::Seven] = true;
	testKeyPad.toggle.flip();
	cpu.ConditionalSkipIfKeyNotPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 46);
}

TEST_F(CpuTests, LoadByte)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadByte(instruction);

	ASSERT_EQ(cpu._registers[0x9], 0xAE);
}
TEST_F(CpuTests, LoadRegister)
{
	TestCpu cpu;
	cpu._registers[0xA] = 0x77;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadRegister(instruction);

	ASSERT_EQ(cpu._registers[0x9], 0x77);
}
TEST_F(CpuTests, LoadDelayTimer)
{
	TestCpu cpu;
	cpu._delayTimer = 42;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadDelayTimer(instruction);

	ASSERT_EQ(cpu._registers[0x9], 42);
}
TEST_F(CpuTests, LoadFontIntoIndexRegister)
{
	TestRam ram;
	ram.fonts.resize(0xFF, 0x7C);

	ram.fonts[0xFA] = 0x42;
	TestCpu cpu;
	cpu._registers[0x9] = 0xFA;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadFontIntoIndexRegister(instruction, ram);

	ASSERT_EQ(cpu._indexRegister, 0x42);
}
TEST_F(CpuTests, LoadRegistersFromRam)
{
	TestRam ram;
	for (size_t i = 0; i < 128; ++i)
		ram.data.push_back(static_cast<emu::Byte>(i));

	TestCpu cpu;
	cpu._indexRegister = static_cast<emu::TwoBytes>(ram.data.size() / 3);
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadRegistersFromRam(instruction, ram);

	for (size_t i = 0; i <= 0x9; ++i)
	{
		ASSERT_EQ(cpu._registers[i], ram.data[cpu._indexRegister + i]);
	}
	for (size_t i = 0x9 + 1; i < cpu._registers.size(); ++i)
		ASSERT_EQ(cpu._registers[i], 0);
}

TEST_F(CpuTests, SetDelayTimer)
{
	TestCpu cpu;

	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0x9] = 0x42;
	cpu.SetDelayTimer(instruction);

	ASSERT_EQ(cpu._delayTimer, 0x42);
}
TEST_F(CpuTests, SetSoundTimer)
{
	TestCpu cpu;

	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0x9] = 0x42;
	cpu.SetSoundTimer(instruction);

	ASSERT_EQ(cpu._soundTimer, 0x42);
}

TEST_F(CpuTests, StoreBinaryCodeRepresentation)
{
	TestRam ram;
	ram.data.resize(0xFFFF);

	TestCpu cpu;
	cpu._indexRegister = 7;

	emu::TwoBytes instruction = 0xD9AE;

	// 0x7F = 127
	cpu._registers[0x9] = 0x7F;
	cpu.StoreBinaryCodeRepresentation(instruction, ram);

	for (size_t i = cpu._indexRegister + 3; i < ram.data.size(); ++i)
		ASSERT_EQ(ram.data[i], 0);
	ASSERT_EQ(ram.data[cpu._indexRegister + 2], 7);
	ASSERT_EQ(ram.data[cpu._indexRegister + 1], 2);
	ASSERT_EQ(ram.data[cpu._indexRegister + 0], 1);
	for (size_t i = 0; i < cpu._indexRegister; ++i)
		ASSERT_EQ(ram.data[i], 0);
}

TEST_F(CpuTests, StoreRegistersInRam)
{
	TestRam ram;
	ram.data.resize(64);
	for (size_t i = 0; i < ram.data.size(); ++i)
		ram.data[i] = 42;

	TestCpu cpu;
	cpu._indexRegister = 7;

	for (size_t i = 0; i < cpu._registers.size(); ++i)
		cpu._registers[i] = static_cast<emu::Byte>(i * i);

	emu::TwoBytes instruction = 0xDCAE;

	cpu.StoreRegistersInRam(instruction, ram);

	for (size_t i = 0; i <= 0xC; ++i)
		ASSERT_EQ(cpu._registers[i], ram.data[cpu._indexRegister + i]);
	for (size_t i = 0xC + 1; i < ram.data.size() - cpu._indexRegister; ++i)
		ASSERT_EQ(ram.data[cpu._indexRegister + i], 42);
}

TEST_F(CpuTests, AddEqualByte)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 42;
	cpu.AddEqualByte(instruction);
	ASSERT_EQ(cpu._registers[0x9], 42 + 0xAE);
}
TEST_F(CpuTests, IndexRegisterAddEqualRegister)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 42;
	cpu._indexRegister = 64;
	cpu.IndexRegisterAddEqualRegister(instruction);
	ASSERT_EQ(cpu._indexRegister, 42 + 64);
}
TEST_F(CpuTests, OrEqualRegister)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0x0A;
	cpu._registers[0xA] = 0xA0;
	cpu.OrEqualRegister(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0xAA);

	cpu._registers[0x9] = 0xAA;
	cpu._registers[0xA] = 0xA0;
	cpu.OrEqualRegister(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0xAA);
}
TEST_F(CpuTests, AndEqualRegister)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0xAA;
	cpu._registers[0xA] = 0xA0;
	cpu.AndEqualRegister(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0xA0);
}
TEST_F(CpuTests, XorEqualRegister)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0x0A;
	cpu._registers[0xA] = 0xA0;
	cpu.XorEqualRegister(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0xAA);

	cpu._registers[0x9] = 0xAA;
	cpu._registers[0xA] = 0xA0;
	cpu.XorEqualRegister(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x0A);
}
TEST_F(CpuTests, AddRegistersAndStoreLastByte)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0x1;
	cpu._registers[0xA] = 0x2;
	cpu.AddRegistersAndStoreLastByte(instruction);
	ASSERT_EQ(cpu._registers.back(), 0);
	ASSERT_EQ(cpu._registers[0x9], 0x1 + 0x2);

	cpu._registers[0x9] = 0xFA;
	cpu._registers[0xA] = 0xFF;
	cpu.AddRegistersAndStoreLastByte(instruction);

	// 0xFA + 0xFF = 0x1F9
	ASSERT_EQ(cpu._registers.back(), 1);
	ASSERT_EQ(cpu._registers[0x9], 0xF9);
}

TEST_F(CpuTests, SubtractEqualRegisters)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0xA0;
	cpu._registers[0xA] = 0x0A;
	cpu.SubtractEqualRegisters(instruction);

	// we store the not borrow here
	ASSERT_EQ(cpu._registers.back(), 1);
	ASSERT_EQ(cpu._registers[0x9], 0x96);

	cpu._registers[0x9] = 0x0A;
	cpu._registers[0xA] = 0xA0;
	cpu.SubtractEqualRegisters(instruction);
	ASSERT_EQ(cpu._registers.back(), 0);

	// 0x0A - 0xA0 = -0x96 = 0x100 - 0x96 = 0x6A
	ASSERT_EQ(cpu._registers[0x9], 0x6A);
}
TEST_F(CpuTests, OppositeSubtractRegisters)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0xA0;
	cpu._registers[0xA] = 0x0A;
	cpu.OppositeSubtractRegisters(instruction);

	// 0x0A - 0xA0 = -0x96 = 0x100 - 0x96 = 0x6A
	ASSERT_EQ(cpu._registers.back(), 0);
	ASSERT_EQ(cpu._registers[0x9], 0x6A);

	cpu._registers[0x9] = 0x0A;
	cpu._registers[0xA] = 0xA0;
	cpu.OppositeSubtractRegisters(instruction);
	// we store the not borrow here
	ASSERT_EQ(cpu._registers.back(), 1);
	ASSERT_EQ(cpu._registers[0x9], 0x96);
}

TEST_F(CpuTests, ShiftRightAndStoreLastBit)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0xA1;
	cpu.ShiftRightAndStoreLastBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x50);
	ASSERT_EQ(cpu._registers.back(), 1);

	cpu._registers[0x9] = 0x0C;
	cpu.ShiftRightAndStoreLastBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x06);
	ASSERT_EQ(cpu._registers.back(), 0);

	cpu._registers[0x9] = 0x0B;
	cpu.ShiftRightAndStoreLastBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x05);
	ASSERT_EQ(cpu._registers.back(), 1);

	cpu._registers[0x9] = 0x1B;
	cpu.ShiftRightAndStoreLastBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x0D);
	ASSERT_EQ(cpu._registers.back(), 1);

	cpu._registers[0x9] = 0x1;
	cpu.ShiftRightAndStoreLastBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x00);
	ASSERT_EQ(cpu._registers.back(), 1);

	cpu._registers[0x9] = 0x0;
	cpu.ShiftRightAndStoreLastBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x00);
	ASSERT_EQ(cpu._registers.back(), 0);
}
TEST_F(CpuTests, ShiftLeftAndStoreFirstBit)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;

	cpu._registers[0x9] = 0x0A;
	cpu.ShiftLeftAndStoreFirstBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x14);
	ASSERT_EQ(cpu._registers.back(), 0);

	cpu._registers[0x9] = 0x1;
	cpu.ShiftLeftAndStoreFirstBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x02);
	ASSERT_EQ(cpu._registers.back(), 0);

	cpu._registers[0x9] = 0x0;
	cpu.ShiftLeftAndStoreFirstBit(instruction);
	ASSERT_EQ(cpu._registers[0x9], 0x00);
	ASSERT_EQ(cpu._registers.back(), 0);
}

TEST_F(CpuTests, SetIndexRegister)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;
	cpu._indexRegister = 7;
	cpu.SetIndexRegister(instruction);
	ASSERT_EQ(cpu._indexRegister, 0x9AE);
}

TEST_F(CpuTests, JumpToLastTwelveBitsPlusFirstRegister)
{
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0] = 0x70;
	cpu.JumpToLastTwelveBitsPlusFirstRegister(instruction);
	ASSERT_EQ(cpu.GetProgramCounter(), cpu._registers[0] + 0x9AE);
}

TEST_F(CpuTests, RandomAndEqualByte)
{
	TestRng rng;
	rng.totallyRandomNumber = 0xA0;
	TestCpu cpu;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.RandomAndEqualByte(instruction, rng);

	ASSERT_EQ(cpu._registers[0x9], 0xA0);
}

TEST_F(CpuTests, Draw)
{
	TestRam ram;

	TestCpu cpu;
	cpu._indexRegister = 7;

	TestDisplay display(8, 8);

	ram.data.resize(32);

	// draw a single 4-byte sprites...
	constexpr size_t spriteBytes = 0x4;
	ram.data[cpu._indexRegister + 0] = 0xA4;	// 0b10100100
	ram.data[cpu._indexRegister + 1] = 0x03;	// 0b00000011
	ram.data[cpu._indexRegister + 2] = 0x7A;	// 0b01111010
	ram.data[cpu._indexRegister + 3] = 0xA4;	// 0b00000001

	// ... starting at (row,col) = (2,3)
	constexpr auto row = 2;
	constexpr auto col = 3;

	constexpr emu::Byte Vx = 0x9;
	constexpr emu::Byte Vy = 0xA;
	constexpr emu::TwoBytes instruction = (Vx << 8u) | (Vy << 4u) | spriteBytes;
	static_assert(utils::LowerFourBitsHighByte(instruction) == Vx);
	static_assert(utils::UpperFourBitsLowByte(instruction) == Vy);
	static_assert(utils::LowestFourBits(instruction) == spriteBytes);
	cpu._registers[Vx] = row;
	cpu._registers[Vy] = col;

	// Vx=0x9
	// Vy=0xA
	// regVx=2
	// regVy=3
	// n = 4
	// this will iterate on the first n=4 rows and set
	// the display at (x,y) [which is at coordinate (regVx + nBit) + (regVy + row) * width],
	// where nBit=0,...7. If the screen is already on, it detects a collision and stores it in the last register.
	// we skip 0 bits (ie transparent pixels)
	// the way 1 bit on pixels are drawn is by mean of xor-ing, which is flipping the display state (flag^1==!flag)
	cpu.Draw(instruction, display, ram);

	// no collisions, as the display was off
	ASSERT_EQ(cpu._registers.back(), 0);

	const auto getCoord = [&](const auto x, const auto y)
	{ return (static_cast<size_t>(cpu._registers[Vy] + x)) * display.GetWidth() + (cpu._registers[Vx] + y); };
	const auto reverseBits = [](auto& b)
	{
		auto reversed = std::decay_t<decltype(b)> {};
		for (size_t i = 0; i < b.size(); ++i)
			reversed[i] = b[b.size() - 1 - i];
		return reversed;
	};
	for (size_t i = 0; i < spriteBytes; ++i)
	{
		std::bitset<8> displayRow;
		for (size_t j = 0; j < displayRow.size(); ++j)
			displayRow[j] = display.GetAt(getCoord(0, j));

		ASSERT_EQ(reverseBits(displayRow).to_ulong(), ram.data[cpu._indexRegister + 0]);
	}

	// "redraw" the same sprite will flip the display off
	cpu.Draw(instruction, display, ram);
	for (size_t i = 0; i < display.GetPixels().size(); ++i)
		ASSERT_FALSE(display.GetPixels()[i]);
	ASSERT_EQ(cpu._registers.back(), 1);

	// finally, try to redraw the same sprite, this should show the original image
	cpu.Draw(instruction, display, ram);
	ASSERT_EQ(cpu._registers.back(), 0);
	for (size_t i = 0; i < spriteBytes; ++i)
	{
		std::bitset<8> displayRow;
		for (size_t j = 0; j < displayRow.size(); ++j)
			displayRow[j] = display.GetAt(getCoord(0, j));

		ASSERT_EQ(reverseBits(displayRow).to_ulong(), ram.data[cpu._indexRegister + 0]);
	}
}

TEST_F(CpuTests, WaitUntilKeyIsPressed)
{
	TestCpu cpu;
	cpu._programCounter = 42;

	TestKeyPad testKeyPad;
	testKeyPad.toggle.reset();

	emu::TwoBytes instruction = 0xD9AE;
	cpu.WaitUntilKeyIsPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 40);	   // still waiting

	testKeyPad.toggle[emu::Keys::Seven] = true;
	cpu.WaitUntilKeyIsPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 40);	   // success
	cpu._registers[0x9] = emu::Keys::Seven;

	// take first one
	testKeyPad.toggle.reset();
	testKeyPad.toggle[emu::Keys::Seven] = true;
	testKeyPad.toggle[emu::Keys::Eight] = true;
	cpu.WaitUntilKeyIsPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 40);	   // success
	cpu._registers[0x9] = std::min(emu::Keys::Seven, emu::Keys::Eight);

	// all pressed (same as before
	testKeyPad.toggle.set();
	cpu.WaitUntilKeyIsPressed(instruction, testKeyPad);
	ASSERT_EQ(cpu._programCounter, 40);	   // success
	cpu._registers[0x9] = emu::Keys::START;
}

TEST_F(CpuTests, DecrementTimers)
{
	TestCpu cpu;
	cpu._soundTimer = 42;
	cpu._delayTimer = 24;
	cpu.DecrementTimers();
	ASSERT_EQ(cpu._soundTimer, 41);
	ASSERT_EQ(cpu._delayTimer, 23);

	cpu._soundTimer = 0;
	cpu._delayTimer = 24;
	cpu.DecrementTimers();
	ASSERT_EQ(cpu._soundTimer, 0);
	ASSERT_EQ(cpu._delayTimer, 23);

	cpu._soundTimer = 42;
	cpu._delayTimer = 0;
	cpu.DecrementTimers();
	ASSERT_EQ(cpu._soundTimer, 41);
	ASSERT_EQ(cpu._delayTimer, 0);

	cpu._soundTimer = 0;
	cpu._delayTimer = 0;
	cpu.DecrementTimers();
	ASSERT_EQ(cpu._soundTimer, 0);
	ASSERT_EQ(cpu._delayTimer, 0);
}
