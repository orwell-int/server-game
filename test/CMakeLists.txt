# Tests' CMakeLists

message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

# Get together all the source files and all the headers
file(GLOB SOURCES Common.cpp ItemCommon.cpp)
file(GLOB EXPORTABLEHEADERS *.hpp *.h)
file(GLOB CONFIG ${CMAKE_SOURCE_DIR}/*.ini)
file(GLOB OPENCV_IMAGES ${CMAKE_SOURCE_DIR}/test/*.jpg)

# Required libraries
find_package(Boost REQUIRED COMPONENTS system)
find_package(Protobuf REQUIRED)
find_package(ZMQ REQUIRED)

find_package(LOG4CXX_LIBRARIES REQUIRED)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/messages
	${LOG4CXX_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
	${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
	${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/include
	)

# Library compilation and linking
add_library(orwell_test SHARED ${SOURCES})
target_link_libraries(
	orwell_test
	PUBLIC
	orwell_server_game
	gtest_main
	gmock_main
	${ZMQ_LIBRARIES}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES}
	${Boost_LIBRARIES}
	${LOG4CXX_LIBRARIES}
	${OpenCV_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
	PRIVATE
	nlohmann_json::nlohmann_json)


include_directories(
	${ZMQ_INCLUDE_DIRS}
	${GMOCK_INCLUDE_DIRS}
	${GTEST_INCLUDE_DIRS}
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

FUNCTION(add_orwell_test test_filename)
	string(FIND ${test_filename} ".cpp" pos REVERSE)
	string(SUBSTRING ${test_filename} 0 ${pos} test_name)
	add_executable(${test_name} ${test_filename})
	target_link_libraries(
		${test_name}
		orwell_server_game
		orwell_test)
	cotire(${test_name})
	add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
	foreach(configFile ${CONFIG})
		add_custom_command(TARGET ${test_name} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${configFile} ${CMAKE_CURRENT_BINARY_DIR})
	endforeach(configFile)
ENDFUNCTION()

FUNCTION(add_google_test test_filename)
	string(FIND ${test_filename} ".cpp" pos REVERSE)
	string(SUBSTRING ${test_filename} 0 ${pos} test_name)
	add_executable(${test_name} ${test_filename})
	target_link_libraries(
		${test_name}
		gtest)
		#gtest_main)
	target_link_libraries(
		${test_name}
		orwell_server_game
		orwell_test)
	cotire(${test_name})
	add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
	foreach(configFile ${CONFIG})
		add_custom_command(TARGET ${test_name} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${configFile} ${CMAKE_CURRENT_BINARY_DIR})
	endforeach(configFile)
ENDFUNCTION()

add_orwell_test(test_ping_pong.cpp)
add_orwell_test(test_input_to_robot.cpp)
add_orwell_test(test_hello_client.cpp)
add_orwell_test(test_application.cpp)
# this is not nice, but used to disable the test when running with Travis CI.
if (DEFINED ENV{ORWELL_DISABLE_TEST_BROADCAST})
else()
	add_orwell_test(test_broadcast.cpp)
endif()
add_google_test(test_agent_proxy.cpp)
add_google_test(test_agent_proxy_json.cpp)
add_orwell_test(test_agent.cpp)
add_orwell_test(test_register_robot.cpp)
add_orwell_test(test_start_game_condition.cpp)
add_google_test(test_socket.cpp)
add_orwell_test(test_input.cpp)
add_orwell_test(test_main.cpp)
add_orwell_test(test_main_plus.cpp)
add_google_test(test_opencv.cpp)
add_orwell_test(test_capture_one_flag.cpp)
add_orwell_test(test_capture_multiflag.cpp)
add_orwell_test(test_stop_after_game_duration.cpp)
add_google_test(test_orwell_game_Clock.cpp)
add_google_test(test_orwell_game_Robot.cpp)
add_google_test(test_orwell_game_Game.cpp)
add_google_test(test_orwell_game_Item.cpp)
add_google_test(test_orwell_game_item_Flag.cpp)
add_google_test(test_orwell_game_item_FlagDetector.cpp)
add_google_test(test_orwell_game_ItemEncoder.cpp)
add_dependencies(test_main server_main)
add_dependencies(test_main_plus server_main)
foreach(image ${OPENCV_IMAGES})
	MESSAGE("Copy OPENCV_IMAGES (test) from ${OPENCV_IMAGES} to ${CMAKE_CURRENT_BINARY_DIR}")
	add_custom_command(TARGET test_opencv COMMAND ${CMAKE_COMMAND} -E copy_if_different ${image} ${CMAKE_CURRENT_BINARY_DIR})
endforeach(image)

get_property(VAR_ALL_TEST_TARGETS GLOBAL PROPERTY P_ALL_TEST_TARGETS)
ADD_CUSTOM_TARGET(ctest
	COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ctest
	DEPENDS ${VAR_ALL_TEST_TARGETS}
)

SET(DART_TESTING_TIMEOUT "6")
SET(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-reachable=yes")

# this might be a hack but it works
CONFIGURE_FILE(
	${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
	${CMAKE_CURRENT_BINARY_DIR}/DartConfiguration.tcl
	)
