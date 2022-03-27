
create_library(
	NAME
		testMain
	SOURCES
		main.cpp
	SYSTEM_DEPENDENCIES
		gtest pthread
)

create_test(
	NAME
		ringBufferTests
	SOURCES
		RingBufferTests.cpp
	DEPENDENCIES
		testMain
)

create_test(
	NAME
		utilityTests
	SOURCES
		UtilityTests.cpp
	DEPENDENCIES
		testMain
)

create_test(
	NAME
		keypadTests
	SOURCES
		KeypadTests.cpp
	DEPENDENCIES
		testMain Keypad
)

create_test(
	NAME
		ramTests
	SOURCES
		RamTests.cpp
	DEPENDENCIES
		testMain Ram
)

create_test(
	NAME
		rngTests
	SOURCES
		RngTests.cpp
	DEPENDENCIES
		testMain Rng
)

create_test(
	NAME
		displayTests
	SOURCES
		DisplayTests.cpp
	DEPENDENCIES
		testMain Display
)

create_test(
	NAME
		cpuTests
	SOURCES
		CpuTests.cpp
	DEPENDENCIES
		testMain Cpu
)

create_test(
	NAME
		chip8Tests
	SOURCES
		Chip8Tests.cpp
	DEPENDENCIES
		testMain Cpu Display Ram Rng Keypad
)

