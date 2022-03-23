
create_library(
	NAME
		Cpu
	SOURCES
		Cpu.cpp
	DEPENDENCIES
		spdlog fmt::fmt
)
create_library(
	NAME
		Ram
	SOURCES
		Ram.cpp
	DEPENDENCIES
		spdlog fmt::fmt
)
create_library(
	NAME
		Display
	SOURCES
		Display.cpp
	DEPENDENCIES
		spdlog fmt::fmt
)
create_library(
	NAME
		Keypad
	SOURCES
		Keypad.cpp
	DEPENDENCIES
		spdlog fmt::fmt
)

create_library(
	NAME
		Rng
	SOURCES
		Rng.cpp
	DEPENDENCIES
		spdlog fmt::fmt
)

create_library(
	NAME
		Chip8
	SOURCES
		Chip8.cpp
	DEPENDENCIES
		Cpu Ram Display Keypad Rng
)
