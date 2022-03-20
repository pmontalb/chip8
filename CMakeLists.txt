
include_guard()

cmake_minimum_required(VERSION 3.14)
project(chip8)

include(${CMAKE_SOURCE_DIR}/cmake/All.cmake)

# external packages
include(FetchContent)
FetchContent_Declare(mahi-gui GIT_REPOSITORY https://github.com/mahilab/mahi-gui.git)
FetchContent_MakeAvailable(mahi-gui)

include_directories(spdlog/include)
###

add_compile_definitions($<$<CONFIG:DEBUG>:SPDLOG_ACTIVE_LEVEL=0>)

add_subdirectory(Emulator)
add_subdirectory(UnitTests)

create_executable(
	NAME
		chip8
	SOURCES
		main.cpp
	DEPENDENCIES
		mahi::gui
)
