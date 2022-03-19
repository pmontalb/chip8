
#include <gtest/gtest.h>
#include "Emulator/Display.h"

class DisplayTests: public ::testing::Test
{
};

TEST_F(DisplayTests, ExpectsWidth64)
{
	emu::Display display;
	ASSERT_EQ(display.GetWidth(), 64);
}
TEST_F(DisplayTests, ExpectsHeight32)
{
	emu::Display display;
	ASSERT_EQ(display.GetHeight(), 32);
}
TEST_F(DisplayTests, HasNotChangedAtStartup)
{
	emu::Display display;
	ASSERT_FALSE(display.HasChanged());
}
TEST_F(DisplayTests, TurnedOffAtStartup)
{
	emu::Display display;
	for (size_t i = 0; i < display.GetHeight() * display.GetWidth(); ++i)
		ASSERT_FALSE(display.GetAt(i));
}
TEST_F(DisplayTests, FlipAt)
{
	emu::Display display;
	for (size_t i = 24; i < 128; ++i)
		display.FlipAt(i);
	for (size_t i = 0; i < display.GetHeight() * display.GetWidth(); ++i)
	{
		if (i >= 24 && i < 128)
			ASSERT_TRUE(display.GetAt(i));
		else
			ASSERT_FALSE(display.GetAt(i));
	}
	ASSERT_TRUE(display.HasChanged());
}
TEST_F(DisplayTests, Clear)
{
	emu::Display display;
	for (size_t i = 24; i < 128; ++i)
		display.FlipAt(i);
	display.Clear();
	for (size_t i = 0; i < display.GetHeight() * display.GetWidth(); ++i)
	{
		ASSERT_FALSE(display.GetAt(i));
	}
	ASSERT_TRUE(display.HasChanged());
}
TEST_F(DisplayTests, Reset)
{
	emu::Display display;
	for (size_t i = 24; i < 128; ++i)
		display.FlipAt(i);
	ASSERT_TRUE(display.HasChanged());
	display.Clear();
	ASSERT_TRUE(display.HasChanged());
	display.Reset();
	ASSERT_FALSE(display.HasChanged());
}
