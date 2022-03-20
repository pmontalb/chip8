
#include "Cpu.h"
#include "Interfaces/IDisplay.h"
#include "Interfaces/IKeypad.h"
#include "Interfaces/IRam.h"
#include "Interfaces/IRng.h"

#include "spdlog/spdlog.h"

#include <cassert>
#include <limits>

namespace emu
{
	void Cpu::SetProgramCounter(const TwoBytes value)
	{
		SPDLOG_DEBUG("pc=({0:d}|{0:X}) -> ({1:d}|{1:X})", _programCounter, value);
		_programCounter = value;
	}

	void Cpu::AdvanceProgramCounter()
	{
		SPDLOG_DEBUG("pc=({0:d}|{0:X}) -> ({1:d}|{1:X})", _programCounter, _programCounter + 2);
		_programCounter += 2;
	}
	void Cpu::RetreatProgramCounter()
	{
		SPDLOG_DEBUG("pc=({0:d}|{0:X}) -> ({1:d}|{1:X})", _programCounter, _programCounter - 2);
		_programCounter -= 2;
	}

	void Cpu::ReturnFromSubRoutine()
	{
		/*
		 * The top of the stack has the address of one instruction past the one that called the subroutine,
		 * so we can put that back into the PC. Note that this overwrites our preemptive pc += 2 earlier.
		 * */

		SPDLOG_DEBUG("sp({0} -> {1}) stack({2:d}|{2:X}) pc({3})", _stackPointer, _stackPointer - 1, _stack[_stackPointer - 1], _programCounter);
		SetProgramCounter(_stack[--_stackPointer]);
	}

	void Cpu::JumpToAddress(const TwoBytes instruction)
	{
		/* A jump doesn't remember its origin, so no stack interaction required */

		SPDLOG_TRACE("pc({})", _programCounter);
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
		SPDLOG_DEBUG("sp({0:d}|{0:X}->{1:d}|{1:X}) pc({2:d}|{2:X}) stack({3:d}|{3:X} -> {4:d}|{4:X})",
					 _stackPointer, _stackPointer + 1,
					 _programCounter,
					 _stack[_stackPointer + 1],
					 _programCounter);
		_stack[_stackPointer++] = _programCounter;

		SetProgramCounter(utils::LowerTwelveBits(instruction));
	}

	void Cpu::ConditionalSkipIfByteEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) kk({2:d}|{2:X})", Vx, _registers[Vx], kk);
		ConditionalSkip(_registers[Vx] == kk);
	}
	void Cpu::ConditionalSkipIfByteNotEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) kk({2:d}|{2:X})", Vx, _registers[Vx], kk);
		ConditionalSkip(_registers[Vx] != kk);
	}
	void Cpu::ConditionalSkipIfRegistersEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) Vy({2:d}|{2:X}) regVy({3:d}|{3:X})", Vx, _registers[Vx], Vy, _registers[Vy]);
		ConditionalSkip(_registers[Vx] == _registers[Vy]);
	}
	void Cpu::ConditionalSkipIfRegistersNotEqual(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) Vy({2:d}|{2:X}) regVy({3:d}|{3:X})", Vx, _registers[Vx], Vy, _registers[Vy]);
		ConditionalSkip(_registers[Vx] != _registers[Vy]);
	}
	void Cpu::ConditionalSkipIfKeyPressed(const TwoBytes instruction, const IKeypad& keypad)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto keyCode = _registers[Vx];
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) keyCode({1:d}|{1:X}|{})", Vx, keyCode, static_cast<Keys::Enum>(keyCode));
		ConditionalSkip(keypad.IsPressed(static_cast<Keys::Enum>(keyCode)));
	}
	void Cpu::ConditionalSkipIfKeyNotPressed(const TwoBytes instruction, const IKeypad& keypad)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto keyCode = _registers[Vx];
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) keyCode({1:d}|{1:X}|{})", Vx, keyCode, static_cast<Keys::Enum>(keyCode));
		ConditionalSkip(!keypad.IsPressed(static_cast<Keys::Enum>(keyCode)));
	}
	void Cpu::ConditionalSkip(const bool condition)
	{
		SPDLOG_TRACE("condition({})", condition);
		if (condition)
		{
			SPDLOG_DEBUG("advancing pc", condition);
			AdvanceProgramCounter();
		}
	}

	void Cpu::LoadByte(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) <~ kk({2:d}|{2:X})", Vx, _registers[Vx], kk);
		_registers[Vx] = kk;
	}
	void Cpu::LoadRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) <~ Vy({2:d}|{2:X}) regVy({3:d}|{3:X})", Vx, _registers[Vx], Vy, _registers[Vy]);
		_registers[Vx] = _registers[Vy];
	}
	void Cpu::LoadDelayTimer(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) <~ delayTimer({2:d}|{2:X})", Vx, _registers[Vx], _delayTimer);
		_registers[Vx] = _delayTimer;
	}
	void Cpu::LoadFontIntoIndexRegister(const TwoBytes instruction, const IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto digit = _registers[Vx];
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) digit({1:d}|{1:X}) indexRegister({2:d}|{2:X}) <~ font({3:d}|{3:X})", Vx, digit, _indexRegister, ram.GetFontAt(digit));
		_indexRegister = ram.GetFontAt(digit);
	}
	void Cpu::LoadRegistersFromRam(const TwoBytes instruction, IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		SPDLOG_DEBUG("copying the first ({}) elements from ram(start={}) into registers", Vx, _indexRegister);
		ram.WriteTo(_indexRegister, _registers.data(), Vx);
	}

	void Cpu::SetDelayTimer(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) delayTimer({1:d}|{1:X}) <~ regVx({2:d}|{2:X})", Vx, _delayTimer, _registers[Vx]);
		_delayTimer = _registers[Vx];
	}
	void Cpu::SetSoundTimer(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) soundTimer({1:d}|{1:X}) <~ regVx({2:d}|{2:X})", Vx, _soundTimer, _registers[Vx]);
		_soundTimer = _registers[Vx];
	}
	void Cpu::StoreBinaryCodeRepresentation(const TwoBytes instruction, IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		auto value = _registers[Vx];
		SPDLOG_TRACE("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) bcd({1:d}|{1:X})", Vx, _registers[Vx], value);

		constexpr auto base = 10;
		// units
		ram.SetAt(_indexRegister + 2, value % base);
		value /= base;

		// tens
		ram.SetAt(_indexRegister + 1, value % base);
		value /= base;

		// hundreds
		ram.SetAt(_indexRegister, value % base);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) bcd({1:d}|{1:X}) = (h={1:d}|{1:X} | t={1:d}|{1:X} | u={1:d}|{1:X})", Vx, _registers[Vx], value,
					 ram.GetAt(_indexRegister), ram.GetAt(_indexRegister + 1), ram.GetAt(_indexRegister + 2));
	}
	void Cpu::StoreRegistersInRam(const TwoBytes instruction, IRam& ram)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		SPDLOG_DEBUG("storing the first ({}) elements from registers into ram(start={})", Vx, _indexRegister);
		ram.CopyFrom(_indexRegister, _registers.data(), Vx);
	}

	void Cpu::AddEqualByte(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) += kk({2:d}|{2:X}) = {3:d}|{3:X}", Vx, _registers[Vx], kk, _registers[Vx], kk);
		_registers[Vx] += kk;
	}
	void Cpu::IndexRegisterAddEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		SPDLOG_DEBUG("index({0:d}|{0:X}) += Vx({1:d}|{1:X}) regVx({2:d}|{2:X}) = {3:d}|{3:X}", _indexRegister, Vx, _registers[Vx], _indexRegister + _registers[Vx]);
		_indexRegister += _registers[Vx];
	}

	void Cpu::OrEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) |= Vy({2:d}|{2:X}) regVy({3:d}|{3:X}) = {4:d}|{4:X}", Vx, _registers[Vx], Vy, _registers[Vy], _registers[Vx] | _registers[Vy]);
		_registers[Vx] |= _registers[Vy];
	}
	void Cpu::AndEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) &= Vy({2:d}|{2:X}) regVy({3:d}|{3:X}) = {4:d}|{4:X}", Vx, _registers[Vx], Vy, _registers[Vy], _registers[Vx] & _registers[Vy]);
		_registers[Vx] &= _registers[Vy];
	}
	void Cpu::XorEqualRegister(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) ^= Vy({2:d}|{2:X}) regVy({3:d}|{3:X}) = {4:d}|{4:X}", Vx, _registers[Vx], Vy, _registers[Vy], _registers[Vx] ^ _registers[Vy]);
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

		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) + Vy({2:d}|{2:X}) regVy({3:d}|{3:X}) -> sum({3:d}|{3:X}) carry({4:d}|{4:X}) regVx({4:d}|{4:X})",
					 Vx, _registers[Vx], Vy, _registers[Vy], sum, _registers.back(), utils::LowestByte(sum));

		// only the lowest 8 bits of the result are kept, and stored in Vx
		_registers[Vx] = utils::LowestByte(sum);
	}
	void Cpu::SubtractEqualRegisters(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_TRACE("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) - Vy({2:d}|{2:X}) regVy({3:d}|{3:X})", Vx, _registers[Vx], Vy, _registers[Vy]);
		SubtractRegisters(Vx, Vx, Vy);
	}
	void Cpu::OppositeSubtractRegisters(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto Vy = utils::UpperFourBitsLowByte(instruction);
		SPDLOG_TRACE("Vy({0:d}|{0:X}) regVy({1:d}|{1:X}) - Vx({2:d}|{2:X}) regVx({3:d}|{3:X})", Vy, _registers[Vy], Vx, _registers[Vx]);
		SubtractRegisters(Vx, Vy, Vx);
	}
	void Cpu::SubtractRegisters(const Byte regOut, const Byte reg1, const Byte reg2)
	{
		// VF is the last register, or register[0xF]
		// lets's store the 'NOT borrow' there
		_registers.back() = _registers[reg1] > _registers[reg2];

		SPDLOG_DEBUG("V1({0:d}|{0:X}) regV1({1:d}|{1:X}) - V2({2:d}|{2:X}) regV2({3:d}|{3:X}) -> notBorrow({4:d}|{4:X}) regOut[{5:d}|{5:X}]({6:d}|{6:X})",
					 reg1, _registers[reg1], reg2, _registers[reg2], _registers.back(), regOut, _registers[reg1] - _registers[reg2]);

		_registers[regOut] = _registers[reg1] - _registers[reg2];
	}

	void Cpu::ShiftRightAndStoreLastBit(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_registers.back() = utils::LastBit(_registers[Vx]);

		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) >> 1 = {2:d}|{2:X} lastBit({2:d}|{2:X}) ", Vx, _registers[Vx], _registers[Vx] >> 1, _registers.back());
		_registers[Vx] >>= 1;
	}
	void Cpu::ShiftLeftAndStoreFirstBit(const TwoBytes instruction)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		_registers.back() = utils::FirstBit(_registers[Vx]);

		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) << 1 = {2:d}|{2:X} lastBit({2:d}|{2:X}) ", Vx, _registers[Vx], _registers[Vx] << 1, _registers.back());
		_registers[Vx] <<= 1;
	}

	void Cpu::SetIndexRegister(const TwoBytes instruction)
	{
		SPDLOG_DEBUG("index({0:d}|{0:X}) = instr({1:d}|{1:X}) ", _indexRegister, utils::LowerTwelveBits(instruction));
		_indexRegister = utils::LowerTwelveBits(instruction);
	}

	void Cpu::JumpToLastTwelveBitsPlusFirstRegister(const TwoBytes instruction)
	{
		SPDLOG_TRACE("pc({0:d}|{0:X}) <~ reg0({1:d}|{1:X}) + instr({2:d}|{2:X}) ", _programCounter, _registers.front(), utils::LowerTwelveBits(instruction));
		SetProgramCounter(_registers.front() + utils::LowerTwelveBits(instruction));
	}

	void Cpu::RandomAndEqualByte(const TwoBytes instruction, IRng& rng)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);
		const auto kk = utils::LowestByte(instruction);
		const auto random = rng.Next();

		SPDLOG_DEBUG("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) = kk({2:d}|{2:X}) & rand({3:d}|{3:X}) == {4:d}|{4:X}", Vx, _registers[Vx], kk, random, random & kk);
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
		SPDLOG_TRACE("Vx({0:d}|{0:X}) regVx({1:d}|{1:X}) Vy({0:d}|{0:X}) regVy({1:d}|{1:X}) n({1:d}|{1:X}) VF({1:d}|{1:X})",
					 Vx, _registers[Vx], Vy, _registers[Vy], n, _registers.back());

		// wrap x and y
		Byte xCoord = _registers[Vx] % display.GetWidth();
		Byte yCoord = _registers[Vy] % display.GetHeight();
		SPDLOG_TRACE("width({0}) height({1}) xCoord({2:d}|{2:X}) yCoord({3:d}|{3:X})",
					 display.GetWidth(), display.GetHeight(), xCoord, yCoord);

		static constexpr size_t colSize = 8;
		for (size_t row = 0; row < n; ++row)
		{
			const auto sprite = ram.GetAt(_indexRegister + row);
			SPDLOG_TRACE("index({0:d}|{0:X}) row({1}) sprite({2:d}|{2:X})", _indexRegister, row, sprite);

			utils::constexprFor<0, colSize>(
				[&](const auto col)
				{
					uint8_t spritePixel = utils::GetBitAt<col>(sprite);

					// Sprite pixel is on
					if (!spritePixel)
					{
						SPDLOG_TRACE("row({}) col({}) spritePixel is off", row, col);
						return;
					}

					const auto coord = (yCoord + row) * display.GetWidth() + (xCoord + col);
					assert(coord < display.GetWidth() * display.GetHeight());

					const auto screenIsOn = display.GetAt(coord);
					if (screenIsOn)
					{
						SPDLOG_TRACE("row({}) col({}) coord({}) screen=ON: collision", row, col, coord);
						// collision detected: need to store 1 in VF
						_registers.back() = 1;
					}

					// Effectively XOR with the sprite pixel
					SPDLOG_DEBUG("row({}) col({}) coord({}) screen=OFF: turning on", row, col, coord);
					display.FlipAt(coord);
				});
		}
	}

	void Cpu::WaitUntilKeyIsPressed(const TwoBytes instruction, const IKeypad& keypad)
	{
		const auto Vx = utils::LowerFourBitsHighByte(instruction);

		for (Byte i = emu::Keys::START; i < emu::Keys::END; ++i)
		{
			if (!keypad.IsPressed(static_cast<Keys::Enum>(i)))
			{
				SPDLOG_TRACE("key({}) not pressed", static_cast<Keys::Enum>(i));
				continue;
			}

			SPDLOG_DEBUG("key({}) pressed: Vx({1:d}|{1:X}) regVx({2:d}|{2:X}) <~ {3:d}|{3:X}", static_cast<Keys::Enum>(i), Vx, _registers[Vx], i);
			_registers[Vx] = i;
			return;
		}

		// at this point, no key is pressed
		// to simulate the wait we can just replay this instruction
		// and since pc is always advanced, we undo this
		SPDLOG_TRACE("no key pressed: retreating pc expecting to re-enter here");
		RetreatProgramCounter();
	}

	void Cpu::DecrementTimers()
	{
		SPDLOG_TRACE("delayTimer({}) soundTimer({}) decrementing", _delayTimer, _soundTimer);

		if (_delayTimer > 0)
			--_delayTimer;
		if (_soundTimer > 0)
			--_soundTimer;

		SPDLOG_DEBUG("decremented delayTimer({}) soundTimer({})", _delayTimer, _soundTimer);
	}
}	 // namespace emu
