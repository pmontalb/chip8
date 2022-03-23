
#include "Emulator/Cpu.h"
#include "Emulator/Interfaces/IKeypad.h"
#include "Emulator/Interfaces/IRam.h"
#include "Emulator/Interfaces/IRng.h"
#include "Emulator/Interfaces/IDisplay.h"

#include <gtest/gtest.h>
#include <bitset>

struct TestKeyPad: public emu::IKeypad
{
	std::bitset<emu::Keys::END> toggle {};

	[[nodiscard]] bool IsPressed(const emu::Keys::Enum code) const override { return toggle[code]; }
	[[nodiscard]] virtual emu::Byte GetSize() const override { return 0; }
	void Press(const emu::Keys::Enum code, const bool toggle_) override { toggle[code] = toggle_;}
};

struct TestRam: public emu::IRam
{
	std::vector<emu::Byte> data;
	std::vector<emu::Byte> fonts;

	[[nodiscard]] std::size_t GetSize() const override { return data.size(); }

	void Load(const std::string&) override {}

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

struct Cpu: public emu::Cpu
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
	Cpu cpu;
	ASSERT_EQ(cpu._stack.size(), 16);
}
TEST_F(CpuTests, RegisterSizeIs16)
{
	Cpu cpu;
	ASSERT_EQ(cpu._stack.size(), 16);
}
TEST_F(CpuTests, InitializationValues)
{
	Cpu cpu;
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
	Cpu cpu;
	cpu._programCounter = 42;
	ASSERT_EQ(cpu.GetProgramCounter(), 42);
}
TEST_F(CpuTests, SetProgramCounter)
{
	Cpu cpu;
	cpu.SetProgramCounter(512);
	ASSERT_EQ(cpu.GetProgramCounter(), 512);
}
TEST_F(CpuTests, AdvanceProgramCounter)
{
	Cpu cpu;
	cpu.SetProgramCounter(512);
	cpu.AdvanceProgramCounter();
	ASSERT_EQ(cpu.GetProgramCounter(), 514);

#ifndef NDEBUG
	cpu._programCounter = std::numeric_limits<emu::TwoBytes>::max() - 1;
	ASSERT_DEATH({ cpu.AdvanceProgramCounter(); }, "");
#endif
}
TEST_F(CpuTests, RetreatProgramCounter)
{
	Cpu cpu;
	cpu.SetProgramCounter(512);
	cpu.RetreatProgramCounter();
	ASSERT_EQ(cpu.GetProgramCounter(), 510);

#ifndef NDEBUG
	cpu._programCounter = 1;
	ASSERT_DEATH({ cpu.RetreatProgramCounter(); }, "");
#endif
}

TEST_F(CpuTests, ReturnFromSubRoutine)
{
	Cpu cpu;
#ifndef NDEBUG
	ASSERT_DEATH({ cpu.ReturnFromSubRoutine(); }, "");
#endif

	cpu._stackPointer = 4;
	cpu._stack[cpu._stackPointer - 1] = 42;
	cpu.ReturnFromSubRoutine();

	ASSERT_EQ(cpu.GetProgramCounter(), 42);
}

TEST_F(CpuTests, JumpToAddress)
{
	Cpu cpu;
	emu::TwoBytes instruction = 0xFD4E;
	cpu.JumpToAddress(instruction);
	ASSERT_EQ(cpu.GetProgramCounter(), 0xD4E);
}

TEST_F(CpuTests, CallSubRoutine)
{
	Cpu cpu;
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
	Cpu cpu;
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
	Cpu cpu;
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
	Cpu cpu;
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
	Cpu cpu;
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
	Cpu cpu;
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
	Cpu cpu;
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
	Cpu cpu;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadByte(instruction);

	ASSERT_EQ(cpu._registers[0x9], 0xAE);
}
TEST_F(CpuTests, LoadRegister)
{
	Cpu cpu;
	cpu._registers[0xA] = 0x77;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadRegister(instruction);

	ASSERT_EQ(cpu._registers[0x9], 0x77);
}
TEST_F(CpuTests, LoadDelayTimer)
{
	Cpu cpu;
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
	Cpu cpu;
	cpu._registers[0x9] = 0xFA;
	emu::TwoBytes instruction = 0xD9AE;
	cpu.LoadFontIntoIndexRegister(instruction, ram);

	ASSERT_EQ(cpu._indexRegister, 0x42);
}
TEST_F(CpuTests, LoadRegistersFromRam)
{
	TestRam ram;
	for (size_t i = 0; i < 128; ++i)
		ram.data.push_back(i);

	Cpu cpu;
	cpu._indexRegister = ram.data.size() / 3;
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
	Cpu cpu;

	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0x9] = 0x42;
	cpu.SetDelayTimer(instruction);

	ASSERT_EQ(cpu._delayTimer, 0x42);
}
TEST_F(CpuTests, SetSoundTimer)
{
	Cpu cpu;

	emu::TwoBytes instruction = 0xD9AE;
	cpu._registers[0x9] = 0x42;
	cpu.SetSoundTimer(instruction);

	ASSERT_EQ(cpu._soundTimer, 0x42);
}

TEST_F(CpuTests, StoreBinaryCodeRepresentation)
{
	TestRam ram;
	ram.data.resize(0xFFFF);

	Cpu cpu;
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
