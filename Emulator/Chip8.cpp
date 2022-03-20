
#include "Chip8.h"

#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>

namespace emu
{
	Chip8::Chip8()
	{
		cpu.SetProgramCounter(Ram::instructionStart);
		SPDLOG_TRACE("Chip8 created and pc={}", cpu.GetProgramCounter());
	}

	bool Chip8::LoadRom(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			SPDLOG_CRITICAL("path({}) doesn't exist", path.string());
			return false;
		}
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			SPDLOG_CRITICAL("file({}) couldn't be opened", path.string());
			return false;
		}

		// Get size of file and allocate a buffer to hold the contents
		std::string buffer;
		buffer.resize(file.tellg());
		SPDLOG_TRACE("buffer size({})", buffer.size());

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), buffer.size());

		ram.Load(buffer);

		return true;
	}

	bool Chip8::Cycle()
	{
		const auto instruction = FetchInstruction();
		SPDLOG_TRACE("fetched instruction({0:d}|{0:X})", instruction);

		// pc += 2
		cpu.AdvanceProgramCounter();

		// now execute the instruction
		if (!ExecuteInstruction(instruction))
			return false;

		cpu.DecrementTimers();

		return true;
	}

	TwoBytes Chip8::FetchInstruction()
	{
		const auto pc = cpu.GetProgramCounter();
		const auto memByte = ram.GetAt(pc);
		const auto nextMemByte = ram.GetAt(pc + 1);

		// the upper bits come from memByte the lowest bit from nextMemByte
		static constexpr TwoBytes byteShift = 8;
		const TwoBytes topPart = memByte << byteShift;

		SPDLOG_DEBUG("pc({0}) ram=({1:d}|{1:X}) nextRam=({2:d}|{2:X}) instruction({3:d}|{3:X})",
					 pc, memByte, nextMemByte, topPart | nextMemByte);

		return topPart | nextMemByte;
	}

	bool Chip8::ExecuteInstruction(const TwoBytes instruction)
	{
		/*
		 * If you look at the list of opcodes, you’ll notice that there are four types:
			The entire opcode is unique:
				$1nnn
				$2nnn
				$3xkk
				$4xkk
				$5xy0
				$6xkk
				$7xkk
				$9xy0
				$Annn
				$Bnnn
				$Cxkk
				$Dxyn
			The first digit repeats but the last digit is unique:
				$8xy0
				$8xy1
				$8xy2
				$8xy3
				$8xy4
				$8xy5
				$8xy6
				$8xy7
				$8xyE
			The first three digits are $00E but the fourth digit is unique:
				$00E0
				$00EE
			The first digit repeats but the last two digits are unique:
				$ExA1
				$Ex9E
				$Fx07
				$Fx0A
				$Fx15
				$Fx18
				$Fx1E
				$Fx29
				$Fx33
				$Fx55
				$Fx65
		 * */

		// zero out the first 3 nibbles and shift it down by 12 bits
		const Byte mostSignificantBit = (instruction & 0xF000ul) >> 12ul;
		SPDLOG_TRACE("instruction({0:d}|{0:X}): msb({1:d}|{1:X})", instruction, mostSignificantBit);

		switch (mostSignificantBit)
		{
			// 0x0*** instructions
			case 0x0:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				switch (lowestByte)
				{
					case 0xE0:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xE0", instruction, mostSignificantBit, lowestByte);
						display.Clear();
						break;
					case 0xEE:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xEE", instruction, mostSignificantBit, lowestByte);
						cpu.ReturnFromSubRoutine();
						break;
					default:
						SPDLOG_CRITICAL("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): illegal instruction", instruction, mostSignificantBit, lowestByte);
						return false;
				}
				break;
			}

			// 0x1nnn
			case 0x1:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x1nnn", instruction, mostSignificantBit);
				cpu.JumpToAddress(instruction);
				break;

			// 0x2nnn
			case 0x2:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x2nnn", instruction, mostSignificantBit);
				cpu.CallSubRoutine(instruction);
				break;

			// 0x3xkk
			case 0x3:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x3xkk", instruction, mostSignificantBit);
				cpu.ConditionalSkipIfByteEqual(instruction);
				break;

			// 0x4xkk
			case 0x4:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x4xkk", instruction, mostSignificantBit);
				cpu.ConditionalSkipIfByteNotEqual(instruction);
				break;

			// 0x5xy0
			case 0x5:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x5xy0", instruction, mostSignificantBit);
				cpu.ConditionalSkipIfRegistersEqual(instruction);
				break;

			// 0x6xkk
			case 0x6:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x6xkk", instruction, mostSignificantBit);
				cpu.LoadByte(instruction);
				break;

			// 0x7xkk
			case 0x7:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x7xkk", instruction, mostSignificantBit);
				cpu.AddEqualByte(instruction);
				break;

			// 0x8*** instructions
			case 0x8:
			{
				const Byte lastFourBits = utils::LowestFourBits(instruction);
				switch (lastFourBits)
				{
					case 0x0:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy0", instruction, mostSignificantBit, lastFourBits);
						cpu.LoadRegister(instruction);
						break;
					case 0x1:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy1", instruction, mostSignificantBit, lastFourBits);
						cpu.OrEqualRegister(instruction);
						break;
					case 0x2:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy2", instruction, mostSignificantBit, lastFourBits);
						cpu.AndEqualRegister(instruction);
						break;
					case 0x3:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy3", instruction, mostSignificantBit, lastFourBits);
						cpu.XorEqualRegister(instruction);
						break;
					case 0x4:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy4", instruction, mostSignificantBit, lastFourBits);
						cpu.AddRegistersAndStoreLastByte(instruction);
						break;
					case 0x5:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy5", instruction, mostSignificantBit, lastFourBits);
						cpu.SubtractEqualRegisters(instruction);
						break;
					case 0x6:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy6", instruction, mostSignificantBit, lastFourBits);
						cpu.ShiftRightAndStoreLastBit(instruction);
						break;
					case 0x7:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy7", instruction, mostSignificantBit, lastFourBits);
						cpu.OppositeSubtractRegisters(instruction);
						break;
					case 0xE:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xyE", instruction, mostSignificantBit, lastFourBits);
						cpu.ShiftLeftAndStoreFirstBit(instruction);
						break;
					default:
						SPDLOG_CRITICAL("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): illegal instruction", instruction, mostSignificantBit, lastFourBits);
						return false;
				}
				break;
			}

			// 0x9xy0
			case 0x9:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x9xy0", instruction, mostSignificantBit);
				cpu.ConditionalSkipIfRegistersNotEqual(instruction);
				break;

			// 0xAnnn
			case 0xA:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0xAnnn", instruction, mostSignificantBit);
				cpu.SetIndexRegister(instruction);
				break;

			// 0xBnnn
			case 0xB:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0xBnnn", instruction, mostSignificantBit);
				cpu.JumpToLastTwelveBitsPlusFirstRegister(instruction);
				break;

			// 0xCxkk
			case 0xC:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0xCxkk", instruction, mostSignificantBit);
				cpu.RandomAndEqualByte(instruction, rng);
				break;

			// 0xDxkk
			case 0xD:
				SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0xDxkk", instruction, mostSignificantBit);
				cpu.Draw(instruction, display, ram);
				break;

			// 0xE*** instructions
			case 0xE:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				switch (lowestByte)
				{
					case 0x9E:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xEx9E", instruction, mostSignificantBit, lowestByte);
						cpu.ConditionalSkipIfKeyPressed(instruction, keypad);
						break;
					case 0xA1:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xExA1", instruction, mostSignificantBit, lowestByte);
						cpu.ConditionalSkipIfKeyNotPressed(instruction, keypad);
						break;
					default:
						SPDLOG_CRITICAL("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): illegal instruction", instruction, mostSignificantBit, lowestByte);
						return false;
				}
				break;
			}

			// 0xF*** instructions
			case 0xF:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				switch (lowestByte)
				{
					case 0x07:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx07", instruction, mostSignificantBit, lowestByte);
						cpu.LoadDelayTimer(instruction);
						break;
					case 0x0A:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx0A", instruction, mostSignificantBit, lowestByte);
						cpu.WaitUntilKeyIsPressed(instruction, keypad);
						break;
					case 0x15:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx15", instruction, mostSignificantBit, lowestByte);
						cpu.SetDelayTimer(instruction);
						break;
					case 0x18:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx18", instruction, mostSignificantBit, lowestByte);
						cpu.SetSoundTimer(instruction);
						break;
					case 0x1E:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx1E", instruction, mostSignificantBit, lowestByte);
						cpu.IndexRegisterAddEqualRegister(instruction);
						break;
					case 0x29:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx29", instruction, mostSignificantBit, lowestByte);
						cpu.LoadFontIntoIndexRegister(instruction, ram);
						break;
					case 0x33:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx33", instruction, mostSignificantBit, lowestByte);
						cpu.StoreBinaryCodeRepresentation(instruction, ram);
						break;
					case 0x55:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx55", instruction, mostSignificantBit, lowestByte);
						cpu.StoreRegistersInRam(instruction, ram);
						break;
					case 0x65:
						SPDLOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): 0xFx65", instruction, mostSignificantBit, lowestByte);
						cpu.LoadRegistersFromRam(instruction, ram);
						break;
					default:
						SPDLOG_CRITICAL("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte({2:d}|{2:X}): illegal instruction", instruction, mostSignificantBit, lowestByte);
						assert(false);
						break;
				}
				break;
			}

			default:
				SPDLOG_CRITICAL("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): illegal instruction", instruction, mostSignificantBit);
				return false;
		}

		return true;
	}
}	 // namespace emu
