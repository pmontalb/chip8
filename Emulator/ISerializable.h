
#pragma once

#include "Types.h"
#include <vector>

namespace emu
{
	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual void Serialize(std::vector<Byte>& byteArray) const = 0;
		virtual void Deserialize(const std::vector<Byte>& byteArray) = 0;
	};
}	 // namespace emu
