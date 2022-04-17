
#include "Emulator/Logging.h"

#include <fstream>
#include <iostream>

namespace emu::detail
{
	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Chip8()
		: _invalidInstruction(
			  [this](const TwoBytes instruction)
			  {
				  LOG_CRITICAL("this instruction({}) shouldn't be called", instruction);
				  _lastError = Error::InvalidInstruction;
			  })
	{
		Initialize();
		PopulateInstructionSetFunctionPointers();
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	void Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::PopulateInstructionSetFunctionPointers()
	{
		_uniquePatternInstructions[0x0] = { detail::_uniquePatternInstructions[0x0], _invalidInstruction };
		_uniquePatternInstructions[0x1] = { detail::_uniquePatternInstructions[0x1], [this](const TwoBytes instruction)
											{ this->_cpu.JumpToAddress(instruction); } };
		_uniquePatternInstructions[0x2] = { detail::_uniquePatternInstructions[0x2], [this](const TwoBytes instruction)
											{ this->_cpu.CallSubRoutine(instruction); } };
		_uniquePatternInstructions[0x3] = { detail::_uniquePatternInstructions[0x3], [this](const TwoBytes instruction)
											{ this->_cpu.ConditionalSkipIfByteEqual(instruction); } };
		_uniquePatternInstructions[0x4] = { detail::_uniquePatternInstructions[0x4], [this](const TwoBytes instruction)
											{ this->_cpu.ConditionalSkipIfByteNotEqual(instruction); } };
		_uniquePatternInstructions[0x5] = { detail::_uniquePatternInstructions[0x5], [this](const TwoBytes instruction)
											{ this->_cpu.ConditionalSkipIfRegistersEqual(instruction); } };
		_uniquePatternInstructions[0x6] = { detail::_uniquePatternInstructions[0x6],
											[this](const TwoBytes instruction) { this->_cpu.LoadByte(instruction); } };
		_uniquePatternInstructions[0x7] = { detail::_uniquePatternInstructions[0x7], [this](const TwoBytes instruction)
											{ this->_cpu.AddEqualByte(instruction); } };
		_uniquePatternInstructions[0x8] = { detail::_uniquePatternInstructions[0x8], _invalidInstruction };
		_uniquePatternInstructions[0x9] = { detail::_uniquePatternInstructions[0x9], [this](const TwoBytes instruction)
											{ this->_cpu.ConditionalSkipIfRegistersNotEqual(instruction); } };
		_uniquePatternInstructions[0xA] = { detail::_uniquePatternInstructions[0xA], [this](const TwoBytes instruction)
											{ this->_cpu.SetIndexRegister(instruction); } };
		_uniquePatternInstructions[0xB] = { detail::_uniquePatternInstructions[0xB], [this](const TwoBytes instruction)
											{ this->_cpu.JumpToLastTwelveBitsPlusFirstRegister(instruction); } };
		_uniquePatternInstructions[0xC] = { detail::_uniquePatternInstructions[0xC], [this](const TwoBytes instruction)
											{ this->_cpu.RandomAndEqualByte(instruction, this->_rng); } };
		_uniquePatternInstructions[0xD] = { detail::_uniquePatternInstructions[0xD], [this](const TwoBytes instruction)
											{ this->_cpu.Draw(instruction, this->_display, this->_ram); } };

		_0x80xyInstructions[0x0] = { detail::_0x80xyInstructions[0x0],
									 [this](const TwoBytes instruction) { this->_cpu.LoadRegister(instruction); } };
		_0x80xyInstructions[0x1] = { detail::_0x80xyInstructions[0x1],
									 [this](const TwoBytes instruction) { this->_cpu.OrEqualRegister(instruction); } };
		_0x80xyInstructions[0x2] = { detail::_0x80xyInstructions[0x2],
									 [this](const TwoBytes instruction) { this->_cpu.AndEqualRegister(instruction); } };
		_0x80xyInstructions[0x3] = { detail::_0x80xyInstructions[0x3],
									 [this](const TwoBytes instruction) { this->_cpu.XorEqualRegister(instruction); } };
		_0x80xyInstructions[0x4] = { detail::_0x80xyInstructions[0x4], [this](const TwoBytes instruction)
									 { this->_cpu.AddRegistersAndStoreLastByte(instruction); } };
		_0x80xyInstructions[0x5] = { detail::_0x80xyInstructions[0x5], [this](const TwoBytes instruction)
									 { this->_cpu.SubtractEqualRegisters(instruction); } };
		_0x80xyInstructions[0x6] = { detail::_0x80xyInstructions[0x6], [this](const TwoBytes instruction)
									 { this->_cpu.ShiftRightAndStoreLastBit(instruction); } };
		_0x80xyInstructions[0x7] = { detail::_0x80xyInstructions[0x7], [this](const TwoBytes instruction)
									 { this->_cpu.OppositeSubtractRegisters(instruction); } };
		for (size_t i = 0x8; i <= 0xD; ++i)
			_0x80xyInstructions[i] = { detail::_0x80xyInstructions[i], _invalidInstruction };
		_0x80xyInstructions[0xE] = { detail::_0x80xyInstructions[0xE], [this](const TwoBytes instruction)
									 { this->_cpu.ShiftLeftAndStoreFirstBit(instruction); } };

		_0x00EkInstructions.fill({ Instruction::END, _invalidInstruction });
		_0x00EkInstructions[0x0] = { detail::_0x00EkInstructions[0x0],
									 [this](const TwoBytes) { this->_display.Clear(); } };
		_0x00EkInstructions[0xE] = { detail::_0x00EkInstructions[0xE],
									 [this](const TwoBytes) { this->_cpu.ReturnFromSubRoutine(); } };

		_0xExyzInstructions.fill({ Instruction::END, _invalidInstruction });
		_0xExyzInstructions[0x1] = { detail::_0xExyzInstructions[0x1], [this](const TwoBytes instruction)
									 { this->_cpu.ConditionalSkipIfKeyNotPressed(instruction, this->_keypad); } };
		_0xExyzInstructions[0xE] = { detail::_0xExyzInstructions[0xE], [this](const TwoBytes instruction)
									 { this->_cpu.ConditionalSkipIfKeyPressed(instruction, this->_keypad); } };

		_0xFxyzInstructions.fill({ Instruction::END, _invalidInstruction });
		_0xFxyzInstructions[0x07] = { detail::_0xFxyzInstructions[0x07],
									  [this](const TwoBytes instruction) { this->_cpu.LoadDelayTimer(instruction); } };
		_0xFxyzInstructions[0x0A] = { detail::_0xFxyzInstructions[0x0A], [this](const TwoBytes instruction)
									  { this->_cpu.WaitUntilKeyIsPressed(instruction, this->_keypad); } };
		_0xFxyzInstructions[0x15] = { detail::_0xFxyzInstructions[0x15],
									  [this](const TwoBytes instruction) { this->_cpu.SetDelayTimer(instruction); } };
		_0xFxyzInstructions[0x18] = { detail::_0xFxyzInstructions[0x18],
									  [this](const TwoBytes instruction) { this->_cpu.SetSoundTimer(instruction); } };
		_0xFxyzInstructions[0x1E] = { detail::_0xFxyzInstructions[0x1E], [this](const TwoBytes instruction)
									  { this->_cpu.IndexRegisterAddEqualRegister(instruction); } };
		_0xFxyzInstructions[0x29] = { detail::_0xFxyzInstructions[0x29], [this](const TwoBytes instruction)
									  { this->_cpu.LoadFontIntoIndexRegister(instruction, this->_ram); } };
		_0xFxyzInstructions[0x33] = { detail::_0xFxyzInstructions[0x33], [this](const TwoBytes instruction)
									  { this->_cpu.StoreBinaryCodeRepresentation(instruction, this->_ram); } };
		_0xFxyzInstructions[0x55] = { detail::_0xFxyzInstructions[0x55], [this](const TwoBytes instruction)
									  { this->_cpu.StoreRegistersInRam(instruction, this->_ram); } };
		_0xFxyzInstructions[0x65] = { detail::_0xFxyzInstructions[0x65], [this](const TwoBytes instruction)
									  { this->_cpu.LoadRegistersFromRam(instruction, this->_ram); } };

		utils::ConstexprFor<0, 0x10>(
			[&](const auto i)
			{
				if constexpr (i == 0)
				{
					_instructionSet[i] = {
						Instruction::END,
						[&](const TwoBytes instruction)
						{
							const auto lowestFourBits = utils::LowestFourBits(instruction);
							if (lowestFourBits >= _0x00EkInstructions.size())
							{
								LOG_CRITICAL(
									"instruction({0:d}|{0:X}) msb({1:d}|{1:X}): 0x00E{1:X} -> illegal instruction",
									instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul));
								_lastError = Error::InvalidInstruction;
								return;
							}

							LOG_INFO("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestFourBits({2:d}|{2:X}) 0x00E{2:X}",
									 instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lowestFourBits);
							_0x00EkInstructions[lowestFourBits].second(instruction);
							_lastExecutedInstructionCode = _0x00EkInstructions[lowestFourBits].first;
							_lastExecutedInstruction = instruction;
						}
					};
				}
				else if constexpr (i == 8)
				{
					_instructionSet[i] = {
						Instruction::END,
						[&](const TwoBytes instruction)
						{
							const auto lastFourBits = utils::LowestFourBits(instruction);
							if (lastFourBits >= _0x80xyInstructions.size())
							{
								LOG_CRITICAL(
									"instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lastFourBits({1:X}): 0x80{1:X} -> illegal instruction",
									instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lastFourBits);
								_lastError = Error::InvalidInstruction;
								return;
							}
							LOG_INFO("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0x8xy{2:X}",
									 instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lastFourBits);
							_0x80xyInstructions[lastFourBits].second(instruction);
							_lastExecutedInstructionCode = _0x80xyInstructions[lastFourBits].first;
							_lastExecutedInstruction = instruction;
						}
					};
				}
				else if constexpr (i == 0xE)
				{
					_instructionSet[i] = {
						Instruction::END,
						[&](const TwoBytes instruction)
						{
							const auto lastFourBits = utils::LowestFourBits(instruction);
							if (lastFourBits >= _0xExyzInstructions.size())
							{
								LOG_CRITICAL(
									"instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lastFourBits({1:X}): 0xExy{1:X} -> illegal instruction",
									instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lastFourBits);
								_lastError = Error::InvalidInstruction;
								return;
							}
							LOG_INFO("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0xExy{2:X}",
									 instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lastFourBits);
							_0xExyzInstructions[lastFourBits].second(instruction);
							_lastExecutedInstructionCode = _0xExyzInstructions[lastFourBits].first;
							_lastExecutedInstruction = instruction;
						}
					};
				}
				else if constexpr (i == 0xF)
				{
					_instructionSet[i] = {
						Instruction::END,
						[&](const TwoBytes instruction)
						{
							const auto lowestByte = utils::LowestByte(instruction);
							if (lowestByte >= _0xFxyzInstructions.size())
							{
								LOG_CRITICAL(
									"instruction({0:d}|{0:X}) msb({1:d}|{1:X}) lowestByte(1:X): 0xFx{2:X} -> illegal instruction",
									instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lowestByte);
								_lastError = Error::InvalidInstruction;
								return;
							}
							LOG_INFO("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) last4Bits({2:d}|{2:X}): 0xFx{2:X}",
									 instruction, static_cast<Byte>((instruction & 0xF000ul) >> 12ul), lowestByte);
							_0xFxyzInstructions[lowestByte].second(instruction);
							_lastExecutedInstructionCode = _0xFxyzInstructions[lowestByte].first;
							_lastExecutedInstruction = instruction;
						}
					};
				}
				else
				{
					_instructionSet[i] = { Instruction::END, [&](const TwoBytes instruction)
										   {
											   const auto mostSignificantBit =
												   static_cast<Byte>((instruction & 0xF000ul) >> 12ul);

											   LOG_INFO("instruction({0:d}|{0:X}) msb({1:d}|{1:X}) 0x{1:X}xyz",
														instruction, mostSignificantBit);
											   _uniquePatternInstructions[mostSignificantBit].second(instruction);
											   _lastExecutedInstructionCode =
												   _uniquePatternInstructions[mostSignificantBit].first;
											   _lastExecutedInstruction = instruction;
										   } };
				}
			});
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	void Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Initialize()
	{
		_cpu = CpuT {};
		_display = DisplayT {};
		_keypad = KeypadT {};

		_cpu.SetProgramCounter(static_cast<TwoBytes>(_ram.GetInstructionStartAddress()));
		LOG_TRACE("Chip8 created and pc={}", _cpu.GetProgramCounter());
	}
	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	bool Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::LoadRom(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			LOG_CRITICAL("path({}) doesn't exist", path.string());
			return false;
		}
		if (!std::filesystem::is_regular_file(path) && !std::filesystem::is_symlink(path))
		{
			LOG_CRITICAL("file({}) couldn't be opened", path.string());
			return false;
		}

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		assert(file.is_open());

		Initialize();

		// Get size of file and allocate a buffer to hold the contents
		std::string buffer;
		buffer.resize(static_cast<std::size_t>(file.tellg()));
		LOG_TRACE("buffer size({})", buffer.size());
		assert(buffer.size() < _ram.GetSize() - _ram.GetInstructionStartAddress());

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<long>(buffer.size()));

