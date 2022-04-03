
#include <gtest/gtest.h>
#include "Emulator/Keypad.h"

class KeypadTests: public ::testing::Test
{
};

TEST_F(KeypadTests, Expects16Keys)
{
	emu::Keypad keypad;
	ASSERT_EQ(keypad.GetSize(), 16);
}

TEST_F(KeypadTests, NoKeyPressAtInitialization)
{
	emu::Keypad keypad;
	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
		ASSERT_FALSE(keypad.IsPressed(static_cast<emu::Keys::Enum>(key)));
}

TEST_F(KeypadTests, Press)
{
	emu::Keypad keypad;
	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
	{
		auto keyCode = static_cast<emu::Keys::Enum>(key);
		keypad.Press(keyCode, true);
		ASSERT_TRUE(keypad.IsPressed(keyCode));
		for (size_t key2 = emu::Keys::START; key2 < emu::Keys::END; ++key2)
		{
			if (key == key2)
				continue;
			ASSERT_FALSE(keypad.IsPressed(static_cast<emu::Keys::Enum>(key2))) << "key=" << key << " | key2=" << key2;
		}
		keypad.Press(keyCode, false);
	}
}

TEST_F(KeypadTests, Release)
{
	emu::Keypad keypad;
	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
	{
		auto keyCode = static_cast<emu::Keys::Enum>(key);
		keypad.Press(keyCode, true);
		ASSERT_TRUE(keypad.IsPressed(keyCode));
	}

	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
	{
		auto keyCode = static_cast<emu::Keys::Enum>(key);
		keypad.Press(keyCode, false);
		ASSERT_FALSE(keypad.IsPressed(keyCode));
		for (size_t key2 = emu::Keys::START; key2 < emu::Keys::END; ++key2)
		{
			if (key == key2)
				continue;
			ASSERT_TRUE(keypad.IsPressed(static_cast<emu::Keys::Enum>(key2)));
		}
		keypad.Press(keyCode, true);
	}
}

TEST_F(KeypadTests, Serialize)
{
	emu::Keypad keypad;
	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
	{
		auto keyCode = static_cast<emu::Keys::Enum>(key);
		keypad.Press(keyCode, key % 3 == 0);
	}

	std::vector<emu::Byte> byteArray;
	keypad.Serialize(byteArray);

	emu::Keypad keypad2;
	keypad2.Deserialize(byteArray);

	for (size_t key = emu::Keys::START; key < emu::Keys::END; ++key)
	{
		auto keyCode = static_cast<emu::Keys::Enum>(key);
		ASSERT_EQ(keypad.IsPressed(keyCode), keypad2.IsPressed(keyCode));
	}
}
