# Root's CMakeFile
cmake_minimum_required(VERSION 2.8.7)

# Use our own modules along with the CMake's ones
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

project(orwell)

# Test suite
include(CTest)

# Generate compile_commands.json (http://clang.llvm.org/docs/JSONCompilationDatabase.html).
# It is used by YCM (https://github.com/Valloric/YouCompleteMe).
# You just need to create a symbolic link to the build directory's compile_commands.json file
# to the root of the project to have completion.
set(CMAKE_EXPORT_COMPILE_COMMANDS, "ON")

set_property(GLOBAL PROPERTY P_ALL_TARGETS "")
set_property(GLOBAL PROPERTY P_ALL_TEST_TARGETS "")

macro(add_library _target)
	_add_library (${_target} ${ARGN})
	get_property(VAR_ALL_TARGETS GLOBAL PROPERTY P_ALL_TARGETS)
	list(APPEND VAR_ALL_TARGETS ${_target})
	list(REMOVE_DUPLICATES VAR_ALL_TARGETS)
	#MESSAGE("add_library(${_target}) ; VAR_ALL_TARGETS = ${VAR_ALL_TARGETS}")
	set_property(GLOBAL PROPERTY P_ALL_TARGETS "${VAR_ALL_TARGETS}")
endmacro()

macro(add_executable _target)
	_add_executable (${_target} ${ARGN})
	get_property(VAR_ALL_TARGETS GLOBAL PROPERTY P_ALL_TARGETS)
	list(APPEND VAR_ALL_TARGETS ${_target})
	list(REMOVE_DUPLICATES VAR_ALL_TARGETS)
	#MESSAGE("add_executable(${_target}) ; VAR_ALL_TARGETS = ${VAR_ALL_TARGETS}")
	set_property(GLOBAL PROPERTY P_ALL_TARGETS "${VAR_ALL_TARGETS}")
endmacro()

macro(add_test _target)
	_add_test (${_target} ${ARGN})
	get_property(VAR_ALL_TARGETS GLOBAL PROPERTY P_ALL_TARGETS)
	list(APPEND VAR_ALL_TARGETS ${_target})
	list(REMOVE_DUPLICATES VAR_ALL_TARGETS)
	#MESSAGE("add_test(${_target}) ; VAR_ALL_TARGETS = ${VAR_ALL_TARGETS}")
	set_property(GLOBAL PROPERTY P_ALL_TARGETS "${VAR_ALL_TARGETS}")
	get_property(VAR_ALL_TEST_TARGETS GLOBAL PROPERTY P_ALL_TEST_TARGETS)
	list(APPEND VAR_ALL_TEST_TARGETS ${_target})
	list(REMOVE_DUPLICATES VAR_ALL_TEST_TARGETS)
	#MESSAGE("add_test(${_target}) ; VAR_ALL_TEST_TARGETS = ${VAR_ALL_TEST_TARGETS}")
	set_property(GLOBAL PROPERTY P_ALL_TEST_TARGETS "${VAR_ALL_TEST_TARGETS}")
endmacro()

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
		if (GCC_VERSION VERSION_LESS 4.8)
			message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8.")
		endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	message(WARNING "You're compiling with Clang, not with g++")
else ()
	message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# Messages
execute_process(COMMAND ln -fs ${CMAKE_SOURCE_DIR}/CMakeLists_messages.txt ${CMAKE_SOURCE_DIR}/messages/CMakeLists.txt)
#add_subdirectory(../messages "${CMAKE_CURRENT_BINARY_DIR}/messages")
add_subdirectory(messages)

# Tests module
add_subdirectory(test)

message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