		_ram.Load(buffer);

		return true;
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	bool Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Cycle()
	{
		if (!IsValid())
			return false;

		const auto instruction = FetchInstruction();
		LOG_TRACE("fetched instruction({0:d}|{0:X})", instruction);

		if (_cpu.GetProgramCounter() < _ram.GetInstructionStartAddress())
		{
			_lastError = Error::InvalidProgramCounter;
			return false;
		}

		// pc += 2
		_cpu.AdvanceProgramCounter();

		// now execute the instruction
		if (!ExecuteInstruction(instruction))
			return false;

		_cpu.DecrementTimers();

		return true;
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	void Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Rewind()
	{
		_cpu.RetreatProgramCounter();
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	TwoBytes Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::FetchInstruction()
	{
		const auto pc = _cpu.GetProgramCounter();
		const auto memByte = _ram.GetAt(pc);
		const auto nextMemByte = _ram.GetAt(pc + 1);

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

		_lastExecutedInstructionCode = Instruction::END;
		_lastExecutedInstruction = 0x0;

		// zero out the first 3 nibbles and shift it down by 12 bits
		const auto mostSignificantBit = static_cast<Byte>((instruction & 0xF000ul) >> 12ul);
		LOG_TRACE("instruction({0:d}|{0:X}): msb({1:d}|{1:X})", instruction, mostSignificantBit);
		_instructionSet[mostSignificantBit].second(instruction);

		return IsValid();
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	void Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Serialize(std::vector<Byte>& byteArray) const
	{
		_cpu.Serialize(byteArray);
		_ram.Serialize(byteArray);
		_display.Serialize(byteArray);
		_keypad.Serialize(byteArray);
	}

	template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
	utils::Span<Byte> Chip8<CpuT, RngT, RamT, DisplayT, KeypadT>::Deserialize(const utils::Span<Byte>& byteArray)
	{
		auto localByteArray = byteArray;
		localByteArray = _cpu.Deserialize(localByteArray);
		localByteArray = _ram.Deserialize(localByteArray);
		localByteArray = _display.Deserialize(localByteArray);
		localByteArray = _keypad.Deserialize(localByteArray);

		return localByteArray;
	}

}	 // namespace emu::detail
