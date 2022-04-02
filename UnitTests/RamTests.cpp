
#include <gtest/gtest.h>
#include "Emulator/Ram.h"

class RamTests: public ::testing::Test
{
};

TEST_F(RamTests, Expects4kRam)
{
	emu::Ram ram;
	ASSERT_EQ(ram.GetSize(), 4096);
}

TEST_F(RamTests, ZeroInitializationAtStartup)
{
	emu::Ram ram;
	for (size_t i = 0; i < ram.GetSize(); ++i)
	{
		if (i >= emu::fontsOffset && i < emu::endFonts)
		{
			ASSERT_EQ(ram.GetAt(i), emu::fonts[i - emu::fontsOffset]);
		}
		else
		{
			ASSERT_EQ(ram.GetAt(i), 0) << i;
		}

	}
}

TEST_F(RamTests, GetFirstCharOfFont)
{
	emu::Ram ram;
	for (size_t i = 0; i < (emu::endFonts - emu::fontsOffset) / emu::fontSize; ++i)
	{
		ASSERT_EQ(ram.GetFontAt(i), emu::fonts[i * emu::fontSize]) << i;
	}
}

TEST_F(RamTests, SetRam)
{
	emu::Ram ram;
	size_t nonZeroIdx = ram.GetInstructionStartAddress() + 127;
	emu::Byte ramValue = 42;
	ram.SetAt(nonZeroIdx, ramValue);
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
	{
		if (i == nonZeroIdx)
			ASSERT_EQ(ram.GetAt(i), ramValue);
		else
			ASSERT_EQ(ram.GetAt(i), 0) << i;
	}
}

TEST_F(RamTests, CopyFrom)
{
	std::vector<emu::Byte> dynamicMemory(512);
	std::fill(dynamicMemory.begin(), dynamicMemory.end(), 42);

	emu::Ram ram;
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ram.SetAt(i, 24);

	// verify ram values are not changed
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ASSERT_EQ(ram.GetAt(i), 24);

	ram.CopyFrom(ram.GetInstructionStartAddress() + 10, dynamicMemory.data(), dynamicMemory.size());

	for (size_t i = ram.GetInstructionStartAddress() + 10; i < ram.GetSize(); ++i)
	{
		if (i - (ram.GetInstructionStartAddress() + 10) < dynamicMemory.size())
		{
			ASSERT_EQ(ram.GetAt(i), dynamicMemory[i - (ram.GetInstructionStartAddress() + 10)]);
			ASSERT_NE(ram.GetAt(i), 24);
		}
		else
			ASSERT_EQ(ram.GetAt(i), 24);
	}
	// verify dynamic values are not changed
	for (size_t i = 0; i < dynamicMemory.size(); ++i)
		ASSERT_EQ(dynamicMemory[i], 42);

	std::array<emu::Byte, 512> staticMemory {};
	std::fill(staticMemory.begin(), staticMemory.end(), 42);
	ram.CopyFrom(ram.GetInstructionStartAddress() + 10, staticMemory.data(), staticMemory.size() / 2);
	for (size_t i = ram.GetInstructionStartAddress() + 10; i < ram.GetSize(); ++i)
	{
		if (i - (ram.GetInstructionStartAddress() + 10) < dynamicMemory.size())
		{
			ASSERT_EQ(ram.GetAt(i), dynamicMemory[i - (ram.GetInstructionStartAddress() + 10)]);
			ASSERT_NE(ram.GetAt(i), 24);
		}
		else
			ASSERT_EQ(ram.GetAt(i), 24);
	}

	// verify static values are not changed
	for (size_t i = 0; i < staticMemory.size(); ++i)
		ASSERT_EQ(staticMemory[i], 42);
}

TEST_F(RamTests, WriteTo)
{
	std::vector<emu::Byte> dynamicMemory(512);
	std::fill(dynamicMemory.begin(), dynamicMemory.end(), 42);

	emu::Ram ram;
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ram.SetAt(i, 24);

	// verify ram values are not changed
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ASSERT_EQ(ram.GetAt(i), 24);

	ram.WriteTo(ram.GetInstructionStartAddress() + 10, dynamicMemory.data(), dynamicMemory.size() / 2);

	for (size_t i = 0; i < dynamicMemory.size(); ++i)
	{
		if (i >= dynamicMemory.size() / 2)
		{
			// not overridden from ram
			ASSERT_EQ(dynamicMemory[i], 42);
			ASSERT_NE(dynamicMemory[i], ram.GetAt(i + 10));
		}
		else
		{
			// overridden from ram
			ASSERT_EQ(dynamicMemory[i], ram.GetAt(i + ram.GetInstructionStartAddress() + 10)) << i;
		}
	}
	// verify ram values are not changed
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ASSERT_EQ(ram.GetAt(i), 24);

	std::array<emu::Byte, 512> staticMemory {};
	std::fill(staticMemory.begin(), staticMemory.end(), 42);
	ram.WriteTo(ram.GetInstructionStartAddress() + 10, staticMemory.data(), staticMemory.size() / 2);
	for (size_t i = 0; i < staticMemory.size(); ++i)
	{
		if (i >= dynamicMemory.size() / 2)
		{
			// not overridden from ram
			ASSERT_EQ(staticMemory[i], 42);
			ASSERT_NE(staticMemory[i], ram.GetAt(i + 10));
		}
		else
		{
			// overridden from ram
			ASSERT_EQ(staticMemory[i], ram.GetAt(i + ram.GetInstructionStartAddress() + 10)) << i;
		}
	}

	// verify ram values are not changed
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ASSERT_EQ(ram.GetAt(i), 24);
}

TEST_F(RamTests, Load)
{
	emu::Ram ram;

	std::string buffer;
	buffer.resize(5);
	buffer[0] = 0x48;
	buffer[1] = 0x65;
	buffer[2] = 0x6C;
	buffer[3] = 0x6C;
	buffer[4] = 0x6F;
	ram.Load(buffer);

	for (size_t i = ram.GetInstructionStartAddress(); i - ram.GetInstructionStartAddress() < buffer.size(); ++i)
		ASSERT_EQ(ram.GetAt(i), buffer[i - ram.GetInstructionStartAddress()]);
	for (size_t i = ram.GetInstructionStartAddress() + buffer.size(); i < ram.GetSize(); ++i)
		ASSERT_EQ(ram.GetAt(i), 0);
}

TEST_F(RamTests, Serialize)
{
	emu::Ram ram;
	for (size_t i = ram.GetInstructionStartAddress(); i < ram.GetSize(); ++i)
		ram.SetAt(i, static_cast<emu::Byte>(i * i + 1));

	std::vector<emu::Byte> bytes;
	ram.Serialize(bytes);

	emu::Ram ram2;
	ram2.Deserialize(bytes);
	for (size_t i = 0; i < ram.GetSize(); ++i)
		ASSERT_EQ(ram.GetAt(i), ram2.GetAt(i));
}

