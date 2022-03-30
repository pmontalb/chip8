
#pragma once

#include "Utilities.h"
#include <array>

namespace emu
{
	class IRng;
	class IDisplay;
	class IRam;
	class IKeypad;

	class Cpu
	{
		static constexpr std::size_t stackSize = 16;
		static constexpr std::size_t registerSize = 16;

		/*
		 *  http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#1nnn
		 *  nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
			n or nibble - A 4-bit value, the lowest 4 bits of the instruction
			x - A 4-bit value, the lower 4 bits of the high byte of the instruction
			y - A 4-bit value, the upper 4 bits of the low byte of the instruction
			kk or byte - An 8-bit value, the lowest 8 bits of the instruction
		 */

	public:
		[[nodiscard]] auto GetProgramCounter() const { return _programCounter; }
		void SetProgramCounter(const TwoBytes value);
		void AdvanceProgramCounter();
		void RetreatProgramCounter();

		void ReturnFromSubRoutine();
		void JumpToAddress(const TwoBytes instruction);
		void CallSubRoutine(const TwoBytes instruction);

		void ConditionalSkipIfByteEqual(const TwoBytes instruction);
		void ConditionalSkipIfByteNotEqual(const TwoBytes instruction);
		void ConditionalSkipIfRegistersEqual(const TwoBytes instruction);
		void ConditionalSkipIfRegistersNotEqual(const TwoBytes instruction);
		void ConditionalSkipIfKeyPressed(const TwoBytes instruction, const IKeypad& keypad);
		void ConditionalSkipIfKeyNotPressed(const TwoBytes instruction, const IKeypad& keypad);

		void LoadByte(const TwoBytes instruction);
		void LoadRegister(const TwoBytes instruction);
		void LoadDelayTimer(const TwoBytes instruction);
		void LoadFontIntoIndexRegister(const TwoBytes instruction, const IRam& ram);
		void LoadRegistersFromRam(const TwoBytes instruction, IRam& ram);

		void SetDelayTimer(const TwoBytes instruction);
		void SetSoundTimer(const TwoBytes instruction);

		void StoreBinaryCodeRepresentation(const TwoBytes instruction, IRam& ram);
		void StoreRegistersInRam(const TwoBytes instruction, IRam& ram);

		void AddEqualByte(const TwoBytes instruction);
		void IndexRegisterAddEqualRegister(const TwoBytes instruction);

		void OrEqualRegister(const TwoBytes instruction);
		void AndEqualRegister(const TwoBytes instruction);
		void XorEqualRegister(const TwoBytes instruction);

		void AddRegistersAndStoreLastByte(const TwoBytes instruction);
		void SubtractEqualRegisters(const TwoBytes instruction);
		void OppositeSubtractRegisters(const TwoBytes instruction);

		void ShiftRightAndStoreLastBit(const TwoBytes instruction);
		void ShiftLeftAndStoreFirstBit(const TwoBytes instruction);

		void SetIndexRegister(const TwoBytes instruction);
		void JumpToLastTwelveBitsPlusFirstRegister(const TwoBytes instruction);

		void RandomAndEqualByte(const TwoBytes instruction, IRng& rng);

		void Draw(const TwoBytes instruction, IDisplay& display, const IRam& ram);

		void WaitUntilKeyIsPressed(const TwoBytes instruction, const IKeypad& keypad);

		void DecrementTimers();

		auto GetIndexRegister() const { return _indexRegister; }
		auto GetStackPointer() const { return _stackPointer; }
		const auto& GetStack() const { return _stack; }
		const auto& GetRegisters() const { return _registers; }
		auto GetDelayTimer() const { return _delayTimer; }
		auto GetSoundTimer() const { return _soundTimer; }
	private:
		void ConditionalSkip(const bool condition);
		void SubtractRegisters(const Byte regOut, const Byte reg1, const Byte reg2);

	protected:
		// 16 bits because it has to hold the max memory address (0xFFF)
		TwoBytes _programCounter = 0;	// program counter
		TwoBytes _indexRegister = 0;		// index register

		Byte _stackPointer = 0;
		std::array<TwoBytes, stackSize> _stack = utils::FlatInitializedArray<TwoBytes, stackSize>(0x0);
		// general-purpose register
		std::array<Byte, registerSize> _registers = utils::FlatInitializedArray<Byte, stackSize>(0x0);

		Byte _delayTimer = 0;
		Byte _soundTimer = 0;
	};
}	 // namespace emu
