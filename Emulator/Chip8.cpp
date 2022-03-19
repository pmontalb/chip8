
#include "Chip8.h"

#include <fstream>
#include <iostream>

namespace emu
{
	Chip8::Chip8() { cpu.SetProgramCounter(Ram::instructionStart); }

	void Chip8::LoadRom(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			std::cout << "path[" << path << "]"
					  << "doesn't exist" << std::endl;
			return;
		}
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			std::cout << "file[" << path << "]"
					  << "couldn't be opened" << std::endl;
			return;
		}

		// Get size of file and allocate a buffer to hold the contents
		std::string buffer;
		buffer.resize(file.tellg());

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), buffer.size());
		file.close();

		ram.Load(buffer);
	}

	void Chip8::Cycle()
	{
		const auto instruction = FetchInstruction();

		// pc += 2
		cpu.AdvanceProgramCounter();

		// now execute the instruction
		ExecuteInstruction(instruction);

		cpu.DecrementTimers();
	}

	TwoBytes Chip8::FetchInstruction()
	{
		const auto pc = cpu.GetProgramCounter();
		const auto memByte = ram.GetAt(pc);
		const auto nextMemByte = ram.GetAt(pc + 1);

		// the upper bits come from memByte the lowest bit from nextMemByte
		static constexpr TwoBytes byteShift = 8;
		const TwoBytes topPart = memByte << byteShift;
		return topPart | nextMemByte;
	}

	void Chip8::ExecuteInstruction(const TwoBytes instruction)
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
		switch (mostSignificantBit)
		{
			// 0x0*** instructions
			case 0x0:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				switch (lowestByte)
				{
					case 0xE0:
						display.Clear();
						break;
					case 0xEE:
						cpu.ReturnFromSubRoutine();
						break;
					default:
						assert(false);
						break;
				}
				break;
			}

			// 0x1nnn
			case 0x1:
				cpu.JumpToAddress(instruction);
				break;

			// 0x2nnn
			case 0x2:
				cpu.CallSubRoutine(instruction);
				break;

			// 0x3xkk
			case 0x3:
				cpu.ConditionalSkipIfByteEqual(instruction);
				break;

			// 0x4xkk
			case 0x4:
				cpu.ConditionalSkipIfByteNotEqual(instruction);
				break;

			// 0x5xy0
			case 0x5:
				cpu.ConditionalSkipIfRegistersEqual(instruction);
				break;

			// 0x6xkk
			case 0x6:
				cpu.LoadByte(instruction);
				break;

			// 0x7xkk
			case 0x7:
				cpu.AddEqualByte(instruction);
				break;

			// 0x8*** instructions
			case 0x8:
			{
				const Byte lastFourBits = utils::LowestFourBits(instruction);
				switch (lastFourBits)
				{
					case 0x0:
						cpu.LoadRegister(instruction);
						break;
					case 0x1:
						cpu.OrEqualRegister(instruction);
						break;
					case 0x2:
						cpu.AndEqualRegister(instruction);
						break;
					case 0x3:
						cpu.XorEqualRegister(instruction);
						break;
					case 0x4:
						cpu.AddRegistersAndStoreLastByte(instruction);
						break;
					case 0x5:
						cpu.SubtractEqualRegisters(instruction);
						break;
					case 0x6:
						cpu.ShiftRightAndStoreLastBit(instruction);
						break;
					case 0x7:
						cpu.OppositeSubtractRegisters(instruction);
						break;
					case 0xE:
						cpu.ShiftLeftAndStoreFirstBit(instruction);
						break;
					default:
						assert(false);
						break;
				}
				break;
			}

			// 0x9xy0
			case 0x9:
				cpu.ConditionalSkipIfRegistersNotEqual(instruction);
				break;

			// 0xAnnn
			case 0xA:
				cpu.SetIndexRegister(instruction);
				break;

			// 0xBnnn
			case 0xB:
				cpu.JumpToLastTwelveBitsPlusFirstRegister(instruction);
				break;

			// 0xCxkk
			case 0xC:
				cpu.RandomAndEqualByte(instruction, rng);
				break;

			// 0xDxkk
			case 0xD:
				cpu.Draw(instruction, display, ram);
				break;

			// 0xE*** instructions
			case 0xE:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				switch (lowestByte)
				{
					case 0x9E:
						cpu.ConditionalSkipIfKeyPressed(instruction, keypad);
						break;
					case 0xA1:
						cpu.ConditionalSkipIfKeyNotPressed(instruction, keypad);
						break;
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
						cpu.LoadDelayTimer(instruction);
						break;
					case 0x0A:
						cpu.WaitUntilKeyIsPressed(instruction, keypad);
						break;
					case 0x15:
						cpu.SetDelayTimer(instruction);
						break;
					case 0x18:
						cpu.SetSoundTimer(instruction);
						break;
					case 0x1E:
						cpu.IndexRegisterAddEqualRegister(instruction);
						break;
					case 0x29:
						cpu.LoadFontIntoIndexRegister(instruction, ram);
						break;
					case 0x33:
						cpu.StoreBinaryCodeRepresentation(instruction, ram);
						break;
					case 0x55:
						cpu.StoreRegistersInRam(instruction, ram);
						break;
					case 0x65:
						cpu.LoadRegistersFromRam(instruction, ram);
						break;
					default:
						assert(false);
						break;
				}
				break;
			}

			default:
				assert(false);
				break;
		}
	}
}	 // namespace emu
