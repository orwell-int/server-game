# Tests' CMakeLists

message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

# Get together all the source files and all the headers
file(GLOB SOURCES Common.cpp)
file(GLOB EXPORTABLEHEADERS *.hpp *.h)

# Required libraries
find_package(Boost REQUIRED COMPONENTS system)
find_package(Protobuf REQUIRED)
find_package(ZMQ REQUIRED)

find_package(LOG4CXX_LIBRARIES REQUIRED)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/messages
	${LOG4CXX_INCLUDE_DIR}
	)


# Library compilation and linking
add_library(orwell_test SHARED ${SOURCES})
target_link_libraries(
	orwell_test
	orwell_server_game
	${ZMQ_LIBRARIES}
	${Boost_LIBRARIES}
	${LOG4CXX_LIBRARIES})

#add_executable(test_registerRobot "register_robot.cpp")
#target_link_libraries(test_registerRobot orwell_com orwell_tasks)
#add_test(test_registerRobot ${CMAKE_CURRENT_BINARY_DIR}/test_registerRobot)

include_directories(
	${ZMQ_INCLUDE_DIRS}
	${LOG4CXX_INCLUDE_DIR}
	${PROTOBUF_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/include/orwell
	${CMAKE_SOURCE_DIR}/include/orwell/callbacks
	${CMAKE_SOURCE_DIR}/include/orwell/com
	${CMAKE_SOURCE_DIR}/include/orwell/game
	${CMAKE_BINARY_DIR}/messages
	)

add_executable(test_input "input_to_robot.cpp")
target_link_libraries(
	test_input
	orwell_server_game
	orwell_test)
add_test(test_input ${CMAKE_CURRENT_BINARY_DIR}/test_input)

add_executable(test_hello "hello_client.cpp")
target_link_libraries(
	test_hello
	orwell_server_game
	orwell_test)
add_test(test_hello ${CMAKE_CURRENT_BINARY_DIR}/test_hello)

add_executable(test_application_errors "application_errors.cpp")
target_link_libraries(
	test_application_errors
	orwell_server_game
	orwell_test)
add_test(test_application_errors ${CMAKE_CURRENT_BINARY_DIR}/test_application_errors)

add_definitions( "-D__HYPER_BLASTER__" )
add_executable(broadcast_test "broadcast_test.cpp")
target_link_libraries(
	broadcast_test
	orwell_server_game
	orwell_test)
add_test(broadcast_test ${CMAKE_CURRENT_BINARY_DIR}/broadcast_test)

add_executable(test_agent_proxy "agent_proxy.cpp")
target_link_libraries(
	test_agent_proxy
	orwell_server_game
	orwell_test)
add_test(test_agent_proxy ${CMAKE_CURRENT_BINARY_DIR}/test_agent_proxy)

get_property(VAR_ALL_TEST_TARGETS GLOBAL PROPERTY P_ALL_TEST_TARGETS)
ADD_CUSTOM_TARGET(ctest
	COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ctest
	DEPENDS ${VAR_ALL_TEST_TARGETS}
)

SET(DART_TESTING_TIMEOUT "7")
SET(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-reachable=yes")

# this might be a hack but it works
CONFIGURE_FILE(
	${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
	${CMAKE_CURRENT_BINARY_DIR}/DartConfiguration.tcl
	)
