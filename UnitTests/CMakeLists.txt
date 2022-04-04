
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
		spanTests
	SOURCES
		SpanTests.cpp
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

# Test Roms from:
# https://github.com/corax89/chip8-test-rom
# https://github.com/metteo/chip8-test-rom
# https://github.com/Skosulor/c8int/tree/master/test
# https://github.com/cbreezier/rchip8/blob/master/roms/
# https://github.com/metteo/chip8-test-rom/blob/master/SCTEST.CH8
set_tests_properties(chip8Tests.test PROPERTIES ENVIRONMENT "DATA_PATH=${CMAKE_BINARY_DIR}")
add_custom_command(TARGET chip8Tests POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/UnitTests/Data/test_opcode.ch8 ${CMAKE_BINARY_DIR}/test_opcode.ch8
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/UnitTests/Data/test-rom.ch8 ${CMAKE_BINARY_DIR}/test-rom.ch8
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/UnitTests/Data/c8_test.c8 ${CMAKE_BINARY_DIR}/c8_test.c8
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/UnitTests/Data/ibm_logo.ch8 ${CMAKE_BINARY_DIR}/ibm_logo.ch8
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/UnitTests/Data/BC_test.ch8 ${CMAKE_BINARY_DIR}/BC_test.ch8
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/UnitTests/Data/SCTEST.CH8 ${CMAKE_BINARY_DIR}/SCTEST.CH8
)
