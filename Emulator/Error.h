
#pragma once

namespace emu
{
	namespace Error
	{
		enum Enum
		{
			START = 0,
			None = START,
			InvalidInstruction,
			InvalidProgramCounter,
			END,
		};
	}	 // namespace Error

	static inline std::string_view ToString(Error::Enum error)
	{
		switch (error)
		{
			case Error::None:
				return "None";
			case Error::InvalidInstruction:
				return "Invalid instruction";
			case Error::InvalidProgramCounter:
				return "Invalid program counter";
			default:
				return "?";
		}
	}

	template<typename OStream>
	OStream& operator<<(OStream& os, const Error::Enum error)
	{
		return os << ToString(error);
	}
}	 // namespace emu
