
#include "Emulator/Logging.h"

#include <fstream>
#include <iostream>

namespace emu::detail
{
	static inline const std::function<void(const TwoBytes)> invalidInstruction = [](const TwoBytes instruction)
	{
		LOG_CRITICAL("this instruction({}) shouldn't be called", instruction);
//		assert(false);
	};

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Chip8()
	{
		cpu.SetProgramCounter(Ram::instructionStart);
		LOG_TRACE("Chip8 created and pc={}", cpu.GetProgramCounter());
		PopulateInstructionSetFunctionPointers();
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	void Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::PopulateInstructionSetFunctionPointers()
	{
		_uniquePatternInstructions[0x0] = { detail::_uniquePatternInstructions[0x0], invalidInstruction };
		_uniquePatternInstructions[0x1] = { detail::_uniquePatternInstructions[0x1], [this](const TwoBytes instruction)
											{ this->cpu.JumpToAddress(instruction); } };
		_uniquePatternInstructions[0x2] = { detail::_uniquePatternInstructions[0x2], [this](const TwoBytes instruction)
											{ this->cpu.CallSubRoutine(instruction); } };
		_uniquePatternInstructions[0x3] = { detail::_uniquePatternInstructions[0x3], [this](const TwoBytes instruction)
											{ this->cpu.ConditionalSkipIfByteEqual(instruction); } };
		_uniquePatternInstructions[0x4] = { detail::_uniquePatternInstructions[0x4], [this](const TwoBytes instruction)
											{ this->cpu.ConditionalSkipIfByteNotEqual(instruction); } };
		_uniquePatternInstructions[0x5] = { detail::_uniquePatternInstructions[0x5], [this](const TwoBytes instruction)
											{ this->cpu.ConditionalSkipIfRegistersEqual(instruction); } };
		_uniquePatternInstructions[0x6] = { detail::_uniquePatternInstructions[0x6],
											[this](const TwoBytes instruction) { this->cpu.LoadByte(instruction); } };
		_uniquePatternInstructions[0x7] = { detail::_uniquePatternInstructions[0x7], [this](const TwoBytes instruction)
											{ this->cpu.AddEqualByte(instruction); } };
		_uniquePatternInstructions[0x8] = { detail::_uniquePatternInstructions[0x8], invalidInstruction };
		_uniquePatternInstructions[0x9] = { detail::_uniquePatternInstructions[0x9], [this](const TwoBytes instruction)
											{ this->cpu.ConditionalSkipIfRegistersNotEqual(instruction); } };
		_uniquePatternInstructions[0xA] = { detail::_uniquePatternInstructions[0xA], [this](const TwoBytes instruction)
											{ this->cpu.SetIndexRegister(instruction); } };
		_uniquePatternInstructions[0xB] = { detail::_uniquePatternInstructions[0xB], [this](const TwoBytes instruction)
											{ this->cpu.JumpToLastTwelveBitsPlusFirstRegister(instruction); } };
		_uniquePatternInstructions[0xC] = { detail::_uniquePatternInstructions[0xC], [this](const TwoBytes instruction)
											{ this->cpu.RandomAndEqualByte(instruction, this->rng); } };
		_uniquePatternInstructions[0xD] = { detail::_uniquePatternInstructions[0xD], [this](const TwoBytes instruction)
											{ this->cpu.Draw(instruction, this->display, this->ram); } };

		_0x80xyInstructions[0x0] = { detail::_0x80xyInstructions[0x0],
									 [this](const TwoBytes instruction) { this->cpu.LoadRegister(instruction); } };
		_0x80xyInstructions[0x1] = { detail::_0x80xyInstructions[0x1],
									 [this](const TwoBytes instruction) { this->cpu.OrEqualRegister(instruction); } };
		_0x80xyInstructions[0x2] = { detail::_0x80xyInstructions[0x2],
									 [this](const TwoBytes instruction) { this->cpu.AndEqualRegister(instruction); } };
		_0x80xyInstructions[0x3] = { detail::_0x80xyInstructions[0x3],
									 [this](const TwoBytes instruction) { this->cpu.XorEqualRegister(instruction); } };
		_0x80xyInstructions[0x4] = { detail::_0x80xyInstructions[0x4], [this](const TwoBytes instruction)
									 { this->cpu.AddRegistersAndStoreLastByte(instruction); } };
		_0x80xyInstructions[0x5] = { detail::_0x80xyInstructions[0x5], [this](const TwoBytes instruction)
									 { this->cpu.SubtractEqualRegisters(instruction); } };
		_0x80xyInstructions[0x6] = { detail::_0x80xyInstructions[0x6], [this](const TwoBytes instruction)
									 { this->cpu.ShiftRightAndStoreLastBit(instruction); } };
		_0x80xyInstructions[0x7] = { detail::_0x80xyInstructions[0x7], [this](const TwoBytes instruction)
									 { this->cpu.OppositeSubtractRegisters(instruction); } };
		for (size_t i = 0x8; i <= 0xD; ++i)
			_0x80xyInstructions[i] = { detail::_0x80xyInstructions[i], invalidInstruction };
		_0x80xyInstructions[0xE] = { detail::_0x80xyInstructions[0xE], [this](const TwoBytes instruction)
									 { this->cpu.ShiftLeftAndStoreFirstBit(instruction); } };

		_0x00EkInstructions.fill({ Instruction::INVALID, invalidInstruction });
		_0x00EkInstructions[0x0] = { detail::_0x00EkInstructions[0x0],
									 [this](const TwoBytes) { this->display.Clear(); } };
		_0x00EkInstructions[0xE] = { detail::_0x00EkInstructions[0xE],
									 [this](const TwoBytes) { this->cpu.ReturnFromSubRoutine(); } };

		_0xExyzInstructions.fill({ Instruction::INVALID, invalidInstruction });
		_0xExyzInstructions[0x1] = { detail::_0xExyzInstructions[0x1],
									 [this](const TwoBytes instruction) { this->cpu.ConditionalSkipIfKeyPressed(instruction, this->keypad); } };
		_0xExyzInstructions[0xE] = { detail::_0xExyzInstructions[0xE],
									 [this](const TwoBytes instruction) { this->cpu.ConditionalSkipIfKeyNotPressed(instruction, this->keypad); } };

		_0xFxyzInstructions.fill({ Instruction::INVALID, invalidInstruction });
		_0xFxyzInstructions[0x07] = { detail::_0xFxyzInstructions[0x07],
									  [this](const TwoBytes instruction) { this->cpu.LoadDelayTimer(instruction); } };
		_0xFxyzInstructions[0x0A] = { detail::_0xFxyzInstructions[0x0A], [this](const TwoBytes instruction)
									  { this->cpu.WaitUntilKeyIsPressed(instruction, this->keypad); } };
		_0xFxyzInstructions[0x15] = { detail::_0xFxyzInstructions[0x15],
									  [this](const TwoBytes instruction) { this->cpu.SetDelayTimer(instruction); } };
		_0xFxyzInstructions[0x18] = { detail::_0xFxyzInstructions[0x18],
									  [this](const TwoBytes instruction) { this->cpu.SetSoundTimer(instruction); } };
		_0xFxyzInstructions[0x1E] = { detail::_0xFxyzInstructions[0x1E], [this](const TwoBytes instruction)
									  { this->cpu.IndexRegisterAddEqualRegister(instruction); } };
		_0xFxyzInstructions[0x29] = { detail::_0xFxyzInstructions[0x29], [this](const TwoBytes instruction)
									  { this->cpu.LoadFontIntoIndexRegister(instruction, this->ram); } };
		_0xFxyzInstructions[0x33] = { detail::_0xFxyzInstructions[0x33], [this](const TwoBytes instruction)
									  { this->cpu.StoreBinaryCodeRepresentation(instruction, this->ram); } };
		_0xFxyzInstructions[0x55] = { detail::_0xFxyzInstructions[0x55], [this](const TwoBytes instruction)
									  { this->cpu.StoreRegistersInRam(instruction, this->ram); } };
		_0xFxyzInstructions[0x65] = { detail::_0xFxyzInstructions[0x65], [this](const TwoBytes instruction)
									  { this->cpu.LoadRegistersFromRam(instruction, this->ram); } };
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	bool Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::LoadRom(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			LOG_CRITICAL("path({}) doesn't exist", path.string());
			return false;
		}
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			LOG_CRITICAL("file({}) couldn't be opened", path.string());
			return false;
		}

		// Get size of file and allocate a buffer to hold the contents
		std::string buffer;
		buffer.resize(static_cast<std::size_t>(file.tellg()));
		LOG_TRACE("buffer size({})", buffer.size());
		assert(buffer.size() < ram.GetSize() - Ram::instructionStart);

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<long>(buffer.size()));

