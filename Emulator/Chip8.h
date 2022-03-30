
#pragma once

#include "Cpu.h"
#include "Display.h"
#include "Keypad.h"
#include "Ram.h"
#include "Rng.h"

#include "Error.h"

#include "InstructionSet.h"

#include <filesystem>

// https://austinmorlan.com/posts/chip8_emulator/
namespace emu
{
	namespace detail
	{
		template<typename CpuT, typename RngT, typename RamT, typename DisplayT, typename KeypadT>
		class Chip8
		{
		public:
			Chip8();
			bool LoadRom(const std::filesystem::path& path);

			bool Cycle();
			void Rewind();
			Instruction GetLastExecutedInstruction() const { return _lastExecutedInstruction; }

			const auto& GetDisplay() const { return _display; }
			const auto& GetCpu() const { return _cpu; }
			const auto& GetRam() const { return _ram; }
			auto& GetKeypad() { return _keypad; }

			[[nodiscard]] bool IsValid() const { return _lastError == Error::None; }
			[[nodiscard]] auto GetLastError() const { return _lastError; }

		protected:
			TwoBytes FetchInstruction();
			bool ExecuteInstruction(const TwoBytes instruction);

		private:
			void Initialize();
			void PopulateInstructionSetFunctionPointers();

		protected:
			CpuT _cpu {};
			RngT _rng {};
			RamT _ram {};
			DisplayT _display {};
			KeypadT _keypad {};

		private:
			Instruction _lastExecutedInstruction = Instruction::INVALID;
			using InstructionSetWorker = std::function<void(const TwoBytes)>;
			using InstructionPair = std::pair<Instruction, InstructionSetWorker>;

			std::array<InstructionPair, detail::_uniquePatternInstructions.size()> _uniquePatternInstructions{};
			std::array<InstructionPair, detail::_0x80xyInstructions.size()> _0x80xyInstructions{};
			std::array<InstructionPair, detail::_0x00EkInstructions.size()> _0x00EkInstructions{};
			std::array<InstructionPair, detail::_0xExyzInstructions.size()> _0xExyzInstructions{};
			std::array<InstructionPair, detail::_0xFxyzInstructions.size()> _0xFxyzInstructions{};

			static constexpr size_t functionTableSize = 0x10;
			std::array<InstructionPair, functionTableSize> _instructionSet{};

			Error _lastError = Error::None;
		};
	}	 // namespace detail

	using Chip8 = detail::Chip8<Cpu, Rng, Ram, Display, Keypad>;
}	 // namespace emu

#include "Chip8.tpp"
