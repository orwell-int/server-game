# Tests' CMakeLists

message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

include_directories(
	${CMAKE_SOURCE_DIR}/com/include
	${CMAKE_SOURCE_DIR}/tasks/include
	${CMAKE_BINARY_DIR}/messages
	)

# Test suite
enable_testing()

add_executable(test_registerRobot "register_robot.cpp")
target_link_libraries(test_registerRobot orwell_com orwell_tasks)
add_test(test_registerRobot ${CMAKE_CURRENT_BINARY_DIR}/test_registerRobot)

add_executable(test_input "input_to_robot.cpp")
target_link_libraries(test_input orwell_com orwell_tasks)
add_test(test_input ${CMAKE_CURRENT_BINARY_DIR}/test_input)

add_executable(test_hello "hello_client.cpp")
target_link_libraries(test_hello orwell_com orwell_tasks)
add_test(test_hello ${CMAKE_CURRENT_BINARY_DIR}/test_hello)