		ram.Load(buffer);

		return true;
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	bool Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Cycle()
	{
		const auto instruction = FetchInstruction();
		LOG_TRACE("fetched instruction({0:d}|{0:X})", instruction);

		// pc += 2
		cpu.AdvanceProgramCounter();

		// now execute the instruction
		if (!ExecuteInstruction(instruction))
			return false;

		cpu.DecrementTimers();

		return true;
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	TwoBytes Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::FetchInstruction()
	{
		const auto pc = cpu.GetProgramCounter();
		const auto memByte = ram.GetAt(pc);
		const auto nextMemByte = ram.GetAt(pc + 1);

		// the upper bits come from memByte the lowest bit from nextMemByte
		static constexpr TwoBytes byteShift = 8;
		const auto topPart = static_cast<TwoBytes>(memByte << byteShift);

		LOG_DEBUG("pc({0}) ram=({1:d}|{1:X}) nextRam=({2:d}|{2:X}) instruction({3:d}|{3:X})", pc, memByte, nextMemByte,
				  topPart | nextMemByte);

		return static_cast<TwoBytes>(topPart | nextMemByte);
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	bool Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::ExecuteInstruction(const TwoBytes instruction)
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

		_lastExecutedInstruction = Instruction::INVALID;

		// zero out the first 3 nibbles and shift it down by 12 bits
		const auto mostSignificantBit = static_cast<Byte>((instruction & 0xF000ul) >> 12ul);
		LOG_TRACE("instruction({0:d}|{0:X}): msb({1:d}|{1:X})", instruction, mostSignificantBit);

		switch (mostSignificantBit)
		{
			// 0x0*** instructions
			case 0x0:
			{
				const auto lowestFourBits = utils::LowestFourBits(instruction);
				LOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestFourBits({2:d}|{2:X})",
						  instruction, mostSignificantBit, lowestFourBits);
				_0x00EkInstructions[lowestFourBits].second(instruction);
				_lastExecutedInstruction = _0x00EkInstructions[lowestFourBits].first;
				break;
			}

			// 0x1nnn
			case 0x1:
			case 0x2:
			case 0x3:
			case 0x4:
			case 0x5:
			case 0x6:
			case 0x7:
			case 0x9:
			case 0xA:
			case 0xB:
			case 0xC:
			case 0xD:
				LOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X})", instruction, mostSignificantBit);
				_uniquePatternInstructions[mostSignificantBit].second(instruction);
				_lastExecutedInstruction = _uniquePatternInstructions[mostSignificantBit].first;
				break;

			// 0x8*** instructions
			case 0x8:
			{
				const Byte lastFourBits = utils::LowestFourBits(instruction);
				LOG_DEBUG("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X})",
						  instruction, mostSignificantBit, lastFourBits);
				_0x80xyInstructions[lastFourBits].second(instruction);
				_lastExecutedInstruction = _0x80xyInstructions[lastFourBits].first;
				break;
			}

			// 0xE*** instructions
			case 0xE:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				_0xExyzInstructions[lowestByte].second(instruction);
				_lastExecutedInstruction = _0xExyzInstructions[lowestByte].first;
				break;
			}

			// 0xF*** instructions
			case 0xF:
			{
				const auto lowestByte = utils::LowestByte(instruction);
				_0xFxyzInstructions[lowestByte].second(instruction);
				_lastExecutedInstruction = _0xFxyzInstructions[lowestByte].first;
				break;
			}

			default:
				LOG_CRITICAL("instruction({0:d}|{0:X}) msb({1:d}|{1:X}): illegal instruction", instruction,
							 mostSignificantBit);
//				assert(false);
				return false;
		}

		return true;
	}
}	 // namespace emu::detail
