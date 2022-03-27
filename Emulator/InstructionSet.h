
#pragma once

#include "Emulator/Utilities.h"

#include <array>
#include <string_view>

namespace emu
{
	enum class Instruction
	{
		_0x00E0,
		_0x00EE,
		
		_0x1nnn,
		_0x2nnn,
		_0x3xkk,
		_0x4xkk,
		_0x5xy0,
		_0x6xkk,
		_0x7xkk,
		
		_0x8xy0,
		_0x8xy1,
		_0x8xy2,
		_0x8xy3,
		_0x8xy4,
		_0x8xy5,
		_0x8xy6,
		_0x8xy7,
		_0x8xyE,
		
		_0x9xy0,
		_0xAnnn,
		_0xBnnn,
		_0xCxkk,
		_0xDxyn,
		
		_0xExA1,
		_0xEx9E,
		
		_0xFx07,
		_0xFx0A,
		_0xFx15,
		_0xFx18,
		_0xFx1E,
		_0xFx29,
		_0xFx33,
		_0xFx55,
		_0xFx65,
		
		INVALID,
	};
	
	// clang-format off
	static constexpr std::size_t nInstructions = 34;
	static constexpr std::array<std::string_view, nInstructions> instructionSetIds = {{
		"0x00E0",
		"0x00EE",
		
		"0x1nnn",
		"0x2nnn",
		"0x3xkk",
		"0x4xkk",
		"0x5xy0",
		"0x6xkk",
		"0x7xkk",
		
		"0x8xy0",
		"0x8xy1",
		"0x8xy2",
		"0x8xy3",
		"0x8xy4",
		"0x8xy5",
		"0x8xy6",
		"0x8xy7",
		"0x8xyE",
		
		"0x9xy0",
		"0xAnnn",
		"0xBnnn",
		"0xCxkk",
		"0xDxyn",
		
		"0xExA1",
		"0xEx9E",
		
		"0xFx07",
		"0xFx0A",
		"0xFx15",
		"0xFx18",
		"0xFx1E",
		"0xFx29",
		"0xFx33",
		"0xFx55",
		"0xFx65",
	}};
	// clang-format on
	static constexpr inline std::string_view ToString(const Instruction instruction)
	{
		return instructionSetIds[static_cast<size_t>(instruction)];
	}
	
	namespace detail
	{
		static constexpr std::array<Instruction, 14> _uniquePatternInstructions = { {
			Instruction::INVALID,
			Instruction::_0x1nnn,
			Instruction::_0x2nnn,
			Instruction::_0x3xkk,
			Instruction::_0x4xkk,
			Instruction::_0x5xy0,
			Instruction::_0x6xkk,
			Instruction::_0x7xkk,
			Instruction::INVALID,
			Instruction::_0x9xy0,
			Instruction::_0xAnnn,
			Instruction::_0xBnnn,
			Instruction::_0xCxkk,
			Instruction::_0xDxyn,
		} };

		static constexpr std::array<Instruction, 15> _0x80xyInstructions = { {
			Instruction::_0x8xy0,
			Instruction::_0x8xy1,
			Instruction::_0x8xy2,
			Instruction::_0x8xy3,
			Instruction::_0x8xy4,
			Instruction::_0x8xy5,
			Instruction::_0x8xy6,
			Instruction::_0x8xy7,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::_0x8xyE,
		} };

		static constexpr std::array<Instruction, 15> _0x00EkInstructions = { {
			Instruction::_0x00E0,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::_0x00EE,
		} };

		static constexpr std::array<Instruction, 15> _0xExyzInstructions = { {
			Instruction::INVALID,
			Instruction::_0xExA1,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::_0xEx9E,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
			Instruction::INVALID,
		} };


		static constexpr std::array<Instruction, 0x66> Get0xFxyzInstructionSet()
		{
			std::array<Instruction, 0x66> ret {};
			utils::constexprFor<0, 0x66>([&](const auto i) { ret[i] = Instruction::INVALID; });

			ret[0x07] = Instruction::_0xFx07;
			ret[0x0A] = Instruction::_0xFx0A;
			ret[0x15] = Instruction::_0xFx15;
			ret[0x18] = Instruction::_0xFx18;
			ret[0x1E] = Instruction::_0xFx1E;
			ret[0x29] = Instruction::_0xFx29;
			ret[0x33] = Instruction::_0xFx33;
			ret[0x55] = Instruction::_0xFx55;
			ret[0x65] = Instruction::_0xFx65;

			return ret;
		}
		static inline constexpr std::array<Instruction, 0x66> _0xFxyzInstructions = Get0xFxyzInstructionSet();
	}	 // namespace detail
}	 // namespace emu
