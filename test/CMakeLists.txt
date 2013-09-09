# Tests' CMakeLists

include_directories(
		${CMAKE_CURRENT_BINARY_DIR}/../com
		${CMAKE_CURRENT_BINARY_DIR}/../tasks
		${CMAKE_CURRENT_BINARY_DIR}/../messages
		${PROTOBUF_INCLUDE_DIRS}
		${ZMQ_INCLUDE_DIRS}
		${CMAKE_BINARY_DIR}/com/
		${CMAKE_SOURCE_DIR}/com/include
		${Boost_INCLUDE_DIRS}
		${LOG4CXX_INCLUDE_DIR})

# Test suite
enable_testing()

add_executable(test_registerRobot "register_robot.cpp")
target_link_libraries(test_registerRobot orwell_com orwell_tasks)
add_test(test_registerRobot ${CMAKE_CURRENT_BINARY_DIR}/test_basics)

add_executable(test_input "input_to_robot.cpp")
target_link_libraries(test_input orwell_com orwell_tasks)
add_test(test_input ${CMAKE_CURRENT_BINARY_DIR}/test_basics)

