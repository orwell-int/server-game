# Tests' CMakeLists

message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

include_directories(
	${CMAKE_SOURCE_DIR}/com/include
	${CMAKE_SOURCE_DIR}/tasks/include
	${CMAKE_BINARY_DIR}/messages
	)

# Get together all the source files and all the headers
file(GLOB SOURCES Common.cpp)
file(GLOB EXPORTABLEHEADERS *.hpp *.h)

# Required libraries
find_package(Boost REQUIRED COMPONENTS system)
find_package(Protobuf REQUIRED)
find_package(ZMQ REQUIRED)

find_package(LOG4CXX_LIBRARIES REQUIRED)


# Library compilation and linking
add_library(orwell_test SHARED ${SOURCES})
target_link_libraries(orwell_test
		orwell_com
		${ZMQ_LIBRARIES}
		${Boost_LIBRARIES}
		${LOG4CXX_LIBRARIES})

# Test suite
enable_testing()

#add_executable(test_registerRobot "register_robot.cpp")
#target_link_libraries(test_registerRobot orwell_com orwell_tasks)
#add_test(test_registerRobot ${CMAKE_CURRENT_BINARY_DIR}/test_registerRobot)

add_executable(test_input "input_to_robot.cpp")
target_link_libraries(test_input orwell_com orwell_tasks orwell_test)
add_test(test_input ${CMAKE_CURRENT_BINARY_DIR}/test_input)

add_executable(test_hello "hello_client.cpp")
target_link_libraries(test_hello orwell_com orwell_tasks orwell_test)
add_test(test_hello ${CMAKE_CURRENT_BINARY_DIR}/test_hello)
