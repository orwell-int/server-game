# Root's CMakeFile
cmake_minimum_required(VERSION 2.8.10)

# Use our own modules along with the CMake's ones
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

project(orwell)

# Communication module, containing Protobuf messages and simple library to play with them
add_subdirectory(com)

