# Root's CMakeFile
cmake_minimum_required(VERSION 2.8.10)

# Use our own modules along with the CMake's ones
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

project(orwell)

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

# Messages
add_subdirectory(../messages "${CMAKE_CURRENT_BINARY_DIR}/messages")

# Communication module
add_subdirectory(com)

# Tasks module
add_subdirectory(tasks)

# Tests module
add_subdirectory(test)


include_directories(
		${CMAKE_CURRENT_BINARY_DIR}/../com/include
		${CMAKE_CURRENT_BINARY_DIR}/../tasks/include
		${CMAKE_CURRENT_BINARY_DIR}/messages
		${ZMQ_INCLUDE_DIRS}
		${PROTOBUF_INCLUDE_DIRS}
		${Boost_INCLUDE_DIRS})
add_executable(server_main main.cpp)
target_link_libraries(server_main orwell_com orwell_tasks)


