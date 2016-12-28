# Tests' CMakeLists

message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

# Get together all the source files and all the headers
file(GLOB SOURCES Common.cpp ItemCommon.cpp)
file(GLOB EXPORTABLEHEADERS *.hpp *.h)
file(GLOB CONFIG ${CMAKE_SOURCE_DIR}/*.ini)
file(GLOB IMAGES ${CMAKE_SOURCE_DIR}/test/*.part)
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
	)


# Library compilation and linking
add_library(orwell_test SHARED ${SOURCES})
target_link_libraries(
	orwell_test
	orwell_server_game
	gtest_main
	gmock_main
	${ZMQ_LIBRARIES}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES}
	${Boost_LIBRARIES}
	${LOG4CXX_LIBRARIES}
	${OpenCV_LIBS}
	${CMAKE_THREAD_LIBS_INIT})


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

FUNCTION(add_orwell_test test_name file_name)
	add_executable(${test_name} ${file_name})
	target_link_libraries(
		${test_name}
		orwell_server_game
		orwell_test)
	add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
	foreach(configFile ${CONFIG})
		add_custom_command(TARGET ${test_name} COMMAND ${CMAKE_COMMAND} -E copy ${configFile} ${CMAKE_CURRENT_BINARY_DIR})
	endforeach(configFile)
ENDFUNCTION()

add_orwell_test(test_input_to_robot "test_input_to_robot.cpp")
add_orwell_test(test_hello "test_hello_client.cpp")
add_orwell_test(test_application "test_application.cpp")
# this is not nice, but used to disable the test when running with Travis CI.
if (DEFINED ENV{ORWELL_DISABLE_TEST_BROADCAST})
else()
	add_orwell_test(test_broadcast "test_broadcast.cpp")
endif()
add_orwell_test(test_agent_proxy "test_agent_proxy.cpp")
add_orwell_test(test_agent "test_agent.cpp")
add_orwell_test(test_register_robot "test_register_robot.cpp")
add_orwell_test(test_start_game_condition "test_start_game_condition.cpp")
add_orwell_test(test_main "test_main.cpp")
add_orwell_test(test_main_plus "test_main_plus.cpp")
add_orwell_test(test_opencv "test_opencv.cpp")
add_orwell_test(test_capture_one_flag "test_capture_one_flag.cpp")
add_orwell_test(test_capture_multiflag "test_capture_multiflag.cpp")
add_orwell_test(test_stop_after_game_duration "test_stop_after_game_duration.cpp")
add_orwell_test(test_orwell_game_Clock "test_orwell_game_Clock.cpp")
add_orwell_test(test_orwell_game_Robot "test_orwell_game_Robot.cpp")
add_orwell_test(test_orwell_game_Game "test_orwell_game_Game.cpp")
add_orwell_test(test_orwell_game_Item "test_orwell_game_Item.cpp")
add_orwell_test(test_orwell_game_item_Flag "test_orwell_game_item_Flag.cpp")
add_orwell_test(test_orwell_game_item_FlagDetector "test_orwell_game_item_FlagDetector.cpp")
add_orwell_test(test_orwell_game_ItemEncoder "test_orwell_game_ItemEncoder.cpp")
add_dependencies(test_main server_main)
add_dependencies(test_main_plus server_main)
foreach(image ${IMAGES})
	MESSAGE("Copy IMAGES (test) from ${IMAGES} to ${CMAKE_CURRENT_BINARY_DIR}")
	add_custom_command(TARGET test_input_to_robot COMMAND ${CMAKE_COMMAND} -E copy ${image} ${CMAKE_CURRENT_BINARY_DIR})
endforeach(image)
foreach(image ${OPENCV_IMAGES})
	MESSAGE("Copy IMAGES (test) from ${OPENCV_IMAGES} to ${CMAKE_CURRENT_BINARY_DIR}")
	add_custom_command(TARGET test_opencv COMMAND ${CMAKE_COMMAND} -E copy ${image} ${CMAKE_CURRENT_BINARY_DIR})
endforeach(image)
execute_process(COMMAND ln -fs ${CMAKE_SOURCE_DIR}/server-web ${CMAKE_CURRENT_BINARY_DIR}/server-web)

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