# Get together all the source files and all the headers
file(GLOB_RECURSE SOURCES src/*.cpp src/*.cc)
file(GLOB_RECURSE EXPORTABLEHEADERS include/*.hpp include/*.h)
file(GLOB CONFIG *.ini)
file(GLOB IMAGES ${CMAKE_SOURCE_DIR}/test/*.part)
file(GLOB OPENCV_IMAGES ${CMAKE_SOURCE_DIR}/test/*.jpg)

# Required libraries
find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(Protobuf REQUIRED)
find_package(ZMQ REQUIRED)
find_package(LOG4CXX_LIBRARIES REQUIRED)
find_package(OpenCV REQUIRED)

# googletest (+googlemock)
enable_testing()
add_subdirectory(third_party/googletest/)

# Helpful debug stuff?
#message ("-- cmake current bin dir "${CMAKE_CURRENT_BINARY_DIR})
#message ("-- cmake zmq bin dir "${ZMQ_INCLUDE_DIRS})
#message ("-- cmake log bin dir "${LOG4CXX_LIBRARIES})

# Library compilation and linking
add_library(orwell_server_game SHARED ${SOURCES})
target_link_libraries(
	orwell_server_game
	orwell_messages
	${ZMQ_LIBRARIES}
	${PROTOBUF_LIBRARIES}
	${Boost_LIBRARIES}
	${LOG4CXX_LIBRARIES}
	${OpenCV_LIBS})

# Where to install stuff, protobuf messages generated H included
install(TARGETS orwell_server_game LIBRARY DESTINATION lib)
install(FILES ${EXPORTABLEHEADERS} DESTINATION include)
install(FILES ${PROTO_HDRS} DESTINATION include)
foreach(configFile ${CONFIG})
	add_custom_command(TARGET orwell_server_game COMMAND ${CMAKE_COMMAND} -E copy ${configFile} ${CMAKE_BINARY_DIR})
endforeach(configFile)
foreach(image ${IMAGES} ${OPENCV_IMAGES})
	add_custom_command(TARGET orwell_server_game  COMMAND ${CMAKE_COMMAND} -E copy ${image} ${CMAKE_CURRENT_BINARY_DIR})
endforeach(image)


include_directories(
	${ZMQ_INCLUDE_DIRS}
	${LOG4CXX_INCLUDE_DIR}
	${PROTOBUF_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/messages
	)

add_definitions(-DVERSION="0.0.2" -DCODENAME="V1Proto")
add_executable(server_main src/main.cpp)
target_link_libraries(
	server_main
	orwell_server_game
	${ZMQ_LIBRARIES}
	${PROTOBUF_LIBRARIES}
	${Boost_LIBRARIES}
	${LOG4CXX_LIBRARIES}
	${OpenCV_LIBS})


OPTION(ORWELL_COVERAGE "Enables code coverage measurements with gcov/lcov." OFF)

IF(ORWELL_COVERAGE)
	MESSAGE("Code coverage considered")
	IF(NOT IS_SUBPROJECT)
		MESSAGE("Code coverage activated")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
		include(CodeCoverage)
		SET(COVERAGE_INCLUDES
			'${CMAKE_SOURCE_DIR}/*'
			)
		SET(COVERAGE_EXCLUDES
			'${CMAKE_SOURCE_DIR}/test/*'
			)
		get_property(VAR_ALL_TARGETS GLOBAL PROPERTY P_ALL_TARGETS)
		setup_target_for_coverage(
			${PROJECT_NAME}_coverage
			ctest
			${CMAKE_BINARY_DIR}/test/coverage
			test
			${CMAKE_BINARY_DIR}
			"${COVERAGE_INCLUDES}"
			"${COVERAGE_EXCLUDES}"
			"${VAR_ALL_TARGETS}"
			)
	ELSE()
		MESSAGE("Code coverage NOT activated")
	ENDIF()
ELSE()
	MESSAGE("Code coverage not considered")
ENDIF()

SET(DART_TESTING_TIMEOUT "10")
SET(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-reachable=yes")

# this might be a hack but it works
CONFIGURE_FILE(
	${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
	${CMAKE_CURRENT_BINARY_DIR}/DartConfiguration.tcl
	)
