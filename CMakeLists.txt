
include_guard()

cmake_minimum_required(VERSION 3.14)
project(chip8)

include(${CMAKE_SOURCE_DIR}/cmake/All.cmake)

include(FetchContent)
FetchContent_Declare(mahi-gui GIT_REPOSITORY https://github.com/mahilab/mahi-gui.git)
FetchContent_MakeAvailable(mahi-gui)

add_subdirectory(Emulator)
add_subdirectory(UnitTests)

#create_executable(
#	NAME
#		main
#	SOURCES
#		main.cpp
#	DEPENDENCIES
#		mahi::gui
#)
