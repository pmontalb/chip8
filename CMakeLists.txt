
include_guard()

cmake_minimum_required(VERSION 3.14)
project(chip8)

include(${CMAKE_SOURCE_DIR}/cmake/All.cmake)

add_compile_definitions($<$<CONFIG:DEBUG>:SPDLOG_ACTIVE_LEVEL=0>)

# external packages
add_subdirectory(Submodules EXCLUDE_FROM_ALL)

add_subdirectory(Emulator)
add_subdirectory(UnitTests)

create_executable(
	NAME
		chip8
	SOURCES
		main.cpp
	DEPENDENCIES
		mahi::gui Cpu Display Ram Rng Keypad
)
