
#include "Cpu.h"
#include "Interfaces/IDisplay.h"
#include "Interfaces/IKeypad.h"
#include "Interfaces/IRam.h"
#include "Interfaces/IRng.h"

#include <cassert>
#include <limits>

namespace emu
{
	void Cpu::SetProgramCounter(const TwoBytes value)
	{ _programCounter = value;
	}

	void Cpu::AdvanceProgramCounter() { _programCounter += 2; }
	void Cpu::RetreatProgramCounter() { _programCounter -= 2; }

	void Cpu::ReturnFromSubRoutine()
	{
		/*
		 * The top of the stack has the address of one instruction past the one that called the subroutine,
		 * so we can put that back into the PC. Note that this overwrites our preemptive pc += 2 earlier.
		 * */
		SetProgramCounter(_stack[--_stackPointer]);
	}

	void Cpu::JumpToAddress(const TwoBytes instruction)
	{
		/* A jump doesn't remember its origin, so no stack interaction required */

		SetProgramCounter(utils::LowerTwelveBits(instruction));
	}

	void Cpu::CallSubRoutine(const TwoBytes instruction)
	{
		/*
		 * When we call a subroutine, we want to return eventually, so we put the current PC onto the top of the stack.
		 * Remember that we did pc += 2 in Cycle(), so the current PC holds the next instruction after this CALL, which is correct.
		 * We don’t want to return to the CALL instruction because it would be an infinite loop of CALLs and RETs.
		 * */

		// current pc on top of the stack, and advancing the stack
		_stack[_stackPointer++] = _programCounter;

		SetProgramCounter(utils::LowerTwelveBits(instruction));
	}

	void Cpu::ConditionalSkipIfByteEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		ConditionalSkip(_registers[Vx] == kk);
	}
	void Cpu::ConditionalSkipIfByteNotEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		ConditionalSkip(_registers[Vx] != kk);
	}
	void Cpu::ConditionalSkipIfRegistersEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		ConditionalSkip(_registers[Vx] == _registers[Vy]);
	}
	void Cpu::ConditionalSkipIfRegistersNotEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		ConditionalSkip(_registers[Vx] != _registers[Vy]);
	}
	void Cpu::ConditionalSkipIfKeyPressed(const TwoBytes instruction, const IKeypad& keypad)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto keyCode = _registers[Vx];
		ConditionalSkip(keypad.IsPressed(static_cast<Keys::Enum>(keyCode)));
	}
	void Cpu::ConditionalSkipIfKeyNotPressed(const TwoBytes instruction, const IKeypad& keypad)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto keyCode = _registers[Vx];
		ConditionalSkip(!keypad.IsPressed(static_cast<Keys::Enum>(keyCode)));
	}
	void Cpu::ConditionalSkip(const bool condition)
	{
		if (condition)
			AdvanceProgramCounter();
	}

	void Cpu::LoadByte(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		_registers[Vx] = kk;
	}
	void Cpu::LoadRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		_registers[Vx] = _registers[Vy];
	}
	void Cpu::LoadDelayTimer(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_registers[Vx] = _delayTimer;
	}
	void Cpu::LoadFontIntoIndexRegister(const TwoBytes instruction, const IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto digit = _registers[Vx];
		_indexRegister = ram.GetFontAt(digit);
	}
	void Cpu::LoadRegistersFromRam(const TwoBytes instruction, IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		ram.WriteTo(_indexRegister, _registers.data(), Vx);
	}

	void Cpu::SetDelayTimer(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_delayTimer = _registers[Vx];
	}
	void Cpu::SetSoundTimer(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_soundTimer = _registers[Vx];
	}
	void Cpu::StoreBinaryCodeRepresentation(const TwoBytes instruction, IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		auto value = _registers[Vx];

		constexpr auto base = 10;
		// units
		ram.SetAt(_indexRegister + 2, value % base);
		value /= base;

		// tens
		ram.SetAt(_indexRegister + 1, value % base);
		value /= base;

		// hundreds
		ram.SetAt(_indexRegister, value % base);
	}
	void Cpu::StoreRegistersInRam(const TwoBytes instruction, IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		ram.CopyFrom(_indexRegister, _registers.data(), Vx);
	}

	void Cpu::AddEqualByte(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		_registers[Vx] += kk;
	}
	void Cpu::IndexRegisterAddEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_indexRegister += _registers[Vx];
	}

	void Cpu::OrEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		_registers[Vx] |= _registers[Vy];
	}
	void Cpu::AndEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		_registers[Vx] &= _registers[Vy];
	}
	void Cpu::XorEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		_registers[Vx] ^= _registers[Vy];
	}

	void Cpu::AddRegistersAndStoreLastByte(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);

		TwoBytes sum = _registers[Vx] + _registers[Vy];

		// VF is the last register, or register[0xF]
		// lets's store the carry there
		_registers.back() = sum > std::numeric_limits<Byte>::max();

		// only the lowest 8 bits of the result are kept, and stored in Vx
		_registers[Vx] = utils::LowestByte(sum);
	}
	void Cpu::SubtractEqualRegisters(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);

		SubtractRegisters(Vx, Vx, Vy);
	}
	void Cpu::OppositeSubtractRegisters(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);

		SubtractRegisters(Vx, Vy, Vx);
	}
	void Cpu::SubtractRegisters(const Byte regOut, const Byte reg1, const Byte reg2)
	{
		// VF is the last register, or register[0xF]
		// lets's store the 'NOT borrow' there
		_registers.back() = _registers[reg1] > _registers[reg2];

		_registers[regOut] = _registers[reg1] - _registers[reg2];
	}

	void Cpu::ShiftRightAndStoreLastBit(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_registers.back() = utils::LastBit(_registers[Vx]);
		_registers[Vx] >>= 1;
	}
	void Cpu::ShiftLeftAndStoreFirstBit(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_registers.back() = utils::FirstBit(_registers[Vx]);
		_registers[Vx] <<= 1;
	}

	void Cpu::SetIndexRegister(const TwoBytes instruction) { _indexRegister = utils::LowerTwelveBits(instruction); }

	void Cpu::JumpToLastTwelveBitsPlusFirstRegister(const TwoBytes instruction) { SetProgramCounter(_registers.front() + utils::LowerTwelveBits(instruction)); }

	void Cpu::RandomAndEqualByte(const TwoBytes instruction, IRng& rng)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		const auto random = rng.Next();
		_registers[Vx] = random & kk;
	}

	void Cpu::Draw(const TwoBytes instruction, IDisplay& display, const IRam& ram)
	{
		/*
		 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
				The interpreter reads n bytes from memory, starting at the address stored in I.
				These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
				Sprites are XORed onto the existing screen.
				If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
				If the sprite is positioned so part of it is outside the coordinates of the display,
				it wraps around to the opposite side of the screen.

				See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information
				on the Chip-8 screen and sprites.
		 */
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		const auto n = utils::LowestFourBits(instruction);
		_registers.back() = 0;

		// wrap x and y
		Byte xCoord = _registers[Vx] % display.GetWidth();
		Byte yCoord = _registers[Vy] % display.GetHeight();

		static constexpr size_t colSize = 8;
		for (size_t row = 0; row < n; ++row)
		{
			const auto sprite = ram.GetAt(_indexRegister + row);
			utils::constexprFor<0, colSize>(
				[&](const auto col)
				{
					uint8_t spritePixel = utils::GetBitAt<col>(sprite);

					// Sprite pixel is on
					if (!spritePixel)
						return;

					const auto coord = (yCoord + row) * display.GetWidth() + (xCoord + col);
					assert(coord < display.GetWidth() * display.GetHeight());

					const auto screenIsOn = display.GetAt(coord);
					if (screenIsOn)
					{
						// collision detected: need to store 1 in VF
						_registers.back() = 1;
					}

					// Effectively XOR with the sprite pixel
					display.FlipAt(coord);
				});
		}
	}

	void Cpu::WaitUntilKeyIsPressed(const TwoBytes instruction, const IKeypad& keypad)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);

		for (Byte i = 0; i < keypad.GetSize(); ++i)
		{
			if (!keypad.IsPressed(static_cast<Keys::Enum>(i)))
				continue;

			_registers[Vx] = i;
			return;
		}

		// at this point, no key is pressed
		// to simulate the wait we can just replay this instruction
		// and since pc is always advanced, we undo this
		RetreatProgramCounter();
	}

	void Cpu::DecrementTimers()
	{
		if (_delayTimer > 0)
			--_delayTimer;
		if (_soundTimer > 0)
			--_soundTimer;
	}
}	 // namespace emu
