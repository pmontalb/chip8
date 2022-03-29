
#pragma once

namespace emu
{
	enum class Error
	{
		None,
		InvalidInstruction,
		InvalidProgramCounter,
	};

	template<typename OStream>
	OStream& operator<<(OStream& os, const Error error)
	{
		switch (error)
		{
			case Error::None:
				os << "None";
				break;
			case Error::InvalidInstruction:
				os << "Invalid instruction";
				break;
			case Error::InvalidProgramCounter:
				os << "Invalid program counter";
				break;
			default:
				os << "?";
				break;
		}
		return os;
	}
}	 // namespace emu
