
create_library(
	NAME
		Cpu
	SOURCES
		Cpu.cpp
)
create_library(
	NAME
		Ram
	SOURCES
		Ram.cpp
)
create_library(
	NAME
		Display
	SOURCES
		Display.cpp
)
create_library(
	NAME
		Keypad
		SOURCES
	Keypad.cpp
)

create_library(
	NAME
		Rng
	SOURCES
		Rng.cpp
)

create_library(
	NAME
		Chip8
	SOURCES
		Chip8.cpp
	DEPENDENCIES
		Cpu Ram Display Keypad Rng
)
