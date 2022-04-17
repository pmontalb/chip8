
#pragma once

#include "Types.h"
#include "Span.h"

#include <vector>

namespace emu
{
	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;

		// once you define the dtor, clang warns you about deprecated implicit definitions
		ISerializable() = default;
		ISerializable(const ISerializable&) = default;
		ISerializable& operator=(const ISerializable&) = default;

		virtual void Serialize(std::vector<Byte>& byteArray) const = 0;
		virtual utils::Span<Byte> Deserialize(const utils::Span<Byte>& byteView) = 0;
	};
}	 // namespace emu
