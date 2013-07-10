# Root's CMakeFile
cmake_minimum_required(VERSION 2.8.10)

# Use our own modules along with the CMake's ones
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

project(orwell)

# Generate compile_commands.json (http://clang.llvm.org/docs/JSONCompilationDatabase.html).
# It is used by YCM (https://github.com/Valloric/YouCompleteMe).
# You just need to create a symbolic link to the build directory's compile_commands.json file
# to the root of the project to have completion.
set(CMAKE_EXPORT_COMPILE_COMMANDS, "ON")

# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	execute_process(
		COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if (NOT (GCC_VERSION VERSION_EQUAL 4.8))
			message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8.")
		endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		message(WARNING "You're compiling with Clang, not with g++")
else ()
		message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# Communication module, containing Protobuf messages and simple library to play with them
add_subdirectory(com)

# Tasks module
add_subdirectory(tasks)

